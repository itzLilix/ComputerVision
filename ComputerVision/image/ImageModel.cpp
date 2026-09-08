#include "imagemodel.h"
#include <QDebug>
#include <QFileInfo>

ImageModel::ImageModel(QObject* parent)
    : QObject(parent)
{
	fileMetadata = ImageFileMetadata();
}

void ImageModel::loadFile(const QString& path)
{
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        emit LoadFailed(LoadError::FileNotFound, path);
        return;
    }

    QImage img(path);
    if (img.isNull()) {
        qDebug() << "Failed to load image:" << path;
        emit LoadFailed(LoadError::InvalidFormat, path);
        return;
    }

    m_originalImage = std::move(img);
    m_currentImage = m_originalImage;

    fileMetadata.path = path;
    fileMetadata.fileSizeBytes = fileInfo.size();
    fileMetadata.lastModified = fileInfo.lastModified();
	fileMetadata.format = fileInfo.suffix().toLower();

    emit newImageLoaded(m_currentImage, fileMetadata.path);
}

void ImageModel::clearImage()
{
    m_originalImage = QImage();
    m_currentImage = QImage();
    fileMetadata.path.clear();

    emit imageClosed();
}