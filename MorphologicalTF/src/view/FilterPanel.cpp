#include "view/FilterPanel.h"

#include "view/ConvolutionMatrixEditor.h"
#include "view/KernelPanel.h"
#include "viewmodel/ProcessingViewModel.h"
#include "viewmodel/KernelViewModel.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

// Each combo item stores one of these as its data
enum FilterType { LinearFilter, MedianFilter };

} // namespace

FilterPanel::FilterPanel(ProcessingViewModel &viewModel, ConvolutionViewModel &convolutionViewModel, KernelViewModel &kernelVM, QWidget *parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem(tr("Linear"), static_cast<int>(LinearFilter));
    m_typeCombo->addItem(tr("Median"), static_cast<int>(MedianFilter));

    auto *form = new QFormLayout;
    form->addRow(tr("Type:"), m_typeCombo);

    // The editor already has its own margins; drop them so they do not add up with ours
    m_matrixEditor = new ConvolutionMatrixEditor(convolutionViewModel, this);
    m_matrixEditor->layout()->setContentsMargins(0, 0, 0, 0);

    m_kernelEditor = new KernelPanel(kernelVM, this);
    m_kernelEditor->layout()->setContentsMargins(0, 0, 0, 0);

    m_applyButton = new QPushButton(tr("Apply"), this);
    connect(m_applyButton, &QPushButton::clicked, this, &FilterPanel::applyCurrent);
    connect(&m_viewModel, &ProcessingViewModel::canProcessChanged, this, &FilterPanel::updateEnabled);
    connect(m_typeCombo, &QComboBox::currentIndexChanged, this, &FilterPanel::updateType);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(m_matrixEditor);
    layout->addWidget(m_kernelEditor);
    layout->addWidget(m_applyButton);
    layout->addStretch(1);

    updateType();
    updateEnabled();
}

void FilterPanel::applyCurrent()
{
    switch (m_typeCombo->currentData().toInt()) {
    case LinearFilter:
        m_viewModel.applyFilter();
        break;
    case MedianFilter:
        m_viewModel.applyMedian();
        break;
    }
}

void FilterPanel::updateType()
{
    const bool linear = m_typeCombo->currentData().toInt() == LinearFilter;
    m_matrixEditor->setVisible(linear);
    m_kernelEditor->setVisible(!linear);
}

void FilterPanel::updateEnabled()
{
    m_applyButton->setEnabled(m_viewModel.canProcess());
}
