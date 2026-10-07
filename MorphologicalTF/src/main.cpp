#include "model/ImageModel.h"
#include "view/MainWindow.h"
#include "viewmodel/ImageViewModel.h"
#include "viewmodel/KernelViewModel.h"
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
    ProcessingViewModel processingViewModel(model, kernelViewModel);
    MainWindow window(viewModel, kernelViewModel, processingViewModel);
    window.show();

    return app.exec();
}
