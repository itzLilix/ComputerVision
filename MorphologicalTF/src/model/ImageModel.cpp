#include "model/ImageModel.h"

#include <QImageReader>
#include <QImageWriter>

ImageModel::ImageModel(QObject *parent)
    : QObject(parent)
{
}

bool ImageModel::load(const QString &path, QString *error)
{
    QImageReader reader(path);
    reader.setAutoTransform(true); // apply EXIF orientation

    const QImage image = reader.read();
    if (image.isNull()) {
        if (error)
            *error = reader.errorString();
        return false;
    }

    // Normalize to a 32-bit format: every pixel is exactly 4 bytes,
    // which keeps the future per-pixel algorithms simple.
    // RGB32 and ARGB32 have the same memory layout.
    const QImage::Format format = image.hasAlphaChannel() ? QImage::Format_ARGB32
                                                          : QImage::Format_RGB32;
    m_original = image.convertToFormat(format);

    // QImage is implicitly shared: this does not copy the pixels yet,
    // but the data is detached automatically on the first write to m_current.
    m_current = m_original;

    m_filePath = path;
    m_modified = false;
    emit imageLoaded();
    emit imageChanged();
    return true;
}

bool ImageModel::save(const QString &path, QString *error)
{
    if (!hasImage()) {
        if (error)
            *error = QStringLiteral("There is no image to save.");
        return false;
    }

    QImageWriter writer(path); // the format is chosen by the file suffix
    writer.setQuality(95);     // used by JPEG/WebP, ignored by lossless formats
    if (!writer.write(m_current)) {
        if (error)
            *error = writer.errorString();
        return false;
    }

    m_modified = false;
    return true;
}

void ImageModel::close()
{
    if (!hasImage())
        return;

    m_original = QImage();
    m_current = QImage();
    m_filePath.clear();
    m_modified = false;
    emit imageCleared();
}

void ImageModel::setCurrent(const QImage &image)
{
    if (!hasImage() || image.isNull())
        return;

    m_current = image;
    m_modified = true;
    emit imageChanged();
}

void ImageModel::resetToOriginal()
{
    if (!hasImage())
        return;

    m_current = m_original;
    m_modified = false; // no unsaved edits remain
    emit imageChanged();
}
