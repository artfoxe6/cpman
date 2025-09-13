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
#include <QTimer>
#include "Theme.h"
#include <QMouseEvent>
#include "PreviewPane.h"
#include "../core/Settings.h"
#include <QGraphicsDropShadowEffect>
#include <QFrame>
#include <QAction>
#include "HistoryListModel.h"
#include <QPixmapCache>

MainPopup::MainPopup(QWidget* parent) : QWidget(parent) {
    QPixmapCache::setCacheLimit(65536); // ~64MB for preview images
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    // Enable translucent background for drop shadow around rounded container
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowTitle(QStringLiteral("剪贴板搜索"));

    // Outer layout with margin to show shadow blur
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(12, 12, 12, 12);
    m_container = new QWidget(this);
    // Rounded background that follows our custom palette window color, no border
    m_container->setStyleSheet("background: palette(window); border-radius: 5px;");
    // Apply initial theme-specific background color to container
    {
        const auto scheme = Theme::effectiveScheme(nullptr);
        QPalette pal = m_container->palette();
        pal.setColor(QPalette::Window, Theme::popupWindowColor(scheme));
        m_container->setAutoFillBackground(true);
        m_container->setPalette(pal);
    }
    auto* v = new QVBoxLayout(m_container);
    v->setContentsMargins(10, 10, 10, 10);
    outer->addWidget(m_container);

    // Subtle drop shadow
    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(0, 0, 0, 80));
    m_container->setGraphicsEffect(shadow);

    // Top container for search and checkboxes
    auto* topContainer = new QWidget();
    auto* topContainerLayout = new QVBoxLayout(topContainer);
    topContainerLayout->setContentsMargins(6, 2, 6, 6);
    topContainerLayout->setSpacing(6);

    // Search line - takes full width
    auto* searchLayout = new QHBoxLayout();
    searchLayout->setContentsMargins(0, 0, 0, 0);
    m_search = new QLineEdit();
    m_search->setPlaceholderText(QStringLiteral("搜索…"));
    searchLayout->addWidget(m_search, 1);
    topContainerLayout->addLayout(searchLayout);

    // Checkboxes - horizontal layout below search
    auto* checkboxesLayout = new QHBoxLayout();
    checkboxesLayout->setContentsMargins(0, 0, 0, 0);
    checkboxesLayout->setSpacing(20);
    m_useDb = new QCheckBox(QStringLiteral("数据库"));
    m_onlyFav = new QCheckBox(QStringLiteral("收藏列表"));
    checkboxesLayout->addWidget(m_useDb);
    checkboxesLayout->addWidget(m_onlyFav);
    checkboxesLayout->addStretch();
    topContainerLayout->addLayout(checkboxesLayout);

    // Route focus of the popup to the search box whenever the window itself gains focus
    setFocusProxy(m_search);
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
    // Defer UI restyle until after Qt has finished updating the palette
    QObject::connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this, applyUi, updateTopIcons]{
        QTimer::singleShot(0, [this, applyUi, updateTopIcons]{
            applyUi(); updateTopIcons();
            // Update background color with new scheme
            const auto scheme = Theme::effectiveScheme(nullptr);
            QPalette pal = m_container->palette();
            pal.setColor(QPalette::Window, Theme::popupWindowColor(scheme));
            m_container->setPalette(pal);
        });
    });
