#include "MainPopup.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QListView>
#include <QScrollArea>
#include <QScrollBar>
#include <QPushButton>
#include <QKeyEvent>
#include <QApplication>
#include <QGuiApplication>
#include <QStyleHints>
#include "Theme.h"
#include <QMouseEvent>
#include "PreviewPane.h"
#include "../core/Settings.h"
#include <QGraphicsDropShadowEffect>
#include <QFrame>
#include <QAction>

MainPopup::MainPopup(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    // Enable translucent background for drop shadow around rounded container
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowTitle(QStringLiteral("剪贴板搜索"));

    // Outer layout with margin to show shadow blur
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(12, 12, 12, 12);
    m_container = new QWidget(this);
    // Rounded background that follows palette, no border
    m_container->setStyleSheet("background: palette(window); border-radius: 5px;");
    auto* v = new QVBoxLayout(m_container);
    v->setContentsMargins(10, 10, 10, 10);
    outer->addWidget(m_container);

    // Subtle drop shadow
    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(0, 0, 0, 80));
    m_container->setGraphicsEffect(shadow);

    auto* top = new QHBoxLayout();
    top->setContentsMargins(6, 2, 6, 6);
    top->setSpacing(8);
    m_search = new QLineEdit();
    m_search->setPlaceholderText(QStringLiteral("搜索…"));
    m_useDb = new QCheckBox(QStringLiteral("数据库"));
    m_onlyFav = new QCheckBox(QStringLiteral("收藏列表"));
    // Control appearance tweaks
    auto applyUi = [this]{
        m_search->setClearButtonEnabled(true);
        // Remove borders on search field
        m_search->setFrame(false);
        m_search->setStyleSheet(
            "QLineEdit{border:none;padding:8px 10px;background:palette(base);border-radius:6px;}"
            "QLineEdit:focus{outline:none;}"
        );
        // Remove frames from list and preview scroll area (set after created)
        if (m_list) m_list->setFrameShape(QFrame::NoFrame);
        if (m_previewScroll) m_previewScroll->setFrameShape(QFrame::NoFrame);
        // Thin scrollbars for children within this popup
        const QString ss =
            // Thin, line-like scrollbars
            " QScrollBar:vertical{width:4px;margin:0;background:transparent;}"
            " QScrollBar:horizontal{height:4px;margin:0;background:transparent;}"
            " QScrollBar::handle:vertical{background:palette(mid);min-height:24px;border-radius:2px;margin:0;}"
            " QScrollBar::handle:horizontal{background:palette(mid);min-width:24px;border-radius:2px;margin:0;}"
            " QScrollBar::add-line, QScrollBar::sub-line{height:0;width:0;background:transparent;}"
            " QScrollBar::add-page, QScrollBar::sub-page{background:transparent;}";
        this->setStyleSheet(ss);
    };
    // Add leading search icon action
    auto updateTopIcons = [this]{
        const auto scheme = Theme::effectiveScheme(nullptr);
        const QString searchIcon = Theme::icon("search", scheme);
        if (!searchIcon.isEmpty()) {
            // Ensure only one leading action
            for (QAction* a : m_search->actions()) m_search->removeAction(a);
            m_search->addAction(QIcon(searchIcon), QLineEdit::LeadingPosition);
        }
        const QString dbIcon = Theme::icon("db", scheme);
        if (!dbIcon.isEmpty()) {
            m_useDb->setIcon(QIcon(dbIcon));
            m_useDb->setIconSize(QSize(16,16));
        }
        const QString favIcon = Theme::icon("heart_filled", scheme);
        if (!favIcon.isEmpty()) {
            m_onlyFav->setIcon(QIcon(favIcon));
            m_onlyFav->setIconSize(QSize(16,16));
        }
    };
    applyUi();
    updateTopIcons();
    // React to theme change (for potential palette changes in styles)
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
    QObject::connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [applyUi, updateTopIcons]{ applyUi(); updateTopIcons(); });
