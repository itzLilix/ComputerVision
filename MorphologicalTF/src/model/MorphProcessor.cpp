#include "MorphProcessor.h"
#include "model/Bounds.h"

QImage operator-(const QImage& a, const QImage& b) {
    if (a.size() != b.size() || a.format() != b.format()) {
        return a;
    }
    QImage result(a.size(), a.format());
    const int width = a.width();
    const int height = a.height();
    const auto* srcA = reinterpret_cast<const QRgb*>(a.constBits());
    const auto* srcB = reinterpret_cast<const QRgb*>(b.constBits());
    auto* dst = reinterpret_cast<QRgb*>(result.bits());
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int index = y * width + x;
            int r = std::clamp(qRed(srcA[index]) - qRed(srcB[index]), 0, 255);
            int g = std::clamp(qGreen(srcA[index]) - qGreen(srcB[index]), 0, 255);
            int b = std::clamp(qBlue(srcA[index]) - qBlue(srcB[index]), 0, 255);
            dst[index] = qRgb(r, g, b);
        }
    }
    return result;
}

namespace MorphProcessor {


    template <Extremum E>
    QImage extremum(
        const QImage& input,
        const QList<QPoint>& offsets)
    {
        if (input.isNull() || offsets.isEmpty())
            return input;

        QImage image = input.convertToFormat(QImage::Format_ARGB32);

        const int width = image.width();
        const int height = image.height();

        QImage output(image.size(), image.format());

        const auto* src =
            reinterpret_cast<const QRgb*>(image.constBits());

        auto* dst =
            reinterpret_cast<QRgb*>(output.bits());

        int defaultValue = (E == Extremum::Min) ? 255 : 0;

        // Kernel offsets relative to the anchor.
        Bounds box;
        for (const QPoint& o : offsets)
            box.include(o);

        // ------------------------------------------------------------
        // Interior
        // ------------------------------------------------------------

        const int xBegin = -box.minDx;
        const int xEnd = width - 1 - box.maxDx;

        const int yBegin = -box.minDy;
        const int yEnd = height - 1 - box.maxDy;

        for (int y = yBegin; y <= yEnd; ++y)
        {
            const int rowStart = y * width;

            for (int x = xBegin; x <= xEnd; ++x)
            {
                const int index = rowStart + x;

                int resR = defaultValue;
                int resG = defaultValue;
                int resB = defaultValue;

                for (const QPoint& o : offsets)
                {
                    const QRgb p = src[index + o.y() * width + o.x()];

                    if constexpr (E == Extremum::Min)
                    {
                        resR = std::min(resR, qRed(p));
                        resG = std::min(resG, qGreen(p));
                        resB = std::min(resB, qBlue(p));
                    }
                    else
                    {
                        resR = std::max(resR, qRed(p));
                        resG = std::max(resG, qGreen(p));
                        resB = std::max(resB, qBlue(p));
                    }
                }

                dst[index] = qRgb(resR, resG, resB);
            }
        }

        // ------------------------------------------------------------
        // Border
        // ------------------------------------------------------------

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                if (x >= xBegin && x <= xEnd &&
                    y >= yBegin && y <= yEnd)
                {
                    continue;
                }

                int resR = defaultValue;
                int resG = defaultValue;
                int resB = defaultValue;

                for (const QPoint& o : offsets)
                {
                    const int sx = x + o.x();
                    const int sy = y + o.y();

                    if (sx < 0 || sx >= width ||
                        sy < 0 || sy >= height)
                    {
                        continue;
                    }

                    const QRgb p = src[sy * width + sx];

                    if constexpr (E == Extremum::Min)
                    {
                        resR = std::min(resR, qRed(p));
                        resG = std::min(resG, qGreen(p));
                        resB = std::min(resB, qBlue(p));
                    }
                    else
                    {
                        resR = std::max(resR, qRed(p));
                        resG = std::max(resG, qGreen(p));
                        resB = std::max(resB, qBlue(p));
                    }
                }

                dst[y * width + x] = qRgb(resR, resG, resB);
            }
        }

        return output;
    }


    QImage apply(MorphologyOp op, const QImage& input, const Kernel& k) {
        switch (op)
        {
        case MorphologyOp::Erosion:
            return MorphProcessor::erode(input, k);
        case MorphologyOp::Dilation:
            return MorphProcessor::dilate(input, k);
        case MorphologyOp::Opening:
            return MorphProcessor::dilate(MorphProcessor::erode(input, k), k);
        case MorphologyOp::Closing:
            return MorphProcessor::erode(MorphProcessor::dilate(input, k), k);
        case MorphologyOp::Gradient:
            return MorphProcessor::dilate(input, k) - MorphProcessor::erode(input, k);
        case MorphologyOp::TopHat:
            return input - MorphProcessor::dilate(MorphProcessor::erode(input, k), k);
        case MorphologyOp::BlackHat:
            return MorphProcessor::dilate(MorphProcessor::erode(input, k), k) - input;
        default:
            return input;
        }
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
};