#endif
    v->addWidget(topContainer);

    auto* bottom = new QHBoxLayout();
    m_list = new QListView();
    m_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setMouseTracking(true);
    m_list->setSpacing(2);
    // Make list item text slightly larger for readability (reduced by one step per request)
    {
        QFont lf = m_list->font();
        if (lf.pointSizeF() > 0) lf.setPointSizeF(lf.pointSizeF() + 1);
        else if (lf.pointSize() > 0) lf.setPointSize(lf.pointSize() + 1);
        else if (lf.pixelSize() > 0) lf.setPixelSize(lf.pixelSize() + 1);
        m_list->setFont(lf);
    }
    // Keep keyboard focus on the search box: prevent the list from stealing focus on click
    m_list->setFocusPolicy(Qt::NoFocus);
    if (m_list->viewport()) m_list->viewport()->setFocusPolicy(Qt::NoFocus);
    // Remove list frame border
    m_list->setFrameShape(QFrame::NoFrame);
    // Double-click on a history item commits just like pressing Enter
    connect(m_list, &QListView::doubleClicked, this, [this](const QModelIndex& idx){
        if (idx.isValid()) m_list->setCurrentIndex(idx);
        emit commitRequested();
    });
    m_previewScroll = new QScrollArea();
    // Keep top info fixed: outer scroll area never scrolls; inner PreviewPane manages content scroll
    m_previewScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_previewScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_previewScroll->setFrameShape(QFrame::NoFrame);
    m_preview = new PreviewPane();
    // Do not allow preview widgets to take focus away from search
    m_previewScroll->setFocusPolicy(Qt::NoFocus);
    m_preview->setFocusPolicy(Qt::NoFocus);
    if (m_previewScroll->viewport()) m_previewScroll->viewport()->setFocusPolicy(Qt::NoFocus);
    // Also ensure scrollbars never take focus
    if (m_previewScroll->horizontalScrollBar()) m_previewScroll->horizontalScrollBar()->setFocusPolicy(Qt::NoFocus);
    if (m_previewScroll->verticalScrollBar()) m_previewScroll->verticalScrollBar()->setFocusPolicy(Qt::NoFocus);
    m_previewScroll->setWidget(m_preview);
    m_previewScroll->setWidgetResizable(true);
    // Heart button is placed within PreviewPane's top bar layout; no floating reparenting needed.
    bottom->addWidget(m_list, 1);
    bottom->addWidget(m_previewScroll, 1);
    v->addLayout(bottom, 1);

    // Strong focus on search; all other interactive widgets are NoFocus
    m_search->setFocusPolicy(Qt::StrongFocus);
    m_useDb->setFocusPolicy(Qt::NoFocus);
    m_onlyFav->setFocusPolicy(Qt::NoFocus);

    m_search->installEventFilter(this);
    m_list->installEventFilter(this);
    // Guard preview region clicks from stealing focus
    m_preview->installEventFilter(this);
    m_previewScroll->installEventFilter(this);
    if (m_previewScroll->viewport()) m_previewScroll->viewport()->installEventFilter(this);
    // Install on existing children (labels) as well
    const auto previewChildren = m_preview->findChildren<QObject*>();
    for (QObject* c : previewChildren) c->installEventFilter(this);
    // Capture mouse for dragging on container area
    m_container->installEventFilter(this);

    connect(m_search, &QLineEdit::textChanged, this, [this](const QString& t){
        emit requestSearch(t, m_useDb->isChecked(), m_onlyFav->isChecked());
    });
    connect(m_useDb, &QCheckBox::toggled, this, [this]{ emit requestSearch(m_search->text(), m_useDb->isChecked(), m_onlyFav->isChecked()); });
    connect(m_onlyFav, &QCheckBox::toggled, this, [this]{ emit requestSearch(m_search->text(), m_useDb->isChecked(), m_onlyFav->isChecked()); });

    // Keep focus anchored on the search field while the popup is visible/active
    QObject::connect(qApp, &QApplication::focusChanged, this,
                     [this](QWidget* /*old*/, QWidget* now){
        if (!this->isVisible() || !this->isActiveWindow()) return;
        // Avoid redundant setFocus if already focused
        if (qApp->focusWidget() == m_search) return;
        // Only intervene when focus is cleared or moved to this popup's non-focusable children
        bool shouldRefocus = false;
        if (!now) {
            shouldRefocus = true;
        } else if ((now == this) || this->isAncestorOf(now)) {
            // Never allow preview subtree (including scrollbars) to keep focus
            const bool inPreview = (now == m_preview) || (now == m_previewScroll) ||
                                   (m_preview && m_preview->isAncestorOf(now)) ||
                                   (m_previewScroll && (m_previewScroll->isAncestorOf(now) ||
                                     now == m_previewScroll->verticalScrollBar() || now == m_previewScroll->horizontalScrollBar()));
            if (inPreview) shouldRefocus = true;
            // Also reclaim if the target child doesn't accept focus
            else if (now != m_search && now->focusPolicy() == Qt::NoFocus) shouldRefocus = true;
        }
        if (shouldRefocus && m_search) {
            QTimer::singleShot(0, [this]{ if (isVisible() && isActiveWindow()) m_search->setFocus(Qt::OtherFocusReason); });
        }
    });
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
    // Clear current selection and preview on every open
    if (m_list) {
        m_list->clearSelection();
        m_list->setCurrentIndex(QModelIndex());
    }
    if (m_preview) {
        m_preview->clear();
    }
    m_currentItemId = 0;
    // Also clear any previous search input when reopening
    if (m_search) {
        // Only trigger clear if non-empty to avoid redundant signals
        if (!m_search->text().isEmpty()) m_search->clear();
    }
    show();
    raise();
    activateWindow();
    m_search->setFocus();
}

void MainPopup::hidePopup() { hide(); emit popupHidden(); }

