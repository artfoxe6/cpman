#pragma once

#include <QStyledItemDelegate>
#include <QCache>
#include <QPixmap>

class HistoryItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit HistoryItemDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    mutable QCache<quint64, QPixmap> m_thumbCache; // key: item id
    QPixmap loadThumb(const QModelIndex& index, int target) const;
};
