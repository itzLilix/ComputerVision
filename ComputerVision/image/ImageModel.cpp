#include "imagemodel.h"
#include <QDebug>

ImageModel::ImageModel(QObject* parent)
    : QObject(parent)
{
}

bool ImageModel::loadFile(const QString path)
{
    QImage img(path);
    if (img.isNull()) {
        qDebug() << "Failed to load image:" << path;
        return false;
    }

    m_originalImage = std::move(img);
    m_currentImage = m_originalImage;
    m_filePath = path;

    emit newImageLoaded(m_currentImage, m_filePath);
    return true;
}

void ImageModel::clearImage()
{
    m_originalImage = QImage();
    m_currentImage = QImage();
    m_filePath.clear();

    emit imageCleared();
}