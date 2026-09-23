#include "ImageProcessor.h"
#include <QTransform>
#include <array>
#include <cmath>
#include <algorithm>

ImageProcessor::ImageProcessor(QObject* parent)
	: QObject(parent)
{
}

QImage ImageProcessor::rotateImage(const QImage& src, double angleDegrees)
{
	QTransform transform;
	transform.rotate(angleDegrees);
	return src.transformed(transform, Qt::SmoothTransformation);
}

QImage ImageProcessor::flipImage(const QImage& src, bool horizontal, bool vertical)
{
	return src.mirrored(horizontal, vertical);
}

//QImage ImageProcessor::scaleImage(const QImage& src, double scaleFactor)
//{
//	return src.scaled(src.width() * scaleFactor, src.height() * scaleFactor, Qt::KeepAspectRatio);
//}



QImage ImageProcessor::applyLut(const QImage& src, const std::array<uchar, 256>& lut)
{
	QImage result = src.convertToFormat(QImage::Format_ARGB32);
	const int width = result.width();
	const int height = result.height();

	for (int y = 0; y < height; ++y) {
		QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
		for (int x = 0; x < width; ++x) {
			QRgb px = line[x];
			line[x] = qRgba(lut[qRed(px)], lut[qGreen(px)], lut[qBlue(px)], qAlpha(px));
		}
	}
	return result;
}

QImage ImageProcessor::adjustBrightness(const QImage& src, int delta)
{
	if (delta == 0)
		return src;

	std::array<uchar, 256> lut;
	for (int i = 0; i < 256; ++i)
		lut[i] = static_cast<uchar>(qBound(0, i + delta, 255));

	return applyLut(src, lut);
}

QImage ImageProcessor::adjustContrast(const QImage& src, double factor)
{
	if (qFuzzyCompare(factor, 1.0))
		return src;

	std::array<uchar, 256> lut;
	for (int i = 0; i < 256; ++i) {
		int value = static_cast<int>((i - 128) * factor + 128.0);
		lut[i] = static_cast<uchar>(qBound(0, value, 255));
	}

	return applyLut(src, lut);
}

QImage ImageProcessor::invertColors(const QImage& src)
{
	std::array<uchar, 256> lut;
	for (int i = 0; i < 256; ++i)
		lut[i] = static_cast<uchar>(255 - i);

	return applyLut(src, lut);
}

QImage ImageProcessor::adjustSaturation(const QImage& src, double factor)
{
	QImage result = src.convertToFormat(QImage::Format_ARGB32);
	const int width = result.width();
	const int height = result.height();

	for (int y = 0; y < height; ++y) {
		QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
		for (int x = 0; x < width; ++x) {
			QRgb px = line[x];
			int gray = (qRed(px) * RED_GRAYSCALE_WEIGHT + qGreen(px) * GREEN_GRAYSCALE_WEIGHT + qBlue(px) * BLUE_GRAYSCALE_WEIGHT) >> 8;
			int newRed = qBound(0, static_cast<int>(gray + (qRed(px) - gray) * factor), 255);
			int newGreen = qBound(0, static_cast<int>(gray + (qGreen(px) - gray) * factor), 255);
			int newBlue = qBound(0, static_cast<int>(gray + (qBlue(px) - gray) * factor), 255);
			line[x] = qRgba(newRed, newGreen, newBlue, qAlpha(px));
		}
	}
	return result;
}

QImage ImageProcessor::convertToGrayscale(const QImage& src)
{
	QImage result = src.convertToFormat(QImage::Format_ARGB32);
	const int width = result.width();
	const int height = result.height();

	for (int y = 0; y < height; ++y) {
		QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
		for (int x = 0; x < width; ++x) {
			QRgb px = line[x];
			int gray = (qRed(px) * RED_GRAYSCALE_WEIGHT + qGreen(px) * GREEN_GRAYSCALE_WEIGHT + qBlue(px) * BLUE_GRAYSCALE_WEIGHT) >> 8;
			line[x] = qRgba(gray, gray, gray, qAlpha(px));
		}
	}
	return result;
}

QImage ImageProcessor::linearCorrection(const QImage& src)
{
	const QImage result = src.convertToFormat(QImage::Format_ARGB32);
	const int width = result.width();
	const int height = result.height();

	uchar minVal = 255, maxVal = 0;
	for (int y = 0; y < height; ++y) {
		auto* line = reinterpret_cast<const QRgb*>(result.constScanLine(y));
		for (int x = 0; x < width; ++x) {
			QRgb px = line[x];
			minVal = std::min({ minVal, uchar(qRed(px)), uchar(qGreen(px)), uchar(qBlue(px)) });
			maxVal = std::max({ maxVal, uchar(qRed(px)), uchar(qGreen(px)), uchar(qBlue(px)) });
		}
	}

	if (maxVal <= minVal) {
		return result;
	}

	std::array<uchar, 256> lut;
	for (int i = 0; i < 256; ++i) {
		int stretched = static_cast<int>((i - minVal) * 255.0 / (maxVal - minVal));
		lut[i] = static_cast<uchar>(qBound(0, stretched, 255));
	}

	return applyLut(result, lut);
}

QImage ImageProcessor::gammaCorrection(const QImage& src, double factor)
{
	if (qFuzzyCompare(factor, 1.0))
		return src;

	std::array<uchar, 256> lut;
	for (int i = 0; i < 256; ++i) {
		double normalized = i / 255.0;
		double corrected = std::pow(normalized, factor);
		lut[i] = static_cast<uchar>(qBound(0.0, corrected * 255.0, 255.0));
	}

	return applyLut(src, lut);
}