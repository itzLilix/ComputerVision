#include "model/ConvolutionPresets.h"

#include <QCoreApplication>

#include <algorithm>
#include <cmath>
#include <vector>

namespace ConvolutionPresets {

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double eps = 1e-9; // guards against rounding errors (cos(90 deg) is not exactly 0)

// Unsharp mask: identity + amount * (identity - gaussian), the weights sum to 1.
// The gaussian is centered on the anchor cell, so the image is not shifted.
void fillSharpen(ConvolutionMatrix &m)
{
    constexpr double amount = 2.0;

    const int w = m.width();
    const int h = m.height();
    const QPoint anchor = m.anchor();

    // The same sigma rule OpenCV uses for a given kernel size
    const double sigma = 0.3 * ((std::max(w, h) - 1) * 0.5 - 1.0) + 0.8;

    std::vector<double> gauss(static_cast<std::size_t>(w) * h);
    double total = 0.0;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const double dx = x - anchor.x();
            const double dy = y - anchor.y();
            const double g = std::exp(-(dx * dx + dy * dy) / (2.0 * sigma * sigma));
            gauss[static_cast<std::size_t>(y) * w + x] = g;
            total += g;
        }
    }

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double weight = -amount * gauss[static_cast<std::size_t>(y) * w + x] / total;
            if (x == anchor.x() && y == anchor.y())
                weight += 1.0 + amount;
            m.set(x, y, weight);
        }
    }
}

// A line through the anchor, sampled densely: every cell gets weight proportional
// to the number of samples that fall into it. The weights sum to 1.
void fillMotionBlur(ConvolutionMatrix &m, double angleDegrees)
{
    const int w = m.width();
    const int h = m.height();
    const QPoint anchor = m.anchor();

    const double rad = angleDegrees * kPi / 180.0;
    const double dirX = std::cos(rad);
    const double dirY = -std::sin(rad); // image Y axis points down

    const double half = std::max(w, h) / 2.0; // half of the line length, cells outside are skipped
    const int steps = 8 * std::max(w, h);     // even; t = 0 is always sampled (the anchor)

    std::vector<int> hits(static_cast<std::size_t>(w) * h, 0);
    int total = 0;
    for (int i = 0; i <= steps; ++i) {
        const double t = -half + 2.0 * half * i / steps;
        const int x = static_cast<int>(std::floor(anchor.x() + t * dirX + 0.5));
        const int y = static_cast<int>(std::floor(anchor.y() + t * dirY + 0.5));
        if (!m.contains(x, y))
            continue;
        ++hits[static_cast<std::size_t>(y) * w + x];
        ++total;
    }

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x)
            m.set(x, y, static_cast<double>(hits[static_cast<std::size_t>(y) * w + x]) / total);
    }
}

// Identity + directional gradient: the weight of a cell is its projection onto the direction.
// The direction is scaled so that its larger component is 1 (45 degrees gives the classic
// integer matrix). The gray level 128 is added so that negative relief stays visible.
void fillEmboss(ConvolutionMatrix &m, double angleDegrees)
{
    const int w = m.width();
    const int h = m.height();
    const QPoint anchor = m.anchor();

    const double rad = angleDegrees * kPi / 180.0;
    double dirX = std::cos(rad);
    double dirY = -std::sin(rad); // image Y axis points down
    const double norm = std::max(std::abs(dirX), std::abs(dirY)); // > 0 since cos^2 + sin^2 = 1
    dirX /= norm;
    dirY /= norm;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double weight = dirX * (x - anchor.x()) + dirY * (y - anchor.y());
            if (std::abs(weight) < eps)
                weight = 0.0;
            if (x == anchor.x() && y == anchor.y())
                weight += 1.0;
            m.set(x, y, weight);
        }
    }
    m.setBias(128.0);
}

} // namespace

const QList<ConvolutionPreset> &all()
{
    static const QList<ConvolutionPreset> presets = {
        ConvolutionPreset::Sharpen, ConvolutionPreset::MotionBlur, ConvolutionPreset::Emboss,
    };
    return presets;
}

QString displayName(ConvolutionPreset preset)
{
    switch (preset) {
    case ConvolutionPreset::Sharpen:    return QCoreApplication::translate("ConvolutionPresets", "Sharpen");
    case ConvolutionPreset::MotionBlur: return QCoreApplication::translate("ConvolutionPresets", "Motion Blur");
    case ConvolutionPreset::Emboss:     return QCoreApplication::translate("ConvolutionPresets", "Emboss");
    }
    return {};
}

bool usesAngle(ConvolutionPreset preset)
{
    return preset == ConvolutionPreset::MotionBlur || preset == ConvolutionPreset::Emboss;
}

double defaultAngle(ConvolutionPreset preset)
{
    return preset == ConvolutionPreset::Emboss ? 315.0 : 0.0;
}

ConvolutionMatrix create(ConvolutionPreset preset, int width, int height, double angleDegrees)
{
    ConvolutionMatrix matrix(width, height); // clamps the size, anchor in the center
    matrix.clear();

    switch (preset) {
    case ConvolutionPreset::Sharpen:    fillSharpen(matrix); break;
    case ConvolutionPreset::MotionBlur: fillMotionBlur(matrix, angleDegrees); break;
    case ConvolutionPreset::Emboss:     fillEmboss(matrix, angleDegrees); break;
    }
    return matrix;
}

} // namespace ConvolutionPresets
