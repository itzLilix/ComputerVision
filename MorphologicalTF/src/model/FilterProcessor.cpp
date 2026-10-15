#include "model/FilterProcessor.h"
#include "model/Bounds.h"

#include <QPoint>

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

    Bounds box;
    for (const QPoint& o : offsets)
        box.include(o);

    // Reused for every pixel, so there are no allocations in the loops
    std::vector<std::uint8_t> reds;
    std::vector<std::uint8_t> greens;
    std::vector<std::uint8_t> blues;
    reds.reserve(offsets.size());
    greens.reserve(offsets.size());
    blues.reserve(offsets.size());

    auto medianOf = [](std::vector<std::uint8_t>& values) {
        const auto middle = values.begin() + values.size() / 2;
        std::nth_element(values.begin(), middle, values.end());
        return static_cast<int>(*middle);
        };

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

            reds.clear();
            greens.clear();
            blues.clear();

            for (const QPoint& o : offsets) {
                const QRgb p = src[index + o.y() * width + o.x()];

                reds.push_back(static_cast<std::uint8_t>(qRed(p)));
                greens.push_back(static_cast<std::uint8_t>(qGreen(p)));
                blues.push_back(static_cast<std::uint8_t>(qBlue(p)));
            }

            dst[index] = qRgba(medianOf(reds), medianOf(greens), medianOf(blues), qAlpha(src[index]));
        }
    }

    // ------------------------------------------------------------
    // Border (pixels outside the image are ignored)
    // ------------------------------------------------------------

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (x >= xBegin && x <= xEnd &&
                y >= yBegin && y <= yEnd) {
                continue;
            }

            reds.clear();
            greens.clear();
            blues.clear();

            for (const QPoint& o : offsets) {
                const int sx = x + o.x();
                const int sy = y + o.y();

                if (sx < 0 || sx >= width ||
                    sy < 0 || sy >= height) {
                    continue;
                }

                const QRgb p = src[sy * width + sx];

                reds.push_back(static_cast<std::uint8_t>(qRed(p)));
                greens.push_back(static_cast<std::uint8_t>(qGreen(p)));
                blues.push_back(static_cast<std::uint8_t>(qBlue(p)));
            }

            const int index = y * width + x;

            if (reds.empty()) { // the whole window is outside (anchor not on the shape)
                dst[index] = src[index];
                continue;
            }

            dst[index] = qRgba(medianOf(reds), medianOf(greens), medianOf(blues), qAlpha(src[index]));
        }
    }

    return output;
}
} // namespace FilterProcessor
