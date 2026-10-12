#include "model/ImageModel.h"
#include "view/MainWindow.h"
#include "viewmodel/ImageViewModel.h"
#include "viewmodel/KernelViewModel.h"
#include "viewmodel/ConvolutionViewModel.h"
#include "viewmodel/ProcessingViewModel.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Composition root: the only place that knows all three layers.
    // Destruction order is the reverse: window -> view models -> model.
    ImageModel model;
    ImageViewModel viewModel(model);
    KernelViewModel kernelViewModel;
    ConvolutionViewModel convolutionViewModel;
    ProcessingViewModel processingViewModel(model, kernelViewModel, convolutionViewModel);
    MainWindow window(viewModel, kernelViewModel, convolutionViewModel, processingViewModel);
    window.show();

    return app.exec();
}
