#pragma once

#include <QWidget>

class ConvolutionMatrixEditor;
class KernelPanel;
class ConvolutionViewModel;
class KernelViewModel;
class ProcessingViewModel;
class QComboBox;
class QLabel;
class QPushButton;

// Filter tool: a type combo (Linear / Median) and one Apply button.
// Linear shows the convolution matrix editor; Median uses the structuring element
// from the Morphology panel as its window, so it only shows a hint.
// The result appears in the main image view through the usual imageChanged flow.
class FilterPanel : public QWidget
{
    Q_OBJECT

public: 
    explicit FilterPanel(ProcessingViewModel &viewModel, ConvolutionViewModel &convolutionViewModel, KernelViewModel& kernelVM, QWidget *parent = nullptr);

private:
    void applyCurrent();
    void updateType();
    void updateEnabled();

    ProcessingViewModel &m_viewModel;
    QComboBox *m_typeCombo = nullptr;
    ConvolutionMatrixEditor *m_matrixEditor = nullptr;
    KernelPanel* m_kernelEditor = nullptr;
    QPushButton *m_applyButton = nullptr;
};
