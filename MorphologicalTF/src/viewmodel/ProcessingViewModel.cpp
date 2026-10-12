#include "viewmodel/ProcessingViewModel.h"

#include "model/ImageModel.h"
#include "model/MorphProcessor.h"
#include "model/FilterProcessor.h"
#include "viewmodel/KernelViewModel.h"
#include "viewmodel/ConvolutionViewModel.h"

#include <QtConcurrent>

// SKELETON: the state handling is done, the operations themselves are not.
// Until a method is implemented it reports "not implemented" instead of silently doing nothing.

ProcessingViewModel::ProcessingViewModel(ImageModel &imageModel, const KernelViewModel &kernelViewModel, const ConvolutionViewModel &convolutionViewModel,
                                         QObject *parent)
    : QObject(parent)
    , m_imageModel(imageModel)
    , m_kernelViewModel(kernelViewModel)
    , m_convolutionViewModel(convolutionViewModel)
{
    m_watcher = new QFutureWatcher<void>(this);
    connect(m_watcher, &QFutureWatcher<void>::finished, this, [=] { setBusy(false); });

    connect(&m_imageModel, &ImageModel::imageLoaded, this, &ProcessingViewModel::canProcessChanged);
    connect(&m_imageModel, &ImageModel::imageCleared, this, &ProcessingViewModel::canProcessChanged);
}

bool ProcessingViewModel::canProcess() const
{
    return m_imageModel.hasImage() && !m_busy;
}

void ProcessingViewModel::setBusy(bool busy) {
    m_busy = busy;
    emit canProcessChanged();
}

void ProcessingViewModel::applyMorphology(MorphologyOp op)
{
    QFuture<void> future = QtConcurrent::run([this, op]{
        setBusy(true);
        m_imageModel.setCurrent(MorphProcessor::apply(op, m_imageModel.current(), m_kernelViewModel.kernel()));
    });
    m_watcher->setFuture(future);
}

void ProcessingViewModel::applyFilter()
{
    QFuture<void> future = QtConcurrent::run([this] {
        setBusy(true);
        m_imageModel.setCurrent(FilterProcessor::convolve(m_imageModel.current(), m_convolutionViewModel.matrix()));
        });
    m_watcher->setFuture(future);
}
