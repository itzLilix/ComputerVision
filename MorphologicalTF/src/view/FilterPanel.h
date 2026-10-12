#pragma once

#include <QWidget>

class ConvolutionMatrixEditor;
class ConvolutionViewModel;
class ProcessingViewModel;
class QPushButton;

// Filter tool: the convolution matrix editor (its Preset combo drives the filter) and one Apply button.
// The result appears in the main image view through the usual imageChanged flow.
class FilterPanel : public QWidget
{
    Q_OBJECT

public:
    explicit FilterPanel(ProcessingViewModel &viewModel, ConvolutionViewModel &convolutionViewModel, QWidget *parent = nullptr);

private:
    void updateEnabled();

    ProcessingViewModel &m_viewModel;
    QPushButton *m_applyButton = nullptr;
};
