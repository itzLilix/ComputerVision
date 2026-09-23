#include "imagemodel.h"
#include <QDebug>
#include <QFileInfo>
#include <QColorSpace>
#include "../lib/easyexif/exif.h"

ImageModel::ImageModel(QObject* parent)
    : QObject(parent)
{
	m_fileMetadata = ImageFileMetadata();
}

void ImageModel::loadFile(const QString& path)
{
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        emit loadFailed(LoadError::FileNotFound, path);
        return;
    }

    QImage img(path);
    if (img.isNull()) {
        qDebug() << "Failed to load image:" << path;
        emit loadFailed(LoadError::InvalidFormat, path);
        return;
    }

    m_originalImage = std::move(img);
    m_currentImage = m_originalImage;

    m_fileMetadata.path = path;
    m_fileMetadata.fileSizeBytes = fileInfo.size();
    m_fileMetadata.lastModified = fileInfo.lastModified();
	m_fileMetadata.format = fileInfo.suffix().toUpper();

    m_fileMetadata.exif = collectExifData(path);

    emit newImageLoaded(m_currentImage, m_fileMetadata.path);
}

std::optional<ExifData> ImageModel::collectExifData(const QString& path)
{
    QFile imgFile(path);
    QByteArray imgBytes;
    imgBytes.reserve(imgFile.size());

    if (imgFile.open(QIODevice::ReadOnly)) {
        imgBytes = imgFile.read(imgFile.size());
        imgFile.close();
    }

    easyexif::EXIFInfo info;
    if (info.parseFrom(reinterpret_cast<const unsigned char*>(imgBytes.constData()),
        imgBytes.size()) != PARSE_EXIF_SUCCESS)
        return std::nullopt;

    ExifData data;
    data.dateTimeOriginal = QString::fromStdString(info.DateTimeOriginal);
    data.cameraMake = QString::fromStdString(info.Make);
    data.cameraModel = QString::fromStdString(info.Model);
    data.exposureTime = info.ExposureTime;
    data.fNumber = info.FNumber;
    data.isoSpeed = info.ISOSpeedRatings;
    data.orientation = info.Orientation;

    return data;
}

ImageLiveMetadata ImageModel::liveMetadata() const
{
    ImageLiveMetadata data;

    data.width = m_currentImage.width();
    data.height = m_currentImage.height();
    data.colorSpace = m_currentImage.colorSpace().description();
    data.colorModel = colorModelToQString(m_currentImage.colorSpace());
    data.colorDepth = m_currentImage.depth();

    return data;
}

void ImageModel::setImage(const QImage& image)
{
    m_currentImage = image;
    emit imageChanged(m_currentImage);
}

void ImageModel::closeImage()
{
    m_originalImage = QImage();
    m_currentImage = QImage();
    m_fileMetadata.path.clear();

    emit imageClosed();
}

void ImageModel::rotate(qreal angle)
{
	m_rotationAngle += angle;

    QTransform transform;
    transform.rotate(angle);

    m_currentImage = m_currentImage.transformed(transform, Qt::SmoothTransformation);
    m_originalImage = m_originalImage.transformed(transform, Qt::SmoothTransformation);

    emit imageChanged(m_currentImage);
}

void ImageModel::flip(bool horizontal, bool vertical)
{
	m_flipState.horizontal = horizontal;
	m_flipState.vertical = vertical;

	m_currentImage = m_currentImage.mirrored(horizontal, vertical);
	m_originalImage = m_originalImage.mirrored(horizontal, vertical);
	
    emit imageChanged(m_currentImage);
}

QString ImageModel::colorModelToQString(const QColorSpace& colorSpace) const
{
    switch (colorSpace.colorModel()) {
    case QColorSpace::ColorModel::Undefined:
        return tr("Undefined");

    case QColorSpace::ColorModel::Rgb:
        return tr("RGB");

    case QColorSpace::ColorModel::Gray:
        return tr("Grayscale");

    case QColorSpace::ColorModel::Cmyk:
        return tr("CMYK");

    default:
        return tr("Unknown");
    }
}