#pragma once
#include "../image/ImageModel.h"

#include <QDialog>

class QTabWidget;
class QStackedWidget;
class QLabel;
class ImageModel;

class ImageInfoDialog : public QDialog
{
	Q_OBJECT
public:
	explicit ImageInfoDialog(QWidget* parent = nullptr, ImageModel* imgModel = nullptr);

private slots:
	void onNewImageLoaded(const QImage& image, const QString& filePath);
	void onImageChanged(const QImage& image);
	void onImageClosed();

private:
	ImageModel* m_imageModel;
    
    QTabWidget* m_tabs = nullptr;

    // General tab — file-derived
    QLabel* m_pathValue = nullptr;
    QLabel* m_sizeValue = nullptr;
    QLabel* m_formatValue = nullptr;
    QLabel* m_modifiedValue = nullptr;

    // General tab — live-derived
    QLabel* m_dimensionsValue = nullptr;
    QLabel* m_colorDepthValue = nullptr;
    QLabel* m_colorSpaceValue = nullptr;
    QLabel* m_colorModelValue = nullptr;

    // EXIF tab (stacked: form page <-> empty page)
    QStackedWidget* m_exifStack = nullptr;
    QLabel* m_dateTimeOriginalValue = nullptr;
    QLabel* m_cameraMakeValue = nullptr;
    QLabel* m_cameraModelValue = nullptr;
    QLabel* m_exposureTimeValue = nullptr;
    QLabel* m_fNumberValue = nullptr;
    QLabel* m_isoValue = nullptr;
    QLabel* m_orientationValue = nullptr;

    void buildUi();
    void refreshFileSection(const ImageFileMetadata& file);
    void refreshLiveSection(const ImageLiveMetadata& live);
    void refreshExifTab(const ImageFileMetadata& file);
    void clearAll();

    static QString formatFileSize(qint64 bytes);
    static QString formatExposureTime(qreal seconds);
};