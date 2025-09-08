#pragma once

#include <QObject>
#include <QString>
#include <QImage>

class ImageStore : public QObject {
    Q_OBJECT
public:
    explicit ImageStore(QObject* parent = nullptr);

    QString mediaDir() const { return m_mediaDir; }
    QString thumbsDir() const { return m_thumbsDir; }

    bool ensureDirs();
    // Save original and thumbnail; returns media path, sets width/height
    QString saveImage(const QImage& img, QString* outThumb = nullptr, int* w = nullptr, int* h = nullptr);

    QString thumbPathForMedia(const QString& mediaPath) const;
    void removeMediaFiles(const QStringList& mediaPaths);

private:
    QString m_mediaDir;
    QString m_thumbsDir;
};