#endif
    top->addWidget(m_search, 1);
    top->addWidget(m_useDb);
    top->addWidget(m_onlyFav);
    v->addLayout(top);

    auto* bottom = new QHBoxLayout();
    m_list = new QListView();
    m_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setMouseTracking(true);
    m_list->setSpacing(2);
    // Remove list frame border
    m_list->setFrameShape(QFrame::NoFrame);
    // Double-click on a history item commits just like pressing Enter
    connect(m_list, &QListView::doubleClicked, this, [this](const QModelIndex& idx){
        if (idx.isValid()) m_list->setCurrentIndex(idx);
        emit commitRequested();
    });
    m_previewScroll = new QScrollArea();
    m_previewScroll->setFrameShape(QFrame::NoFrame);
    m_preview = new PreviewPane();
    m_previewScroll->setWidget(m_preview);
    m_previewScroll->setWidgetResizable(true);
    // Reparent preview's heart button to viewport so it floats fixed at top-right
    if (auto* heart = m_preview->findChild<QPushButton*>("previewHeart")) {
        auto* vp = m_previewScroll->viewport();
        heart->setParent(vp);
        heart->show();
        auto placeHeart = [heart, vp]{
            const int margin = 4;
            QSize sz = heart->size();
            heart->move(vp->width() - sz.width() - margin, margin);
            heart->raise();
        };
        placeHeart();
        // Adjust on viewport resize
        m_previewScroll->viewport()->installEventFilter(this);
    }
    bottom->addWidget(m_list, 1);
    bottom->addWidget(m_previewScroll, 1);
    v->addLayout(bottom, 1);

    m_search->installEventFilter(this);
    m_list->installEventFilter(this);
    // Capture mouse for dragging on container area
    m_container->installEventFilter(this);

    connect(m_search, &QLineEdit::textChanged, this, [this](const QString& t){
        emit requestSearch(t, m_useDb->isChecked(), m_onlyFav->isChecked());
    });
    connect(m_useDb, &QCheckBox::toggled, this, [this]{ emit requestSearch(m_search->text(), m_useDb->isChecked(), m_onlyFav->isChecked()); });
    connect(m_onlyFav, &QCheckBox::toggled, this, [this]{ emit requestSearch(m_search->text(), m_useDb->isChecked(), m_onlyFav->isChecked()); });
}

void MainPopup::showPopup() {
    if (!m_sizedOnce) {
        // Prefer user-defined default size from settings
        if (m_settings) {
            const QSize s = m_settings->popupSize();
            if (s.isValid()) {
                resize(s);
            } else {
                // Fallback: enlarge initial size to 1.5x of natural size
                resize(QSize(600, 550));
            }
        } else {
            resize(QSize(600, 550));
        }
        m_sizedOnce = true;
    }
    show();
    raise();
    activateWindow();
    m_search->setFocus();
}

void MainPopup::hidePopup() { hide(); emit popupHidden(); }

