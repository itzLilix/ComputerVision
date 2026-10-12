#pragma once

#include <QWidget>

class KernelViewModel;
class ProcessingViewModel;
class QComboBox;
class QPushButton;

// Morphology tool: the operation combo on top (same place as the Preset combo in FilterPanel),
// the structuring element editor below, one Apply button.
// The result appears in the main image view through the usual imageChanged flow.
class MorphologyPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MorphologyPanel(ProcessingViewModel &viewModel, KernelViewModel &kernelViewModel, QWidget *parent = nullptr);

private:
    void applyCurrent();
    void updateEnabled();

    ProcessingViewModel &m_viewModel;
    QComboBox *m_operationCombo = nullptr;
    QPushButton *m_applyButton = nullptr;
};
