#pragma once

#include <QObject>
#include <QImage>
#include "ImageMetadata.h"

enum class LoadError {
    FileNotFound,
    InvalidFormat
};

class ImageModel : public QObject {
    Q_OBJECT

public:
    explicit ImageModel(QObject* parent = nullptr);

    const QImage& image() const { return m_currentImage; }

public slots:
    void loadFile(const QString& path);
    void clearImage();

signals:
    void newImageLoaded(const QImage& image, const QString& m_filePath);
    void LoadFailed(LoadError error, const QString& filepath);
    void imageChanged(const QImage& image);
    void imageClosed();

private:
    QImage m_originalImage;
    QImage m_currentImage;
    ImageFileMetadata fileMetadata;
    ExifData exifData;
};