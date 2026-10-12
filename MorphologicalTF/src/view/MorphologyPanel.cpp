#include "view/MorphologyPanel.h"

#include "view/KernelPanel.h"
#include "viewmodel/ProcessingViewModel.h"

#include <QComboBox>
#include <QFormLayout>
#include <QPushButton>
#include <QVBoxLayout>

MorphologyPanel::MorphologyPanel(ProcessingViewModel &viewModel, KernelViewModel &kernelViewModel, QWidget *parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    // Operation selector: every item stores its MorphologyOp value
    m_operationCombo = new QComboBox(this);
    for (MorphologyOp op : MorphologyOps::all())
        m_operationCombo->addItem(MorphologyOps::displayName(op), static_cast<int>(op));

    auto *form = new QFormLayout;
    form->addRow(tr("Operation:"), m_operationCombo);

    // The editor already has its own margins; drop them so they do not add up with ours
    auto *kernelPanel = new KernelPanel(kernelViewModel, this);
    kernelPanel->layout()->setContentsMargins(0, 0, 0, 0);

    m_applyButton = new QPushButton(tr("Apply"), this);
    connect(m_applyButton, &QPushButton::clicked, this, &MorphologyPanel::applyCurrent);
    connect(&m_viewModel, &ProcessingViewModel::canProcessChanged, this, &MorphologyPanel::updateEnabled);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(kernelPanel, 1);
    layout->addWidget(m_applyButton);

    updateEnabled();
}

void MorphologyPanel::applyCurrent()
{
    const QVariant data = m_operationCombo->currentData();
    if (data.isValid())
        m_viewModel.applyMorphology(static_cast<MorphologyOp>(data.toInt()));
}

void MorphologyPanel::updateEnabled()
{
    m_applyButton->setEnabled(m_viewModel.canProcess());
}
