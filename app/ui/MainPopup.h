#pragma once

#include <QWidget>
#include <QModelIndex>

class QLineEdit;
class QCheckBox;
class QListView;
class QScrollArea;
class QAbstractItemModel;
class PreviewPane;
class Settings;

class MainPopup : public QWidget {
    Q_OBJECT
public:
    explicit MainPopup(QWidget* parent = nullptr);
    void attachSettings(class Settings* settings);
    void showPopup();
    void hidePopup();

    void setListModel(QAbstractItemModel* model);
    QModelIndex currentIndex() const;
    void setCurrentIndex(const QModelIndex& idx);

    QString queryText() const;
    bool useDbChecked() const;
    bool onlyFavChecked() const;

    PreviewPane* previewPane() const { return m_preview; }

    // Apply a default size from settings or UI change.
    // If visible, resizes immediately; otherwise, re-applies on next show.
    void applyDefaultSize(const QSize& sz);

signals:
    void requestSearch(QString query, bool useDb, bool onlyFav);
    void commitRequested();
    void popupHidden();

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;
    void focusOutEvent(QFocusEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;

private:
    QWidget* m_container = nullptr;
    QLineEdit* m_search = nullptr;
    QCheckBox* m_useDb = nullptr;
    QCheckBox* m_onlyFav = nullptr;
    QListView* m_list = nullptr;
    QScrollArea* m_previewScroll = nullptr;
    PreviewPane* m_preview = nullptr;
    void updatePreviewFromIndex(const QModelIndex& idx);

    bool m_dragging = false;
    QPoint m_dragOffset;
    bool m_sizedOnce = false;
    Settings* m_settings = nullptr;
    // Track current item id to restore selection across model resets
    qint64 m_currentItemId = 0;
};