bool MainPopup::eventFilter(QObject* obj, QEvent* ev) {
    // Also react to palette/style changes that may arrive on some Qt builds
    if (ev->type() == QEvent::ApplicationPaletteChange || ev->type() == QEvent::PaletteChange || ev->type() == QEvent::StyleChange) {
        // Guard to avoid infinite restyling loops
        if (this->property("styleRefreshInProgress").toBool()) {
            return QWidget::eventFilter(obj, ev);
        }
        this->setProperty("styleRefreshInProgress", true);
        // Re-apply stylesheet which uses palette(...) so it repolishes correctly
        QTimer::singleShot(0, [this]{
            // Re-run the same initialization tweaks
            if (m_search) {
                m_search->setStyleSheet(
                    "QLineEdit{border:none;padding:8px 10px;background:palette(base);border-radius:6px;}"
                    "QLineEdit:focus{outline:none;}"
                );
            }
            const QString ss =
                " QScrollBar:vertical{width:4px;margin:0;background:transparent;}"
                " QScrollBar:horizontal{height:4px;margin:0;background:transparent;}"
                " QScrollBar::handle:vertical{background:palette(mid);min-height:24px;border-radius:2px;margin:0;}"
                " QScrollBar::handle:horizontal{background:palette(mid);min-width:24px;border-radius:2px;margin:0;}"
                " QScrollBar::add-line, QScrollBar::sub-line{height:0;width:0;background:transparent;}"
                " QScrollBar::add-page, QScrollBar::sub-page{background:transparent;}";
            this->setStyleSheet(ss);
            this->setProperty("styleRefreshInProgress", false);
        });
    }
    // Allow dragging the popup by grabbing the container background
    if (obj == m_container) {
        if (ev->type() == QEvent::MouseButtonPress) {
            auto* me = static_cast<QMouseEvent*>(ev);
            if (me->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragOffset = me->globalPosition().toPoint() - frameGeometry().topLeft();
                // Ensure search retains focus even when clicking empty container space (defer to avoid churn)
                if (m_search) QTimer::singleShot(0, [this]{ if (isVisible()) m_search->setFocus(Qt::MouseFocusReason); });
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
    if (obj == m_list && ev->type() == QEvent::MouseButtonPress) {
        // Clicking the list should not steal focus; keep it on the search box
        if (m_search) QTimer::singleShot(0, [this]{ if (isVisible()) m_search->setFocus(Qt::MouseFocusReason); });
        // Do not consume the event so selection still updates
        return QWidget::eventFilter(obj, ev);
    }
    if ((obj == m_preview || obj == m_previewScroll || obj == (m_previewScroll ? m_previewScroll->viewport() : nullptr))
        && ev->type() == QEvent::MouseButtonPress) {
        // Clicking preview area should not steal focus either
        if (m_search) QTimer::singleShot(0, [this]{ if (isVisible()) m_search->setFocus(Qt::MouseFocusReason); });
        return QWidget::eventFilter(obj, ev);
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
    // no floating heart; no viewport reposition needed
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
    // Restore selection after model resets if possible
    if (model) {
        connect(model, &QAbstractItemModel::modelReset, this, [this, model]{
            if (!m_list) return;
            qint64 prevId = m_currentItemId;
            QModelIndex target;
            if (prevId != 0) {
                const QModelIndex start = model->index(0, 0);
                const auto matches = model->match(start, HistoryListModel::IdRole, QVariant(prevId), 1, Qt::MatchExactly);
                if (!matches.isEmpty()) target = matches.first();
            }
            if (target.isValid()) {
                m_list->setCurrentIndex(target);
            } else {
                m_list->clearSelection();
                m_list->setCurrentIndex(QModelIndex());
                if (m_preview) m_preview->clear();
                m_currentItemId = 0;
            }
        });
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
    m_currentItemId = id;
    const bool fav = idx.data(Qt::UserRole + 5).toBool(); // FavoriteRole
    const int usage = idx.data(Qt::UserRole + 7).toInt(); // UsageCountRole
    const QString appName = idx.data(Qt::UserRole + 9).toString(); // AppNameRole
    if (type == "text") {
        const QString text = idx.data(Qt::UserRole + 3).toString(); // TextRole
        m_preview->showText(id, text, fav, usage, appName);
    } else {
        const QString path = idx.data(Qt::UserRole + 4).toString(); // MediaPathRole
        QImage img(path);
        if (!img.isNull()) {
            int w = m_previewScroll && m_previewScroll->viewport() ? m_previewScroll->viewport()->width() : m_preview->width();
            QString key = QString::number(id) + ":" + QString::number(w);
            QPixmap cached;
            if (QPixmapCache::find(key, &cached)) {
                m_preview->showImage(id, img, fav, usage, appName, cached);
            } else {
                m_preview->showImage(id, img, fav, usage, appName);
            }
        }
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
        if (m_search) m_search->setFocus(Qt::MouseFocusReason);
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
