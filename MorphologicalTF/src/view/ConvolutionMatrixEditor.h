#pragma once

#include <QWidget>

class ConvolutionViewModel;
class MatrixCellDelegate;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QPushButton;
class QSpinBox;
class QTableWidget;

// Convolution matrix editor: preset selector, size and angle, the weight table, the tool
// buttons, divisor and offset. Same structure as KernelPanel: it holds no state of its own,
// sends commands to the view model and redraws itself on its signals.
class ConvolutionMatrixEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ConvolutionMatrixEditor(ConvolutionViewModel &viewModel, QWidget *parent = nullptr);

private:
    void syncMatrix();      // size, table, anchor, divisor and offset <- view model
    void syncPreset();      // preset combo and angle <- view model
    void syncAutoDivisor(); // "Auto" checkbox and divisor enabled state <- view model
    void updateTableHeight();

    ConvolutionViewModel &m_viewModel;
    QComboBox *m_presetCombo = nullptr;
    QSpinBox *m_widthSpin = nullptr;
    QSpinBox *m_heightSpin = nullptr;
    QDoubleSpinBox *m_angleSpin = nullptr;
    QTableWidget *m_table = nullptr;
    MatrixCellDelegate *m_delegate = nullptr;
    QPushButton *m_anchorButton = nullptr;
    QCheckBox *m_autoDivisor = nullptr;
    QDoubleSpinBox *m_divisorSpin = nullptr;
    QDoubleSpinBox *m_offsetSpin = nullptr;
};
