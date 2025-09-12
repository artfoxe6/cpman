#include "HistoryListModel.h"

HistoryListModel::HistoryListModel(QObject* parent) : QAbstractListModel(parent) {}

int HistoryListModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : m_items.size();
}

QVariant HistoryListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) return {};
    const auto& it = m_items.at(index.row());
    switch (role) {
        case Qt::DisplayRole: {
            if (it.type == ItemType::Text) {
                QString s = it.text;
                s.replace('\n', ' ');
                if (s.size() > 200) s = s.left(200) + QStringLiteral("…");
                return s;
            } else {
                return QStringLiteral("[图片] %1x%2").arg(it.width).arg(it.height);
            }
        }
        case Qt::ToolTipRole: {
            if (it.type == ItemType::Text) {
                QString s = it.text;
                if (s.size() > 1000) s = s.left(1000) + QStringLiteral("…");
                return s;
            } else {
                return QStringLiteral("图片 %1x%2\n%3").arg(it.width).arg(it.height).arg(it.mediaPath);
            }
        }
        case TextRole: return it.type == ItemType::Text ? it.text : QString();
        case IdRole: return it.id;
        case TypeRole: return it.type == ItemType::Text ? "text" : "image";
        case MediaPathRole: return it.mediaPath;
        case FavoriteRole: return it.favorite;
        case CreatedAtRole: return it.createdAt;
        case UsageCountRole: return it.usageCount;
        case HashRole: return it.hash;
        case AppNameRole: return it.appName;
        default: return {};
    }
}

QHash<int, QByteArray> HistoryListModel::roleNames() const {
    return {
        {IdRole, "id"},
        {TypeRole, "type"},
        {TextRole, "text"},
        {MediaPathRole, "mediaPath"},
        {FavoriteRole, "favorite"},
        {CreatedAtRole, "createdAt"},
        {UsageCountRole, "usageCount"},
        {HashRole, "hash"},
        {AppNameRole, "appName"}
    };
}

void HistoryListModel::setItems(const QVector<HistoryItem>& items) {
    if (items == m_items) return;
    beginResetModel();
    m_items = items;
    endResetModel();
}

void HistoryListModel::onItemUpdated(const HistoryItem& item) {
    for (int row = 0; row < m_items.size(); ++row) {
        if (m_items[row].id == item.id) {
            m_items[row] = item;
            QModelIndex idx = index(row, 0);
            emit dataChanged(idx, idx);
            break;
        }
    }
}