bool MainPopup::eventFilter(QObject* obj, QEvent* ev) {
    // Allow dragging the popup by grabbing the container background
    if (obj == m_container) {
        if (ev->type() == QEvent::MouseButtonPress) {
            auto* me = static_cast<QMouseEvent*>(ev);
            if (me->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragOffset = me->globalPosition().toPoint() - frameGeometry().topLeft();
                return true;
            }
        } else if (ev->type() == QEvent::MouseMove) {
            auto* me = static_cast<QMouseEvent*>(ev);
            if (m_dragging && (me->buttons() & Qt::LeftButton)) {
                move(me->globalPosition().toPoint() - m_dragOffset);
                return true;
            }
        } else if (ev->type() == QEvent::MouseButtonRelease) {
            m_dragging = false;
        }
    }
    if ((obj == m_search || obj == m_list) && ev->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(ev);
        // When typing in the search box, route navigation keys to list
        if (obj == m_search) {
            switch (ke->key()) {
                case Qt::Key_Up:
                case Qt::Key_Down:
                case Qt::Key_PageUp:
                case Qt::Key_PageDown:
                case Qt::Key_Home:
                case Qt::Key_End:
                    QApplication::sendEvent(m_list, ev);
                    return true;
                default:
                    break;
            }
        }

        // Global shortcuts while popup is focused (list or search)
        switch (ke->key()) {
            case Qt::Key_D:
                if ((ke->modifiers() & Qt::ControlModifier) || (ke->modifiers() & Qt::MetaModifier)) {
                    if (m_preview) m_preview->triggerToggleFavorite();
                    return true;
                }
                break;
            case Qt::Key_Return:
            case Qt::Key_Enter:
                emit commitRequested();
                return true;
            case Qt::Key_Escape:
                hidePopup();
                return true;
            default:
                break;
        }
    }
    // Reposition floating heart when preview viewport resizes
    if (m_previewScroll && obj == m_previewScroll->viewport() && ev->type() == QEvent::Resize) {
        if (auto* heart = m_previewScroll->viewport()->findChild<QPushButton*>("previewHeart")) {
            const int margin = 4;
            QWidget* vp = m_previewScroll->viewport();
            QSize sz = heart->size();
            heart->move(vp->width() - sz.width() - margin, margin);
            heart->raise();
        }
    }
    return QWidget::eventFilter(obj, ev);
}

void MainPopup::focusOutEvent(QFocusEvent* e) {
    QWidget::focusOutEvent(e);
    hidePopup();
}

void MainPopup::setListModel(QAbstractItemModel* model) {
    m_list->setModel(model);
    if (m_list->selectionModel()) {
        connect(m_list->selectionModel(), &QItemSelectionModel::currentChanged,
                this, [this](const QModelIndex& current, const QModelIndex&){ updatePreviewFromIndex(current); });
    }
}

QModelIndex MainPopup::currentIndex() const {
    return m_list->currentIndex();
}

void MainPopup::setCurrentIndex(const QModelIndex& idx) {
    m_list->setCurrentIndex(idx);
}

void MainPopup::updatePreviewFromIndex(const QModelIndex& idx) {
    if (!idx.isValid()) return;
    const auto type = idx.data(Qt::UserRole + 2).toString(); // TypeRole string
    const qint64 id = idx.data(Qt::UserRole + 1).toLongLong(); // IdRole
    const bool fav = idx.data(Qt::UserRole + 5).toBool(); // FavoriteRole
    if (type == "text") {
        const QString text = idx.data(Qt::UserRole + 3).toString(); // TextRole
        m_preview->showText(id, text, fav);
    } else {
        const QString path = idx.data(Qt::UserRole + 4).toString(); // MediaPathRole
        QImage img(path);
        if (!img.isNull()) m_preview->showImage(id, img, fav);
    }
}

QString MainPopup::queryText() const { return m_search->text(); }
bool MainPopup::useDbChecked() const { return m_useDb->isChecked(); }
bool MainPopup::onlyFavChecked() const { return m_onlyFav->isChecked(); }

void MainPopup::attachSettings(Settings* settings) {
    m_settings = settings;
    if (m_preview) m_preview->attachSettings(settings);
}

void MainPopup::applyDefaultSize(const QSize& sz) {
    if (!sz.isValid()) return;
    if (isVisible()) {
        resize(sz);
    } else {
        // Re-apply size on next show
        m_sizedOnce = false;
    }
}

void MainPopup::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragOffset = e->globalPosition().toPoint() - frameGeometry().topLeft();
        e->accept();
    } else {
        QWidget::mousePressEvent(e);
    }
}

void MainPopup::mouseMoveEvent(QMouseEvent* e) {
    if (m_dragging && (e->buttons() & Qt::LeftButton)) {
        move(e->globalPosition().toPoint() - m_dragOffset);
        e->accept();
    } else {
        QWidget::mouseMoveEvent(e);
    }
}
