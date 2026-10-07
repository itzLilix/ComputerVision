#pragma once

#include <QWidget>

class ConvolutionMatrixEditor;
class ProcessingViewModel;
class QComboBox;
class QDoubleSpinBox;
class QPushButton;
class QSpinBox;
class QStackedWidget;

// Picks an operation, shows its parameters and sends one command to the view model.
// The result appears in the main image view through the usual imageChanged flow.
class ProcessingPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ProcessingPanel(ProcessingViewModel &viewModel, QWidget *parent = nullptr);

private:
    void applyCurrent();
    void updateEnabled();

    ProcessingViewModel &m_viewModel;
    QComboBox *m_operationCombo = nullptr;
    QStackedWidget *m_pages = nullptr;
    QPushButton *m_applyButton = nullptr;

    QDoubleSpinBox *m_sharpenAmount = nullptr;
    QSpinBox *m_blurLength = nullptr;
    QSpinBox *m_blurAngle = nullptr;
    QSpinBox *m_embossAngle = nullptr;
    QDoubleSpinBox *m_embossStrength = nullptr;
    QSpinBox *m_medianSize = nullptr;
    ConvolutionMatrixEditor *m_matrixEditor = nullptr;
};
