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
#include "Theme.h"
#include "../core/Settings.h"

PreviewPane::PreviewPane(QWidget* parent) : QWidget(parent) {
    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(8,8,8,8);
    auto* top = new QHBoxLayout();
    m_scaleLabel = new QLabel();
    m_scaleLabel->setStyleSheet("color: palette(mid); font-size: 11px;");
    top->addWidget(m_scaleLabel);
    top->addStretch();
    m_usageLabel = new QLabel();
    m_usageLabel->setStyleSheet("color: palette(mid); font-size: 11px;");
    m_usageLabel->setText(QStringLiteral("使用次数：0"));
    m_usageLabel->setVisible(false);
    top->addWidget(m_usageLabel);
    v->addLayout(top);

    m_textLabel = new QLabel();
    m_textLabel->setWordWrap(true);
    m_textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    m_textLabel->setVisible(false);
    v->addWidget(m_textLabel, 1);

    m_imageLabel = new QLabel();
    m_imageLabel->setVisible(false);
    m_imageLabel->installEventFilter(this);
    m_imageLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    v->addWidget(m_imageLabel, 1);

    // Heart button placed in top layout (right side)
    m_heart = new QPushButton(this);
    m_heart->setObjectName("previewHeart");
    m_heart->setFlat(true);
    m_heart->setCursor(Qt::PointingHandCursor);
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
    QObject::connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this]{ updateHeart(); updateImageDisplay(); });
#endif
}

void PreviewPane::showText(qint64 id, const QString& text, bool favorite, int usageCount) {
    m_id = id; m_favorite = favorite; m_isImage = false; m_usageCount = usageCount;
    m_textLabel->setText(text);
    m_textLabel->setVisible(true);
    m_imageLabel->setVisible(false);
    m_heart->setVisible(true);
    updateHeart();
    updateScaleLabel();
    m_usageLabel->setVisible(true);
    updateUsageLabel();
}

void PreviewPane::showImage(qint64 id, const QImage& img, bool favorite, int usageCount) {
    m_id = id; m_favorite = favorite; m_isImage = true; m_fitToWidth = true; m_usageCount = usageCount;
    m_imageOrig = QPixmap::fromImage(img);
    m_imageLabel->setVisible(true);
    m_textLabel->setVisible(false);
    m_heart->setVisible(true);
    updateImageDisplay();
    updateScaleLabel();
    updateHeart();
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
    if (!m_isImage) { m_scaleLabel->setText(QString()); return; }
    if (m_fitToWidth) {
        m_scaleLabel->setText(QStringLiteral("适应宽度"));
    } else {
        m_scaleLabel->setText(QStringLiteral("100%"));
    }
}

void PreviewPane::updateUsageLabel() {
    if (!m_usageLabel) return;
    m_usageLabel->setText(QStringLiteral("使用次数：%1").arg(m_usageCount));
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
    m_scaleLabel->clear();
}
