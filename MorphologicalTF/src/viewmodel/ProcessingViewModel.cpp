#include "viewmodel/ProcessingViewModel.h"

#include "model/ImageModel.h"
#include "model/MorphProcessor.h"
#include "viewmodel/KernelViewModel.h"

// SKELETON: the state handling is done, the operations themselves are not.
// Until a method is implemented it reports "not implemented" instead of silently doing nothing.

ProcessingViewModel::ProcessingViewModel(ImageModel &imageModel, const KernelViewModel &kernelViewModel,
                                         QObject *parent)
    : QObject(parent)
    , m_imageModel(imageModel)
    , m_kernelViewModel(kernelViewModel)
{
    connect(&m_imageModel, &ImageModel::imageLoaded, this, &ProcessingViewModel::canProcessChanged);
    connect(&m_imageModel, &ImageModel::imageCleared, this, &ProcessingViewModel::canProcessChanged);
}

bool ProcessingViewModel::canProcess() const
{
    return m_imageModel.hasImage() && !m_busy;
}

void ProcessingViewModel::applyMorphology(MorphologyOp op)
{
	m_imageModel.setCurrent(MorphProcessor::apply(op, m_imageModel.current(), m_kernelViewModel.kernel()));
}

void ProcessingViewModel::applySharpen(double amount)
{
    Q_UNUSED(amount);
    emit errorOccurred(tr("Sharpen is not implemented yet."));
}

void ProcessingViewModel::applyMotionBlur(int length, double angleDegrees)
{
    Q_UNUSED(length);
    Q_UNUSED(angleDegrees);
    emit errorOccurred(tr("Motion blur is not implemented yet."));
}

void ProcessingViewModel::applyEmboss(double angleDegrees, double strength)
{
    Q_UNUSED(angleDegrees);
    Q_UNUSED(strength);
    emit errorOccurred(tr("Emboss is not implemented yet."));
}

void ProcessingViewModel::applyMedian(int windowSize)
{
    Q_UNUSED(windowSize);
    emit errorOccurred(tr("Median filter is not implemented yet."));
}

void ProcessingViewModel::applyCustomFilter(int width, int height, const QVector<double> &weights,
                                            double divisor, double offset)
{
    Q_UNUSED(width);
    Q_UNUSED(height);
    Q_UNUSED(weights);
    Q_UNUSED(divisor);
    Q_UNUSED(offset);
    emit errorOccurred(tr("Custom filter is not implemented yet."));
}
