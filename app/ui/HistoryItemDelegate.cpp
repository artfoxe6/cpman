#include "HistoryItemDelegate.h"

#include <QPainter>
#include <QApplication>
#include <QStyle>
#include <QFileInfo>

// Role constants mirror HistoryListModel
static constexpr int RoleId = Qt::UserRole + 1;
static constexpr int RoleType = Qt::UserRole + 2; // "text" or "image"
static constexpr int RoleText = Qt::UserRole + 3;
static constexpr int RoleMediaPath = Qt::UserRole + 4;
static constexpr int RoleFavorite = Qt::UserRole + 5;

HistoryItemDelegate::HistoryItemDelegate(QObject* parent) : QStyledItemDelegate(parent) {
    m_thumbCache.setMaxCost(256); // up to 256 cached thumbs
}

QPixmap HistoryItemDelegate::loadThumb(const QModelIndex& index, int target) const {
    const qint64 id = index.data(RoleId).toLongLong();
    if (auto* c = m_thumbCache.object(id)) return *c;
    const QString path = index.data(RoleMediaPath).toString();
    QPixmap pm(path);
    if (!pm.isNull()) {
        QPixmap scaled = pm.scaled(target, target, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        auto* heapPm = new QPixmap(scaled);
        m_thumbCache.insert(id, heapPm, 1);
        return scaled;
    }
    return QPixmap();
}

void HistoryItemDelegate::paint(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx) const {
    QStyleOptionViewItem option = opt;
    initStyleOption(&option, idx);
    const QWidget* w = option.widget;
    QStyle* style = w ? w->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, p, w);

    const QRect r = option.rect.adjusted(8, 4, -8, -4);
    const QString type = idx.data(RoleType).toString();
    const bool fav = idx.data(RoleFavorite).toBool();
    const int iconSize = 48;

    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);

    if (type == "image") {
        QPixmap thumb = loadThumb(idx, iconSize);
        QRect imgRect(r.left(), r.top(), iconSize, iconSize);
        if (!thumb.isNull()) {
            QSize ts = thumb.size();
            QPoint topLeft = imgRect.topLeft();
            p->drawPixmap(QRect(topLeft, ts), thumb);
        } else {
            p->fillRect(imgRect, QColor(220,220,220));
        }
        QRect textRect = QRect(r.left() + iconSize + 8, r.top(), r.width() - iconSize - 8, r.height());
        QFont f = option.font;
        QFontMetrics fm(f);
        const QString label = QStringLiteral("图片  %1").arg(QFileInfo(idx.data(RoleMediaPath).toString()).fileName());
        p->setPen(option.palette.color(QPalette::Text));
        p->drawText(textRect, Qt::AlignVCenter | Qt::TextSingleLine, fm.elidedText(label, Qt::ElideRight, textRect.width()));
    } else {
        // text item: show two lines max
        const QString text = idx.data(RoleText).toString().replace('\n', ' ');
        QFont f = option.font;
        QFontMetrics fm(f);
        const int lineHeight = fm.height();
        QRect textRect = r;
        p->setPen(option.palette.color(QPalette::Text));
        QString el = fm.elidedText(text, Qt::ElideRight, textRect.width());
        p->drawText(textRect, Qt::AlignVCenter | Qt::TextSingleLine, el);
    }

    // 收藏状态不在列表中展示（仅预览窗口可切换/查看）
    p->restore();
}

QSize HistoryItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    const QString type = index.data(RoleType).toString();
    if (type == "image") return QSize(option.rect.width(), 56);
    QFontMetrics fm(option.font);
    return QSize(option.rect.width(), fm.height() + 12);
}
