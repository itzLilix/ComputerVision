#pragma once

#include <QWidget>

class KernelGridView;
class KernelViewModel;
class QComboBox;
class QPushButton;
class QSpinBox;

// Structuring element editor: preset selector, size spinners, the grid and the tool buttons.
class KernelPanel : public QWidget
{
    Q_OBJECT

public:
    explicit KernelPanel(KernelViewModel &viewModel, QWidget *parent = nullptr);

private:
    void syncKernel(); // size spinners and grid <- view model
    void syncPreset(); // preset combo <- view model

    KernelViewModel &m_viewModel;
    QComboBox *m_presetCombo = nullptr;
    QSpinBox *m_widthSpin = nullptr;
    QSpinBox *m_heightSpin = nullptr;
    KernelGridView *m_grid = nullptr;
    QPushButton *m_anchorButton = nullptr;
};
