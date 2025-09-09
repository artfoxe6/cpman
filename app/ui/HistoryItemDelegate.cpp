#include "HistoryItemDelegate.h"

#include <QPainter>
#include <QApplication>
#include <QStyle>
#include <QFileInfo>
#include <QPainterPath>

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
    // Custom background with rounded hover/selection capsule
    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    QRect bgRect = option.rect.adjusted(6, 2, -6, -2);
    const qreal radius = 6.0;
    if (option.state & QStyle::State_Selected) {
        p->setBrush(option.palette.highlight());
        p->setPen(Qt::NoPen);
        p->drawRoundedRect(bgRect, radius, radius);
    } else if (option.state & QStyle::State_MouseOver) {
        QColor hov = option.palette.highlight().color();
        hov.setAlpha(28);
        p->setBrush(hov);
        p->setPen(Qt::NoPen);
        p->drawRoundedRect(bgRect, radius, radius);
    } else {
        // default panel background
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, p, w);
    }

    const QRect r = option.rect.adjusted(12, 6, -12, -6);
    const QString type = idx.data(RoleType).toString();
    const bool fav = idx.data(RoleFavorite).toBool();
    Q_UNUSED(fav);
    const int iconSize = 44;

    if (type == "image") {
        QPixmap thumb = loadThumb(idx, iconSize);
        QRect imgRect(r.left(), r.top(), iconSize, iconSize);
        // Rounded thumb background
        QPainterPath path; path.addRoundedRect(imgRect, 4, 4);
        p->setClipPath(path);
        if (!thumb.isNull()) {
            p->drawPixmap(imgRect, thumb);
        } else {
            p->fillRect(imgRect, option.palette.mid());
        }
        p->setClipping(false);
        QRect textRect = QRect(r.left() + iconSize + 10, r.top(), r.width() - iconSize - 10, r.height());
        QFont f = option.font;
        QFontMetrics fm(f);
        QString base = QFileInfo(idx.data(RoleMediaPath).toString()).fileName();
        if (base.isEmpty()) base = QStringLiteral("图片");
        const QString label = base;
        p->setPen((option.state & QStyle::State_Selected)
                      ? option.palette.highlightedText().color()
                      : option.palette.color(QPalette::Text));
        p->drawText(textRect, Qt::AlignVCenter | Qt::TextSingleLine,
                    fm.elidedText(label, Qt::ElideRight, textRect.width()));
    } else {
        // text item: show two lines max
        const QString text = idx.data(RoleText).toString().replace('\n', ' ');
        QFont f = option.font;
        QFontMetrics fm(f);
        QRect textRect = r;
        p->setPen((option.state & QStyle::State_Selected)
                      ? option.palette.highlightedText().color()
                      : option.palette.color(QPalette::Text));
        // Single line elided for performance and clarity
        QString el = fm.elidedText(text, Qt::ElideRight, textRect.width());
        p->drawText(textRect, Qt::AlignVCenter | Qt::TextSingleLine, el);
    }

    // 收藏状态不在列表中展示（仅预览窗口可切换/查看）
    p->restore();
}

QSize HistoryItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    const QString type = index.data(RoleType).toString();
    if (type == "image") return QSize(option.rect.width(), 60);
    QFontMetrics fm(option.font);
    return QSize(option.rect.width(), fm.height() + 16);
}
