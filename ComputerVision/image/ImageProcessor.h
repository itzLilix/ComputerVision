#pragma once

#include <QObject>
#include <QImage>

class ImageProcessor : public QObject
{
	Q_OBJECT
public:
	explicit ImageProcessor(QObject* parent = nullptr);

	QImage rotateImage(const QImage& src, qreal angleDegrees);
	QImage flipImage(const QImage& src, bool horizontal, bool vertical);
	//QImage scaleImage(const QImage& src, double scaleFactor);

	QImage adjustBrightness(const QImage& src, int delta);
	QImage adjustContrast(const QImage& src, double factor);
	QImage adjustSaturation(const QImage& src, double factor);
	QImage convertToGrayscale(const QImage& src);
	QImage invertColors(const QImage& src);
	QImage linearCorrection(const QImage& src);
	QImage gammaCorrection(const QImage& src, double factor);

private:
	static constexpr int RED_GRAYSCALE_WEIGHT = 54;    // ~0.2126 * 256
	static constexpr int GREEN_GRAYSCALE_WEIGHT = 183; // ~0.7152 * 256
	static constexpr int BLUE_GRAYSCALE_WEIGHT = 18;   // ~0.0722 * 256

	static QImage applyLut(const QImage& src, const std::array<uchar, 256>& lut);
};