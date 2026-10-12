#pragma once

#include "model/ConvolutionMatrix.h"

#include <QList>
#include <QString>

// Median is not here on purpose: it is not a convolution (see FilterProcessor::median).
// Its window is a plain Kernel, so KernelPresets can be reused for it.
enum class ConvolutionPreset { Sharpen, MotionBlur, Emboss };

namespace ConvolutionPresets {

const QList<ConvolutionPreset> &all();
QString displayName(ConvolutionPreset preset);

// Does the preset depend on the angle parameter (to enable/disable the angle input)
bool usesAngle(ConvolutionPreset preset);

// Angle the preset starts with (MotionBlur: 0 = horizontal, Emboss: 315 = classic look)
double defaultAngle(ConvolutionPreset preset);

// Builds the matrix for any size (not necessarily square or odd).
// The anchor is in the center. For even sizes the shapes are built around the anchor cell,
// so the result is not shifted.
// angleDegrees is counterclockwise from the +x axis, ignored by Sharpen:
//   MotionBlur - direction of the blur line (0 = horizontal);
//   Emboss     - direction of the relief gradient (315 gives the classic
//                [-2 -1 0; -1 1 1; 0 1 2] for 3x3).
ConvolutionMatrix create(ConvolutionPreset preset, int width, int height, double angleDegrees = 0.0);

} // namespace ConvolutionPresets
