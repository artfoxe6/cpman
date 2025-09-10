#include "PreviewPane.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QMouseEvent>
#include <QFont>
#include <QGuiApplication>
#include <QStyleHints>
#include <QTimer>
#include <QPalette>
#include <QColor>
#include "Theme.h"
#include "../core/Settings.h"

PreviewPane::PreviewPane(QWidget* parent) : QWidget(parent) {
    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(8,8,8,8);
    auto* top = new QHBoxLayout();
    m_scaleLabel = new QLabel();
    // Color adapts via applySecondaryTextStyle()
    m_scaleLabel->setVisible(false); // hide scale indicator text per UX
    top->addWidget(m_scaleLabel);
    top->addStretch();
    m_sourceLabel = new QLabel();
    // Color adapts via applySecondaryTextStyle()
    m_sourceLabel->setVisible(false);
    top->addWidget(m_sourceLabel);

    m_usageLabel = new QLabel();
    // Color adapts via applySecondaryTextStyle()
    m_usageLabel->setText(QStringLiteral("使用次数：0"));
    m_usageLabel->setVisible(false);
    top->addWidget(m_usageLabel);
    v->addLayout(top);

    m_textLabel = new QLabel();
    m_textLabel->setWordWrap(true);
    // Allow mouse selection without taking keyboard focus
    m_textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_textLabel->setFocusPolicy(Qt::NoFocus);
    m_textLabel->setVisible(false);
    v->addWidget(m_textLabel, 1);

    m_imageLabel = new QLabel();
    m_imageLabel->setFocusPolicy(Qt::NoFocus);
    m_imageLabel->setVisible(false);
    m_imageLabel->installEventFilter(this);
    m_imageLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    v->addWidget(m_imageLabel, 1);

    // Heart button placed in top layout (right side)
    m_heart = new QPushButton(this);
    m_heart->setObjectName("previewHeart");
    m_heart->setFlat(true);
    m_heart->setCursor(Qt::PointingHandCursor);
    // Keep popup's keyboard focus on the search box
    m_heart->setFocusPolicy(Qt::NoFocus);
    m_heart->setToolTip(QStringLiteral("切换收藏 (Ctrl/⌘+D)"));
    // Keep square button to avoid icon squashing
    m_heart->setStyleSheet("QPushButton{background:transparent;border:none;padding:4px;} QPushButton:hover{background:rgba(127,127,127,0.12);border-radius:6px;}");
    m_heart->setIconSize(QSize(20, 20));
    m_heart->setFixedSize(QSize(28, 28));
    m_heart->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // add to layout
    top->addWidget(m_heart);
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

void PreviewPane::showImage(qint64 id, const QImage& img, bool favorite, int usageCount, const QString& appName) {
    m_id = id; m_favorite = favorite; m_isImage = true; m_fitToWidth = true; m_usageCount = usageCount; m_appName = appName;
    m_imageOrig = QPixmap::fromImage(img);
    m_imageLabel->setVisible(true);
    m_textLabel->setVisible(false);
    m_heart->setVisible(true);
    updateImageDisplay();
    updateScaleLabel();
    updateHeart();
    m_sourceLabel->setVisible(!m_appName.trimmed().isEmpty());
    updateSourceLabel();
    m_usageLabel->setVisible(true);
    updateUsageLabel();
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
    // Try to locate the parent scroll area to manage scrollbars
    QScrollArea* sa = nullptr;
    {
        QWidget* p = this->parentWidget();
        while (p && !qobject_cast<QScrollArea*>(p)) p = p->parentWidget();
        sa = qobject_cast<QScrollArea*>(p);
    }
    // In fit-to-width mode, disable horizontal scrollbar; in 100% mode, allow as needed
    if (sa) sa->setHorizontalScrollBarPolicy(m_fitToWidth ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded);
    if (m_fitToWidth) {
        // Prefer the scroll area's viewport width to avoid feedback resizing
        int vpw = this->width();
        if (sa) vpw = sa->viewport()->width();
        // Account for left/right layout margins (8 + 8)
        int w = vpw - 16;
        if (w < 1) w = 1;
        {
            QPixmap cur = m_imageLabel->pixmap();
            if (!cur.isNull() && qAbs(cur.width() - w) <= 1) return; // prevent thrashing growth
        }
        QPixmap scaled = m_imageOrig.scaledToWidth(w, Qt::SmoothTransformation);
        m_imageLabel->setPixmap(scaled);
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
