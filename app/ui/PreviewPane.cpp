#include "PreviewPane.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QMouseEvent>
#include <QFont>
#include <QGuiApplication>
#include <QStyleHints>
#include <QTimer>
#include <QPalette>
#include <QColor>
#include <QPixmapCache>
#include <QtConcurrent>
#include <QFutureWatcher>
#include "Theme.h"
#include "../core/Settings.h"

PreviewPane::PreviewPane(QWidget* parent) : QWidget(parent) {
    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(8,8,8,8);
    auto* top = new QHBoxLayout();
    // Heart button placed in top layout (LEFT side)
    m_heart = new QPushButton(this);
    m_heart->setObjectName("previewHeart");
    m_heart->setFlat(true);
    m_heart->setCursor(Qt::PointingHandCursor);
    // Keep popup's keyboard focus on the search box
    m_heart->setFocusPolicy(Qt::NoFocus);
    m_heart->setToolTip(QStringLiteral("切换收藏 (Ctrl/⌘+D)"));
    // Keep square button to avoid icon squashing
    m_heart->setStyleSheet("QPushButton{background:transparent;border:none;padding:4px;} QPushButton:hover{background:rgba(127,127,127,0.12);border-radius:6px;}");
    // Shrink heart icon/button by 20%
    m_heart->setIconSize(QSize(16, 16));
    m_heart->setFixedSize(QSize(22, 22));
    m_heart->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    top->addWidget(m_heart);
    top->addSpacing(6);

    m_usageLabel = new QLabel();
    // Color adapts via applySecondaryTextStyle()
    m_usageLabel->setText(QStringLiteral("使用次数：0"));
    m_usageLabel->setVisible(false);
    top->addWidget(m_usageLabel);

    top->addSpacing(10);
    m_sourceLabel = new QLabel();
    // Color adapts via applySecondaryTextStyle()
    m_sourceLabel->setVisible(false);
    top->addWidget(m_sourceLabel);

    top->addStretch();
    m_scaleLabel = new QLabel();
    // Color adapts via applySecondaryTextStyle()
    m_scaleLabel->setVisible(false); // hide scale indicator text per UX
    top->addWidget(m_scaleLabel);
    v->addLayout(top);

    // Content scroll area below the fixed top bar
    m_contentScroll = new QScrollArea(this);
    m_contentScroll->setFrameShape(QFrame::NoFrame);
    m_contentScroll->setWidgetResizable(true);
    // Default: no horizontal scroll for fit-to-width; can be enabled for 100% images
    m_contentScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // Create inner content widget
    auto* content = new QWidget();
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    m_textLabel = new QLabel();
    m_textLabel->setWordWrap(true);
    // Allow mouse selection without taking keyboard focus
    m_textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_textLabel->setFocusPolicy(Qt::NoFocus);
    // Make preview content text slightly smaller than default
    {
        QFont f = m_textLabel->font();
        if (f.pointSizeF() > 0) f.setPointSizeF(std::max(1.0, f.pointSizeF() - 1));
        else if (f.pointSize() > 0) f.setPointSize(std::max(1, f.pointSize() - 1));
        else if (f.pixelSize() > 0) f.setPixelSize(std::max(8, f.pixelSize() - 1));
        m_textLabel->setFont(f);
    }
    m_textLabel->setVisible(false);
    contentLayout->addWidget(m_textLabel, 1);

    m_imageLabel = new QLabel();
    m_imageLabel->setFocusPolicy(Qt::NoFocus);
    m_imageLabel->setVisible(false);
    m_imageLabel->installEventFilter(this);
    m_imageLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    contentLayout->addWidget(m_imageLabel, 1);

    m_contentScroll->setWidget(content);
    v->addWidget(m_contentScroll, 1);

    // heart click handler
    connect(m_heart, &QPushButton::clicked, this, &PreviewPane::onHeart);
    // react to theme changes
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
    // Defer updates to avoid picking stale palette/icon variants during transition
    QObject::connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this]{
        QTimer::singleShot(0, [this]{
            updateHeart();
            updateImageDisplay();
            applySecondaryTextStyle();
        });
    });
#endif
    // Initial theme-aware text color
    applySecondaryTextStyle();
}

void PreviewPane::showText(qint64 id, const QString& text, bool favorite, int usageCount, const QString& appName) {
    m_id = id; m_favorite = favorite; m_isImage = false; m_usageCount = usageCount; m_appName = appName;
    m_textLabel->setText(text);
    m_textLabel->setVisible(true);
    m_imageLabel->setVisible(false);
    m_heart->setVisible(true);
    updateHeart();
    updateScaleLabel();
    m_sourceLabel->setVisible(!m_appName.trimmed().isEmpty());
    updateSourceLabel();
    m_usageLabel->setVisible(true);
    updateUsageLabel();
}

void PreviewPane::showImage(qint64 id, const QImage& img, bool favorite, int usageCount,
                            const QString& appName, const QPixmap& scaled) {
    m_id = id; m_favorite = favorite; m_isImage = true; m_fitToWidth = true; m_usageCount = usageCount; m_appName = appName;
    m_imageOrig = QPixmap::fromImage(img);
    m_imageLabel->setVisible(true);
    m_textLabel->setVisible(false);
    m_heart->setVisible(true);
    if (!scaled.isNull()) {
        m_imageLabel->setPixmap(scaled);
    } else {
        updateImageDisplay();
    }
    updateScaleLabel();
    updateHeart();
    m_sourceLabel->setVisible(!m_appName.trimmed().isEmpty());
    updateSourceLabel();
    m_usageLabel->setVisible(true);
    updateUsageLabel();
}

