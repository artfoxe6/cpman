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
    v->setContentsMargins(4,4,4,4);
    auto* top = new QHBoxLayout();
    m_scaleLabel = new QLabel();
    m_scaleLabel->setStyleSheet("color: gray; font-size: 11px;");
    top->addWidget(m_scaleLabel);
    top->addStretch();
    v->addLayout(top);

    m_textLabel = new QLabel();
    m_textLabel->setWordWrap(true);
    m_textLabel->setVisible(false);
    v->addWidget(m_textLabel, 1);

    m_imageLabel = new QLabel();
    m_imageLabel->setVisible(false);
    m_imageLabel->installEventFilter(this);
    m_imageLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    v->addWidget(m_imageLabel, 1);

    // Floating heart button (overlay, fixed at top-right)
    m_heart = new QPushButton(this);
    m_heart->setObjectName("previewHeart");
    m_heart->setFlat(true);
    m_heart->setCursor(Qt::PointingHandCursor);
    m_heart->setToolTip(QStringLiteral("切换收藏 (Ctrl/⌘+D)"));
    // Keep square button to avoid icon squashing
    m_heart->setStyleSheet("QPushButton{background:transparent;border:none;padding:4px;} ");
    m_heart->setIconSize(QSize(20, 20));
    m_heart->setFixedSize(QSize(28, 28));
    m_heart->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_heart->raise();
    connect(m_heart, &QPushButton::clicked, this, &PreviewPane::onHeart);
    // react to theme changes
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
    QObject::connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this]{ updateHeart(); updateImageDisplay(); });
#endif
}

void PreviewPane::showText(qint64 id, const QString& text, bool favorite) {
    m_id = id; m_favorite = favorite; m_isImage = false;
    m_textLabel->setText(text);
    m_textLabel->setVisible(true);
    m_imageLabel->setVisible(false);
    updateHeart();
    updateScaleLabel();
}

void PreviewPane::showImage(qint64 id, const QImage& img, bool favorite) {
    m_id = id; m_favorite = favorite; m_isImage = true; m_fitToWidth = true;
    m_imageOrig = QPixmap::fromImage(img);
    m_imageLabel->setVisible(true);
    m_textLabel->setVisible(false);
    updateImageDisplay();
    updateScaleLabel();
    updateHeart();
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
    // place heart at top-right with small margin when owned by this widget
    if (m_heart && m_heart->parent() == this) {
        const int margin = 4;
        QSize sz = m_heart->size();
        m_heart->move(width() - sz.width() - margin, margin);
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
    if (m_fitToWidth) {
        int w = this->width() - 8; // padding
        if (w < 1) w = 1;
        QPixmap scaled = m_imageOrig.scaledToWidth(w, Qt::SmoothTransformation);
        m_imageLabel->setPixmap(scaled);
    } else {
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

void PreviewPane::attachSettings(Settings* settings) {
    m_settings = settings;
    if (m_settings) QObject::connect(m_settings, &Settings::changed, this, [this]{ updateHeart(); updateImageDisplay(); });
    updateHeart();
}
