#pragma once

#include "model/ConvolutionMatrix.h"
#include "model/Kernel.h"

#include <QImage>

namespace FilterProcessor {

// True convolution: out(p) = sum of weight(o) * in(p - o), then / divisor + bias.
// Implemented like dilate(): the matrix is reflected and applied as a correlation.
// Borders: edge pixels are replicated. The alpha channel is kept as is.
// Returns the input unchanged if the image is null or the matrix has only zeros.
QImage convolve(const QImage &input, const ConvolutionMatrix &matrix);

// Median filter: every channel becomes the median of the pixels under the active cells
// of the window (any shape, size and anchor; KernelPresets can be used to build it).
// For an even sample count the upper median is taken.
// Borders: pixels outside the image are ignored. The alpha channel is kept as is.
// Returns the input unchanged if the image is null or the window is empty.
QImage median(const QImage& input, const Kernel& window);
} // namespace FilterProcessor
