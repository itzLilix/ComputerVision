#include "ImageInfo.h"
#include "../image/ImageModel.h"

#include <QTabWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QLocale>

ImageInfoDialog::ImageInfoDialog(QWidget* parent, ImageModel* imgModel)
    : QDialog(parent)
    , m_imageModel(imgModel)
{
    setWindowTitle(tr("Image Info"));
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::NonModal);

    buildUi();

    connect(m_imageModel, &ImageModel::newImageLoaded,
        this, &ImageInfoDialog::onNewImageLoaded);
    connect(m_imageModel, &ImageModel::imageChanged,
        this, &ImageInfoDialog::onImageChanged);
    connect(m_imageModel, &ImageModel::imageClosed,
        this, &ImageInfoDialog::onImageClosed);

    refreshFileSection(m_imageModel->fileMetadata());
    refreshExifTab(m_imageModel->fileMetadata());
    refreshLiveSection(m_imageModel->liveMetadata());
}

void ImageInfoDialog::buildUi()
{
    m_tabs = new QTabWidget(this);

    // ---- General tab ----
    auto* generalPage = new QWidget(m_tabs);
    auto* generalForm = new QFormLayout(generalPage);

    m_pathValue = new QLabel(generalPage);
    m_pathValue->setWordWrap(true);
    m_pathValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_sizeValue = new QLabel(generalPage);
    m_formatValue = new QLabel(generalPage);
    m_modifiedValue = new QLabel(generalPage);
    m_dimensionsValue = new QLabel(generalPage);
    m_colorDepthValue = new QLabel(generalPage);
    m_colorSpaceValue = new QLabel(generalPage);
    m_colorModelValue = new QLabel(generalPage);

    generalForm->addRow(tr("Path:"), m_pathValue);
    generalForm->addRow(tr("Size:"), m_sizeValue);
    generalForm->addRow(tr("Format:"), m_formatValue);
    generalForm->addRow(tr("Modified:"), m_modifiedValue);
    generalForm->addRow(tr("Dimensions:"), m_dimensionsValue);
    generalForm->addRow(tr("Color depth:"), m_colorDepthValue);
    generalForm->addRow(tr("Color space:"), m_colorSpaceValue);
    generalForm->addRow(tr("Color model:"), m_colorModelValue);

    m_tabs->addTab(generalPage, tr("General"));

    // ---- EXIF tab ----
    m_exifStack = new QStackedWidget(m_tabs);

    auto* exifForm = new QWidget(m_exifStack);
    auto* exifLayout = new QFormLayout(exifForm);

    m_dateTimeOriginalValue = new QLabel(exifForm);
    m_cameraMakeValue = new QLabel(exifForm);
    m_cameraModelValue = new QLabel(exifForm);
    m_exposureTimeValue = new QLabel(exifForm);
    m_fNumberValue = new QLabel(exifForm);
    m_isoValue = new QLabel(exifForm);
    m_orientationValue = new QLabel(exifForm);

    exifLayout->addRow(tr("Date taken:"), m_dateTimeOriginalValue);
    exifLayout->addRow(tr("Camera make:"), m_cameraMakeValue);
    exifLayout->addRow(tr("Camera model:"), m_cameraModelValue);
    exifLayout->addRow(tr("Exposure time:"), m_exposureTimeValue);
    exifLayout->addRow(tr("F-number:"), m_fNumberValue);
    exifLayout->addRow(tr("ISO speed:"), m_isoValue);
    exifLayout->addRow(tr("Orientation:"), m_orientationValue);

    auto* emptyPage = new QLabel(tr("No EXIF data available for this image."), m_exifStack);
    emptyPage->setAlignment(Qt::AlignCenter);
    emptyPage->setWordWrap(true);

    m_exifStack->addWidget(exifForm);  // 0: populated
    m_exifStack->addWidget(emptyPage); // 1: empty

    m_tabs->addTab(m_exifStack, tr("EXIF"));

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_tabs);
    resize(420, 360);
}

void ImageInfoDialog::onNewImageLoaded(const QImage& image, const QString& filePath)
{
    Q_UNUSED(image);

    setWindowTitle(tr("Image Info — %1").arg(QFileInfo(filePath).fileName()));

    const ImageFileMetadata file = m_imageModel->fileMetadata();
    refreshFileSection(file);
    refreshExifTab(file);
    refreshLiveSection(m_imageModel->liveMetadata());
}

void ImageInfoDialog::onImageChanged(const QImage& image)
{
    Q_UNUSED(image);
    refreshLiveSection(m_imageModel->liveMetadata());
}

void ImageInfoDialog::onImageClosed()
{
    clearAll();
}

void ImageInfoDialog::refreshFileSection(const ImageFileMetadata& file)
{
    m_pathValue->setText(file.path);
    m_sizeValue->setText(formatFileSize(file.fileSizeBytes));
    m_formatValue->setText(file.format);
    m_modifiedValue->setText(QLocale().toString(file.lastModified, QLocale::ShortFormat));
}

void ImageInfoDialog::refreshLiveSection(const ImageLiveMetadata& live)
{
    m_dimensionsValue->setText(tr("%1 × %2").arg(live.width).arg(live.height));
    m_colorDepthValue->setText(tr("%1-bit").arg(live.colorDepth));
    m_colorSpaceValue->setText(live.colorSpace.isEmpty() ? tr("Unknown") : live.colorSpace);
    m_colorModelValue->setText(live.colorModel);
}

void ImageInfoDialog::refreshExifTab(const ImageFileMetadata& file)
{
    if (!file.exif.has_value()) {
        m_exifStack->setCurrentIndex(1);
        return;
    }
    m_exifStack->setCurrentIndex(0);

    const ExifData& exif = *file.exif;
    m_dateTimeOriginalValue->setText(exif.dateTimeOriginal.isEmpty() ? tr("—") : exif.dateTimeOriginal);
    m_cameraMakeValue->setText(exif.cameraMake.isEmpty() ? tr("—") : exif.cameraMake);
    m_cameraModelValue->setText(exif.cameraModel.isEmpty() ? tr("—") : exif.cameraModel);
    m_exposureTimeValue->setText(formatExposureTime(exif.exposureTime));
    m_fNumberValue->setText(exif.fNumber > 0.0 ? tr("f/%1").arg(exif.fNumber, 0, 'g', 2) : tr("—"));
    m_isoValue->setText(exif.isoSpeed > 0 ? QString::number(exif.isoSpeed) : tr("—"));
    m_orientationValue->setText(QString::number(exif.orientation));
}

void ImageInfoDialog::clearAll()
{
    setWindowTitle(tr("Image Info"));

    m_pathValue->clear();
    m_sizeValue->clear();
    m_formatValue->clear();
    m_modifiedValue->clear();
    m_dimensionsValue->clear();
    m_colorDepthValue->clear();
    m_colorSpaceValue->clear();
    m_colorModelValue->clear();

    m_exifStack->setCurrentIndex(1);
}

QString ImageInfoDialog::formatFileSize(qint64 bytes)
{
    return QLocale().formattedDataSize(bytes);
}

QString ImageInfoDialog::formatExposureTime(qreal seconds)
{
    if (seconds <= 0.0)
        return tr("—");
    if (seconds >= 1.0)
        return tr("%1 s").arg(seconds, 0, 'g', 3);
    return tr("1/%1 s").arg(qRound(1.0 / seconds));
}