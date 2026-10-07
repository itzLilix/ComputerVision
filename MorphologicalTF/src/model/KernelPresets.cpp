#include "model/KernelPresets.h"

#include <QCoreApplication>

#include <cmath>

namespace KernelPresets {

const QList<KernelPreset> &all()
{
    static const QList<KernelPreset> presets = {
        KernelPreset::Rectangle, KernelPreset::Cross, KernelPreset::Ellipse,
        KernelPreset::Diamond,   KernelPreset::Row,   KernelPreset::Column,
    };
    return presets;
}

QString displayName(KernelPreset preset)
{
    switch (preset) {
    case KernelPreset::Rectangle: return QCoreApplication::translate("KernelPresets", "Rectangle");
    case KernelPreset::Cross:     return QCoreApplication::translate("KernelPresets", "Cross");
    case KernelPreset::Ellipse:   return QCoreApplication::translate("KernelPresets", "Ellipse");
    case KernelPreset::Diamond:   return QCoreApplication::translate("KernelPresets", "Diamond");
    case KernelPreset::Row:       return QCoreApplication::translate("KernelPresets", "Row");
    case KernelPreset::Column:    return QCoreApplication::translate("KernelPresets", "Column");
    }
    return {};
}

Kernel create(KernelPreset preset, int width, int height)
{
    Kernel kernel(width, height); // clamps the size, anchor in the center
    const int w = kernel.width();
    const int h = kernel.height();
    const QPoint anchor = kernel.anchor();

    // Geometric center of the grid and the semi-axes of the round shapes.
    // For even sizes the center lies between two cells, so round shapes stay symmetric,
    // while Cross/Row/Column are one cell thick and pass through the anchor cell.
    const double cx = (w - 1) / 2.0;
    const double cy = (h - 1) / 2.0;
    const double rx = w / 2.0;
    const double ry = h / 2.0;
    constexpr double eps = 1e-9; // guards against rounding errors on the boundary

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const double dx = std::abs(x - cx) / rx; // normalized distance from the center
            const double dy = std::abs(y - cy) / ry;

            bool active = false;
            switch (preset) {
            case KernelPreset::Rectangle: active = true; break;
            case KernelPreset::Cross:     active = (x == anchor.x() || y == anchor.y()); break;
            case KernelPreset::Ellipse:   active = dx * dx + dy * dy <= 1.0 + eps; break;
            case KernelPreset::Diamond:   active = dx + dy <= 1.0 + eps; break;
            case KernelPreset::Row:       active = (y == anchor.y()); break;
            case KernelPreset::Column:    active = (x == anchor.x()); break;
            }
            kernel.set(x, y, active);
        }
    }
    return kernel;
}

} // namespace KernelPresets
