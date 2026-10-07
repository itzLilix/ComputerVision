#include "view/ProcessingPanel.h"

#include "view/ConvolutionMatrixEditor.h"
#include "viewmodel/ProcessingViewModel.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace {

// Stack indices; each operation in the combo remembers which page it uses
enum Page { MorphologyPage, SharpenPage, MotionBlurPage, EmbossPage, MedianPage, CustomFilterPage };

constexpr int kPageRole = Qt::UserRole;
constexpr int kMorphologyOpRole = Qt::UserRole + 1;

QSpinBox *makeAngleSpin(QWidget *parent, int value)
{
    auto *spin = new QSpinBox(parent);
    spin->setRange(0, 359);
    spin->setWrapping(true);
    spin->setSuffix(QStringLiteral(" deg"));
    spin->setValue(value);
    return spin;
}

QDoubleSpinBox *makeFactorSpin(QWidget *parent)
{
    auto *spin = new QDoubleSpinBox(parent);
    spin->setRange(0.1, 10.0);
    spin->setSingleStep(0.1);
    spin->setDecimals(1);
    spin->setValue(1.0);
    return spin;
}

} // namespace

ProcessingPanel::ProcessingPanel(ProcessingViewModel &viewModel, QWidget *parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    // Operation list: morphology first, a separator, then the filters
    m_operationCombo = new QComboBox(this);
    const auto addOperation = [this](const QString &text, Page page, int morphologyOp = -1) {
        m_operationCombo->addItem(text);
        const int row = m_operationCombo->count() - 1;
        m_operationCombo->setItemData(row, static_cast<int>(page), kPageRole);
        m_operationCombo->setItemData(row, morphologyOp, kMorphologyOpRole);
    };
    for (MorphologyOp op : MorphologyOps::all())
        addOperation(MorphologyOps::displayName(op), MorphologyPage, static_cast<int>(op));
    m_operationCombo->insertSeparator(m_operationCombo->count());
    addOperation(tr("Sharpen"), SharpenPage);
    addOperation(tr("Motion Blur"), MotionBlurPage);
    addOperation(tr("Emboss"), EmbossPage);
    addOperation(tr("Median Filter"), MedianPage);
    addOperation(tr("Custom Matrix Filter"), CustomFilterPage);

    // Parameter pages, added in the order of the Page enum
    m_pages = new QStackedWidget(this);

    auto *morphologyPage = new QLabel(tr("Uses the matrix from the Structuring Element panel."), this);
    morphologyPage->setWordWrap(true);
    morphologyPage->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_pages->addWidget(morphologyPage);

    auto *sharpenPage = new QWidget(this);
    auto *sharpenForm = new QFormLayout(sharpenPage);
    sharpenForm->setContentsMargins(0, 0, 0, 0);
    m_sharpenAmount = makeFactorSpin(sharpenPage);
    sharpenForm->addRow(tr("Amount:"), m_sharpenAmount);
    m_pages->addWidget(sharpenPage);

    auto *blurPage = new QWidget(this);
    auto *blurForm = new QFormLayout(blurPage);
    blurForm->setContentsMargins(0, 0, 0, 0);
    m_blurLength = new QSpinBox(blurPage);
    m_blurLength->setRange(1, 200);
    m_blurLength->setValue(15);
    m_blurLength->setSuffix(QStringLiteral(" px"));
    m_blurAngle = makeAngleSpin(blurPage, 0);
    blurForm->addRow(tr("Length:"), m_blurLength);
    blurForm->addRow(tr("Angle:"), m_blurAngle);
    m_pages->addWidget(blurPage);

    auto *embossPage = new QWidget(this);
    auto *embossForm = new QFormLayout(embossPage);
    embossForm->setContentsMargins(0, 0, 0, 0);
    m_embossAngle = makeAngleSpin(embossPage, 135);
    m_embossStrength = makeFactorSpin(embossPage);
    embossForm->addRow(tr("Angle:"), m_embossAngle);
    embossForm->addRow(tr("Strength:"), m_embossStrength);
    m_pages->addWidget(embossPage);

    auto *medianPage = new QWidget(this);
    auto *medianForm = new QFormLayout(medianPage);
    medianForm->setContentsMargins(0, 0, 0, 0);
    m_medianSize = new QSpinBox(medianPage);
    m_medianSize->setRange(3, 31);
    m_medianSize->setSingleStep(2);
    m_medianSize->setKeyboardTracking(false); // validate after typing, not on every keystroke
    m_medianSize->setSuffix(QStringLiteral(" px"));
    m_medianSize->setToolTip(tr("Window size, always odd"));
    connect(m_medianSize, &QSpinBox::valueChanged, this, [this](int value) {
        if (value % 2 == 0)
            m_medianSize->setValue(value + 1);
    });
    medianForm->addRow(tr("Window:"), m_medianSize);
    m_pages->addWidget(medianPage);

    m_matrixEditor = new ConvolutionMatrixEditor(this);
    m_pages->addWidget(m_matrixEditor);

    // Size the stack to the visible page only, so short pages do not inherit
    // the height of the matrix editor (QStackedLayout ignores pages with an Ignored policy)
    const auto fitStackToPage = [this](int current) {
        for (int i = 0; i < m_pages->count(); ++i) {
            QWidget *page = m_pages->widget(i);
            page->setSizePolicy(page->sizePolicy().horizontalPolicy(),
                                i == current ? QSizePolicy::Preferred : QSizePolicy::Ignored);
        }
    };
    connect(m_pages, &QStackedWidget::currentChanged, this, fitStackToPage);
    fitStackToPage(m_pages->currentIndex());

    connect(m_operationCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_pages->setCurrentIndex(m_operationCombo->itemData(index, kPageRole).toInt());
    });

    m_applyButton = new QPushButton(tr("Apply"), this);
    connect(m_applyButton, &QPushButton::clicked, this, &ProcessingPanel::applyCurrent);
    connect(&m_viewModel, &ProcessingViewModel::canProcessChanged, this, &ProcessingPanel::updateEnabled);

    auto *form = new QFormLayout;
    form->addRow(tr("Operation:"), m_operationCombo);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(m_pages);
    layout->addWidget(m_applyButton);
    layout->addStretch(1);

    updateEnabled();
}

void ProcessingPanel::applyCurrent()
{
    const int index = m_operationCombo->currentIndex();

    switch (m_operationCombo->itemData(index, kPageRole).toInt()) {
    case MorphologyPage:
        m_viewModel.applyMorphology(
            static_cast<MorphologyOp>(m_operationCombo->itemData(index, kMorphologyOpRole).toInt()));
        break;
    case SharpenPage:
        m_viewModel.applySharpen(m_sharpenAmount->value());
        break;
    case MotionBlurPage:
        m_viewModel.applyMotionBlur(m_blurLength->value(), m_blurAngle->value());
        break;
    case EmbossPage:
        m_viewModel.applyEmboss(m_embossAngle->value(), m_embossStrength->value());
        break;
    case MedianPage:
        m_viewModel.applyMedian(m_medianSize->value());
        break;
    case CustomFilterPage:
        m_viewModel.applyCustomFilter(m_matrixEditor->matrixWidth(), m_matrixEditor->matrixHeight(),
                                      m_matrixEditor->weights(), m_matrixEditor->divisor(),
                                      m_matrixEditor->offset());
        break;
    }
}

void ProcessingPanel::updateEnabled()
{
    m_applyButton->setEnabled(m_viewModel.canProcess());
}
