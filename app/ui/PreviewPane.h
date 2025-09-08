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

    void showText(qint64 id, const QString& text, bool favorite);
    void showImage(qint64 id, const QImage& img, bool favorite);

    void triggerToggleFavorite();

signals:
    void favoriteToggled(qint64 id, bool on);

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;
    void resizeEvent(QResizeEvent* e) override;

private slots:
    void onHeart();

private:
    void updateHeart();
    void updateImageDisplay();
    void updateScaleLabel();

    qint64 m_id = 0;
    bool m_favorite = false;
    bool m_isImage = false;
    bool m_fitToWidth = true;
    QLabel* m_textLabel = nullptr;
    QLabel* m_imageLabel = nullptr;
    QPushButton* m_heart = nullptr;
    QLabel* m_scaleLabel = nullptr;
    QPixmap m_imageOrig;
    Settings* m_settings = nullptr;
};
