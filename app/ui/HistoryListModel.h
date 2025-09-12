#pragma once

#include <QAbstractListModel>
#include <QVector>
#include "../core/Types.h"

class HistoryListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles { IdRole = Qt::UserRole + 1, TypeRole, TextRole, MediaPathRole, FavoriteRole, CreatedAtRole, UsageCountRole, HashRole, AppNameRole };

    explicit HistoryListModel(QObject* parent = nullptr);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(const QVector<HistoryItem>& items);
    const QVector<HistoryItem>& items() const { return m_items; }

public slots:
    void onItemUpdated(const HistoryItem& item);

private:
    QVector<HistoryItem> m_items;
};
