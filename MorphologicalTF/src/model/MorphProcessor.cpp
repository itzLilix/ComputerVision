#include "MorphProcessor.h"

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
        int minDx = 0;
        int maxDx = 0;
        int minDy = 0;
        int maxDy = 0;

        bool first = true;

        for (const QPoint& p : offsets)
        {
            if (first)
            {
                minDx = maxDx = p.x();
                minDy = maxDy = p.y();
                first = false;
            }
            else
            {
                minDx = std::min(minDx, p.x());
                maxDx = std::max(maxDx, p.x());
                minDy = std::min(minDy, p.y());
                maxDy = std::max(maxDy, p.y());
            }
        }

        // ------------------------------------------------------------
        // Interior
        // ------------------------------------------------------------

        const int xBegin = -minDx;
        const int xEnd = width - 1 - maxDx;

        const int yBegin = -minDy;
        const int yEnd = height - 1 - maxDy;

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
};