void PreviewPane::setScaledPixmap(const QPixmap& pix) {
    if (!pix.isNull()) m_imageLabel->setPixmap(pix);
}

bool PreviewPane::eventFilter(QObject* obj, QEvent* ev) {
    if (obj == m_imageLabel && ev->type() == QEvent::MouseButtonDblClick) {
        m_fitToWidth = !m_fitToWidth;
        updateImageDisplay();
        updateScaleLabel();
        return true;
    }
    return QWidget::eventFilter(obj, ev);
}

void PreviewPane::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    if (m_isImage) { updateImageDisplay(); updateScaleLabel(); }
}

void PreviewPane::changeEvent(QEvent* e) {
    QWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::PaletteChange:
        case QEvent::ApplicationPaletteChange:
        case QEvent::StyleChange:
            // Recompute label colors when palette/style changes
            applySecondaryTextStyle();
            break;
        default:
            break;
    }
}

void PreviewPane::onHeart() {
    if (m_id <= 0) return;
    m_favorite = !m_favorite;
    updateHeart();
    emit favoriteToggled(m_id, m_favorite);
}

void PreviewPane::triggerToggleFavorite() {
    onHeart();
}

void PreviewPane::updateHeart() {
    const auto scheme = Theme::effectiveScheme(m_settings);
    const QString iconPath = Theme::icon(m_favorite ? "heart_filled" : "heart_empty", scheme);
    if (!iconPath.isEmpty()) m_heart->setIcon(QIcon(iconPath));
    else m_heart->setText(m_favorite ? "♥" : "♡");
}

void PreviewPane::updateImageDisplay() {
    if (m_imageOrig.isNull()) return;
    // Manage scrollbars on the inner content scroll area only
    if (m_contentScroll) m_contentScroll->setHorizontalScrollBarPolicy(m_fitToWidth ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded);
    if (m_fitToWidth) {
        // Use the viewport width directly to avoid oscillation from scrollbar visibility changes
        int w = this->width();
        if (m_contentScroll && m_contentScroll->viewport())
            w = m_contentScroll->viewport()->width();
        if (w < 1) w = 1;
        const QString key = QString::number(m_id) + ":" + QString::number(w);
        QPixmap cached;
        if (QPixmapCache::find(key, &cached)) {
            m_imageLabel->setPixmap(cached);
            return;
        }
        // Show a quick placeholder using fast transformation
        QPixmap fast = m_imageOrig.scaledToWidth(w, Qt::FastTransformation);
        m_imageLabel->setPixmap(fast);
        qint64 currentId = m_id;
        auto future = QtConcurrent::run([orig = m_imageOrig, w]{
            return orig.scaledToWidth(w, Qt::SmoothTransformation);
        });
        auto* watcher = new QFutureWatcher<QPixmap>(this);
        connect(watcher, &QFutureWatcher<QPixmap>::finished, this, [this, watcher, key, currentId]{
            QPixmap pix = watcher->result();
            QPixmapCache::insert(key, pix);
            if (m_id == currentId && m_fitToWidth) m_imageLabel->setPixmap(pix);
            watcher->deleteLater();
        });
        watcher->setFuture(future);
    } else {
        // Only update if different to avoid resize loops
        {
            QPixmap cur = m_imageLabel->pixmap();
            if (!cur.isNull() && cur.cacheKey() == m_imageOrig.cacheKey()) return;
        }
        m_imageLabel->setPixmap(m_imageOrig);
    }
}

void PreviewPane::updateScaleLabel() {
    // Removed scale text from UI; keep label hidden
    if (!m_scaleLabel) return;
    m_scaleLabel->clear();
    m_scaleLabel->setVisible(false);
}

void PreviewPane::updateUsageLabel() {
    if (!m_usageLabel) return;
    m_usageLabel->setText(QStringLiteral("使用次数：%1").arg(m_usageCount));
}

void PreviewPane::updateSourceLabel() {
    if (!m_sourceLabel) return;
    const QString name = m_appName.trimmed();
    if (name.isEmpty()) {
        m_sourceLabel->clear();
        m_sourceLabel->setVisible(false);
    } else {
        m_sourceLabel->setText(QStringLiteral("来源：%1").arg(name));
        m_sourceLabel->setVisible(true);
    }
}

void PreviewPane::applySecondaryTextStyle() {
    // Pick a de-emphasized foreground color that follows the current palette
    QPalette pal = this->palette();
    QColor c = pal.color(QPalette::PlaceholderText);
    if (!c.isValid() || c.alpha() == 0) {
        // Fallback: derive from WindowText with reduced alpha
        c = pal.color(QPalette::WindowText);
        c.setAlpha(160);
    }
    const QString css = QStringLiteral("color: %1; font-size: 11px;").arg(c.name(QColor::HexArgb));
    if (m_scaleLabel) m_scaleLabel->setStyleSheet(css);
    if (m_sourceLabel) m_sourceLabel->setStyleSheet(css);
    if (m_usageLabel) m_usageLabel->setStyleSheet(css);
}

void PreviewPane::attachSettings(Settings* settings) {
    m_settings = settings;
    if (m_settings) QObject::connect(m_settings, &Settings::changed, this, [this]{ updateHeart(); updateImageDisplay(); });
    updateHeart();
}

void PreviewPane::clear() {
    m_id = 0;
    m_favorite = false;
    m_isImage = false;
    m_imageOrig = QPixmap();
    m_textLabel->clear();
    m_imageLabel->clear();
    m_textLabel->setVisible(false);
    m_imageLabel->setVisible(false);
    m_heart->setVisible(false);
    m_usageLabel->setVisible(false);
    if (m_sourceLabel) { m_sourceLabel->clear(); m_sourceLabel->setVisible(false); }
    m_scaleLabel->clear();
}
