#include "model/FilterProcessor.h"
#include "model/Bounds.h"
#include "model/SlidingHistogram.h"

#include <QPoint>
#include <QSet>
#include <QtConcurrent>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace FilterProcessor {

namespace {

// Bounding box of the offsets. It always includes (0, 0), even if the anchor
// is not on the shape, so the interior range below never leaves the image.


int toChannel(double value)
{
    return std::clamp(static_cast<int>(std::lround(value)), 0, 255);
}

} // namespace

QImage convolve(const QImage &input, const ConvolutionMatrix &matrix)
{
    // True convolution = correlation with the matrix rotated by 180 degrees
    const QList<ConvolutionMatrix::Tap> taps = matrix.reflected().taps();

    if (input.isNull() || taps.isEmpty())
        return input;

    const QImage image = input.convertToFormat(QImage::Format_ARGB32);

    const int width = image.width();
    const int height = image.height();

    QImage output(image.size(), image.format());

    const auto *src = reinterpret_cast<const QRgb *>(image.constBits());
    auto *dst = reinterpret_cast<QRgb *>(output.bits());

    const double scale = 1.0 / matrix.divisor();
    const double bias = matrix.bias();

    Bounds box;
    for (const auto &tap : taps)
        box.include(tap.offset);

    // ------------------------------------------------------------
    // Interior
    // ------------------------------------------------------------

    const int xBegin = -box.minDx;
    const int xEnd = width - 1 - box.maxDx;

    const int yBegin = -box.minDy;
    const int yEnd = height - 1 - box.maxDy;

    for (int y = yBegin; y <= yEnd; ++y) {
        const int rowStart = y * width;

        for (int x = xBegin; x <= xEnd; ++x) {
            const int index = rowStart + x;

            double sumR = 0.0;
            double sumG = 0.0;
            double sumB = 0.0;

            for (const auto &tap : taps) {
                const QRgb p = src[index + tap.offset.y() * width + tap.offset.x()];

                sumR += tap.weight * qRed(p);
                sumG += tap.weight * qGreen(p);
                sumB += tap.weight * qBlue(p);
            }

            dst[index] = qRgba(toChannel(sumR * scale + bias),
                               toChannel(sumG * scale + bias),
                               toChannel(sumB * scale + bias),
                               qAlpha(src[index]));
        }
    }

    // ------------------------------------------------------------
    // Border (edge pixels are replicated)
    // ------------------------------------------------------------

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (x >= xBegin && x <= xEnd &&
                y >= yBegin && y <= yEnd) {
                continue;
            }

            double sumR = 0.0;
            double sumG = 0.0;
            double sumB = 0.0;

            for (const auto &tap : taps) {
                const int sx = std::clamp(x + tap.offset.x(), 0, width - 1);
                const int sy = std::clamp(y + tap.offset.y(), 0, height - 1);

                const QRgb p = src[sy * width + sx];

                sumR += tap.weight * qRed(p);
                sumG += tap.weight * qGreen(p);
                sumB += tap.weight * qBlue(p);
            }

            const int index = y * width + x;
            dst[index] = qRgba(toChannel(sumR * scale + bias),
                               toChannel(sumG * scale + bias),
                               toChannel(sumB * scale + bias),
                               qAlpha(src[index]));
        }
    }

    return output;
}

QImage median(const QImage& input, const Kernel& window)
{
    const QList<QPoint> offsets = window.offsets();

    if (input.isNull() || offsets.isEmpty())
        return input;

    const QImage image = input.convertToFormat(QImage::Format_ARGB32);
    const int width = image.width();
    const int height = image.height();

    QImage output(image.size(), image.format());

    const auto* src = reinterpret_cast<const QRgb*>(image.constBits());
    auto* dst = reinterpret_cast<QRgb*>(output.bits());

    SlidingHistogram::slideRows(src, width, height, offsets,
        [&](int index, const SlidingHistogram::Histogram& r, const SlidingHistogram::Histogram& g,
            const SlidingHistogram::Histogram& b, int count) {
                dst[index] = (count == 0)
                    ? src[index]
                    : qRgba(r.select(count / 2), g.select(count / 2), b.select(count / 2),
                        qAlpha(src[index]));
        });

    return output;
}
} // namespace FilterProcessor
