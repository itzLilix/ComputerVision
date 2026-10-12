#include "view/FilterPanel.h"

#include "view/ConvolutionMatrixEditor.h"
#include "viewmodel/ProcessingViewModel.h"

#include <QPushButton>
#include <QVBoxLayout>

FilterPanel::FilterPanel(ProcessingViewModel &viewModel, ConvolutionViewModel &convolutionViewModel, QWidget *parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    // The editor already has its own margins; drop them so they do not add up with ours
    auto *editor = new ConvolutionMatrixEditor(convolutionViewModel, this);
    editor->layout()->setContentsMargins(0, 0, 0, 0);

    m_applyButton = new QPushButton(tr("Apply"), this);
    connect(m_applyButton, &QPushButton::clicked, &m_viewModel, &ProcessingViewModel::applyFilter);
    connect(&m_viewModel, &ProcessingViewModel::canProcessChanged, this, &FilterPanel::updateEnabled);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(editor);
    layout->addWidget(m_applyButton);
    layout->addStretch(1);

    updateEnabled();
}

void FilterPanel::updateEnabled()
{
    m_applyButton->setEnabled(m_viewModel.canProcess());
}
