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
    const QImage& originalImage() const { return m_originalImage; }
    const ImageFileMetadata& fileMetadata() const { return m_fileMetadata; }
    ImageLiveMetadata liveMetadata() const;

public slots:
    void loadFile(const QString& path);
    void setImage(const QImage& image);
    void closeImage();
	void rotate(qreal angle);
	void flip(bool horizontal, bool vertical);

signals:
    void newImageLoaded(const QImage& image, const QString& m_filePath);
    void loadFailed(LoadError error, const QString& filepath);
    void imageChanged(const QImage& image);
    void imageClosed();

private:
    QImage m_originalImage;
    QImage m_currentImage;
    ImageFileMetadata m_fileMetadata;
	qreal m_rotationAngle = 0.0;
	struct flipState {
		bool horizontal = false;
		bool vertical = false;
	} m_flipState;

    std::optional<ExifData> collectExifData(const QString& path);
    QString colorModelToQString(const QColorSpace& colorSpace) const;
};