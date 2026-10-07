#pragma once

#include "model/Kernel.h"

#include <QList>
#include <QString>

enum class KernelPreset { Rectangle, Cross, Ellipse, Diamond, Row, Column };

namespace KernelPresets {

const QList<KernelPreset> &all();
QString displayName(KernelPreset preset);

// Builds the shape for any size (not necessarily square or odd).
// The anchor is in the center and always lies on the shape.
Kernel create(KernelPreset preset, int width, int height);

} // namespace KernelPresets
