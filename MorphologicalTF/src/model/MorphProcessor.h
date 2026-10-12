#pragma once
#include "Kernel.h"
#include "MorphologyOp.h"
#include <QImage>
#include <QList>
#include <QPoint>


namespace MorphProcessor {
	enum class Extremum { Min, Max };

	// Core: min (erosion) or max (dilation) over the given offsets
	template <Extremum E>
	QImage extremum(const QImage& input, const QList<QPoint>& offsets);

	inline QImage erode(const QImage& input, const Kernel& k) { return extremum<Extremum::Min>(input, k.offsets()); }
	inline QImage dilate(const QImage& input, const Kernel& k) { return extremum<Extremum::Max>(input, k.reflected().offsets()); }

	// Facade used by applyMorphology(): composes the above
	QImage apply(MorphologyOp op, const QImage& input, const Kernel& k);

	// Median filter: every channel becomes the median of the pixels under the active cells
	// of the window (any shape, size and anchor; KernelPresets can be used to build it).
	// For an even sample count the upper median is taken.
	// Borders: pixels outside the image are ignored. The alpha channel is kept as is.
	// Returns the input unchanged if the image is null or the window is empty.
	QImage median(const QImage& input, const Kernel& window);
}