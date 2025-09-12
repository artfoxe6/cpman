#pragma once

#include <QWidget>
#include <QPixmap>

class QLabel;
class QPushButton;

class Settings;

class PreviewPane : public QWidget {
    Q_OBJECT
public:
    explicit PreviewPane(QWidget* parent = nullptr);
    void attachSettings(Settings* settings);

    void showText(qint64 id, const QString& text, bool favorite, int usageCount, const QString& appName);
    void showImage(qint64 id, const QImage& img, bool favorite, int usageCount,
                   const QString& appName, const QPixmap& scaled = QPixmap());
    void setScaledPixmap(const QPixmap& pix);
    void clear();

    void triggerToggleFavorite();

signals:
    void favoriteToggled(qint64 id, bool on);

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;
    void resizeEvent(QResizeEvent* e) override;
    void changeEvent(QEvent* e) override;

private slots:
    void onHeart();

private:
    void updateHeart();
    void updateImageDisplay();
    void updateScaleLabel();
    void updateUsageLabel();
    void updateSourceLabel();
    void applySecondaryTextStyle();

    qint64 m_id = 0;
    bool m_favorite = false;
    bool m_isImage = false;
    bool m_fitToWidth = true;
    QLabel* m_textLabel = nullptr;
    QLabel* m_imageLabel = nullptr;
    class QScrollArea* m_contentScroll = nullptr;
    QPushButton* m_heart = nullptr;
    QLabel* m_scaleLabel = nullptr;
    QLabel* m_sourceLabel = nullptr;
    QLabel* m_usageLabel = nullptr;
    QPixmap m_imageOrig;
    Settings* m_settings = nullptr;
    int m_usageCount = 0;
    QString m_appName;
};
