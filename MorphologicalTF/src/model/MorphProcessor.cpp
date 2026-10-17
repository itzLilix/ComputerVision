#include "MorphProcessor.h"
#include "model/Bounds.h"
#include "model/SlidingHistogram.h"

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
    QImage extremum(const QImage& input, const QList<QPoint>& offsets)
    {
        if (input.isNull() || offsets.isEmpty())
            return input;

        const QImage image = input.convertToFormat(QImage::Format_ARGB32);
        const int width = image.width();
        const int height = image.height();

        QImage output(image.size(), image.format());

        const auto* src = reinterpret_cast<const QRgb*>(image.constBits());
        auto* dst = reinterpret_cast<QRgb*>(output.bits());

        const int emptyValue = (E == Extremum::Min) ? 255 : 0;

        SlidingHistogram::slideRows(src, width, height, offsets,
            [&](int index, const SlidingHistogram::Histogram& r, const SlidingHistogram::Histogram& g,
                const SlidingHistogram::Histogram& b, int count) {
                    if (count == 0) {
                        dst[index] = qRgb(emptyValue, emptyValue, emptyValue);
                        return;
                    }

                    const int target = (E == Extremum::Min) ? 0 : count - 1;
                    dst[index] = qRgb(r.select(target), g.select(target), b.select(target));
            });

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