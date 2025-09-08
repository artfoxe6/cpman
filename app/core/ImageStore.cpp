#include "ImageStore.h"

#include <QStandardPaths>
#include <QDir>
#include <QUuid>

ImageStore::ImageStore(QObject* parent) : QObject(parent) {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_mediaDir = base + "/media";
    m_thumbsDir = base + "/thumbs";
}

bool ImageStore::ensureDirs() {
    return QDir().mkpath(m_mediaDir) && QDir().mkpath(m_thumbsDir);
}

QString ImageStore::saveImage(const QImage& img, QString* outThumb, int* w, int* h) {
    if (w) *w = img.width();
    if (h) *h = img.height();
    ensureDirs();
    const QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString mediaPath = m_mediaDir + "/" + uuid + ".png";
    img.save(mediaPath, "PNG");
    // thumb longest side ~512
    QImage thumb = img;
    const int lw = std::max(thumb.width(), thumb.height());
    if (lw > 512) thumb = thumb.scaledToWidth(thumb.width() >= thumb.height() ? 512 : int(512.0 * thumb.width() / thumb.height()), Qt::SmoothTransformation);
    const QString thumbPath = m_thumbsDir + "/" + uuid + ".jpg";
    thumb.save(thumbPath, "JPG", 85);
    if (outThumb) *outThumb = thumbPath;
    return mediaPath;
}

QString ImageStore::thumbPathForMedia(const QString& mediaPath) const {
    QFileInfo fi(mediaPath);
    const QString base = fi.completeBaseName();
    return m_thumbsDir + "/" + base + ".jpg";
}

void ImageStore::removeMediaFiles(const QStringList& mediaPaths) {
    for (const auto& mp : mediaPaths) {
        if (mp.isEmpty()) continue;
        QFile::remove(mp);
        QFile::remove(thumbPathForMedia(mp));
    }
}
