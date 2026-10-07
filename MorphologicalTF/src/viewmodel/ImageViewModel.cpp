#include "viewmodel/ImageViewModel.h"

#include "model/ImageModel.h"

#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QStringList>

namespace {

QString buildFilter(const QString &title, const QList<QByteArray> &formats)
{
    QStringList patterns;
    for (const QByteArray &format : formats)
        patterns << QStringLiteral("*.") + QString::fromLatin1(format).toLower();
    return QStringLiteral("%1 (%2)").arg(title, patterns.join(QLatin1Char(' ')));
}

} // namespace

ImageViewModel::ImageViewModel(ImageModel &model, QObject *parent)
    : QObject(parent)
    , m_model(model)
{
    // Relay model notifications to the view
    connect(&m_model, &ImageModel::imageLoaded, this, &ImageViewModel::imageLoaded);
    connect(&m_model, &ImageModel::imageChanged, this, &ImageViewModel::imageChanged);
    connect(&m_model, &ImageModel::imageCleared, this, &ImageViewModel::imageCleared);
}

bool ImageViewModel::hasImage() const
{
    return m_model.hasImage();
}

bool ImageViewModel::isModified() const
{
    return m_model.isModified();
}

const QImage &ImageViewModel::currentImage() const
{
    return m_model.current();
}

QString ImageViewModel::statusText() const
{
    if (!m_model.hasImage())
        return tr("No image loaded");

    const QImage &image = m_model.current();
    return tr("%1 - %2 x %3 px")
        .arg(QFileInfo(m_model.filePath()).fileName())
        .arg(image.width())
        .arg(image.height());
}

QString ImageViewModel::suggestedSaveName() const
{
    // Never suggest the source file itself, so the original is not overwritten by accident
    const QFileInfo info(m_model.filePath());
    return QDir(info.absolutePath()).filePath(info.completeBaseName() + QStringLiteral("_edited.png"));
}

QString ImageViewModel::openFileFilter()
{
    return buildFilter(tr("Images"), QImageReader::supportedImageFormats());
}

QString ImageViewModel::saveFileFilter()
{
    return buildFilter(tr("Images"), QImageWriter::supportedImageFormats());
}

bool ImageViewModel::isSupportedImageFile(const QString &path)
{
    const QFileInfo info(path);
    return info.isFile()
        && QImageReader::supportedImageFormats().contains(info.suffix().toLower().toLatin1());
}

void ImageViewModel::openImage(const QString &path)
{
    QString error;
    if (!m_model.load(path, &error))
        emit errorOccurred(tr("Cannot open \"%1\": %2").arg(QDir::toNativeSeparators(path), error));
}

void ImageViewModel::saveImage(const QString &path)
{
    QString target = path;
    if (QFileInfo(target).suffix().isEmpty())
        target += QStringLiteral(".png"); // the writer picks the format by suffix

    QString error;
    if (!m_model.save(target, &error))
        emit errorOccurred(tr("Cannot save \"%1\": %2").arg(QDir::toNativeSeparators(target), error));
}

void ImageViewModel::closeImage()
{
    m_model.close();
}

void ImageViewModel::resetToOriginal()
{
    m_model.resetToOriginal();
}
