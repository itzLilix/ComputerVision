#include "view/KernelPanel.h"

#include "view/KernelGridView.h"
#include "viewmodel/KernelViewModel.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>

KernelPanel::KernelPanel(KernelViewModel& viewModel, QWidget* parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    // Preset selector: "Custom" has item data -1, real presets store their enum value
    m_presetCombo = new QComboBox(this);
    m_presetCombo->addItem(tr("Custom"), -1);
    for (KernelPreset preset : KernelPresets::all())
        m_presetCombo->addItem(KernelPresets::displayName(preset), static_cast<int>(preset));

    m_widthSpin = new QSpinBox(this);
    m_widthSpin->setRange(Kernel::kMinSize, Kernel::kMaxSize);
    m_widthSpin->setToolTip(tr("Width"));
    m_heightSpin = new QSpinBox(this);
    m_heightSpin->setRange(Kernel::kMinSize, Kernel::kMaxSize);
    m_heightSpin->setToolTip(tr("Height"));

    auto* sizeRow = new QHBoxLayout;
    sizeRow->addWidget(m_widthSpin);
    sizeRow->addWidget(new QLabel(QStringLiteral("x"), this));
    sizeRow->addWidget(m_heightSpin);

    auto* form = new QFormLayout;
    form->addRow(tr("Preset:"), m_presetCombo);
    form->addRow(tr("Size:"), sizeRow);

    m_grid = new KernelGridView(this);

    auto* clearButton = new QPushButton(tr("Clear"), this);
    auto* fillButton = new QPushButton(tr("Fill"), this);
    auto* invertButton = new QPushButton(tr("Invert"), this);
    auto* flipHButton = new QPushButton(tr("Flip H"), this);
    auto* flipVButton = new QPushButton(tr("Flip V"), this);
    m_anchorButton = new QPushButton(tr("Set Anchor"), this);
    m_anchorButton->setCheckable(true);
    m_anchorButton->setToolTip(tr("Then click a cell to move the anchor there"));

    auto* buttons = new QGridLayout;
    buttons->addWidget(clearButton, 0, 0);
    buttons->addWidget(fillButton, 0, 1);
    buttons->addWidget(invertButton, 0, 2);
    buttons->addWidget(flipHButton, 1, 0);
    buttons->addWidget(flipVButton, 1, 1);
    buttons->addWidget(m_anchorButton, 1, 2);

    // Shift row: four arrow buttons, holding one down repeats the move
    const auto makeArrowButton = [this, clearButton](Qt::ArrowType arrow, const QString& toolTip) {
        auto* button = new QToolButton(this);
        button->setArrowType(arrow);
        button->setToolTip(toolTip);
        button->setAutoRepeat(true);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        button->setFixedHeight(clearButton->sizeHint().height());
        return button;
        };
    auto* moveLeftButton = makeArrowButton(Qt::LeftArrow, tr("Shift left"));
    auto* moveUpButton = makeArrowButton(Qt::UpArrow, tr("Shift up"));
    auto* moveDownButton = makeArrowButton(Qt::DownArrow, tr("Shift down"));
    auto* moveRightButton = makeArrowButton(Qt::RightArrow, tr("Shift right"));

    auto* shiftRow = new QHBoxLayout;
    shiftRow->addWidget(new QLabel(tr("Shift:"), this));
    shiftRow->addWidget(moveLeftButton);
    shiftRow->addWidget(moveUpButton);
    shiftRow->addWidget(moveDownButton);
    shiftRow->addWidget(moveRightButton);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(m_grid, 1);
    layout->addLayout(buttons);
    layout->addLayout(shiftRow);

    // View -> view model (commands)
    connect(m_presetCombo, &QComboBox::activated, this, [this](int index) {
        const int data = m_presetCombo->itemData(index).toInt();
        if (data < 0)
            m_viewModel.setCustom();
        else
            m_viewModel.applyPreset(static_cast<KernelPreset>(data));
        });

    const auto applySize = [this] { m_viewModel.setSize(m_widthSpin->value(), m_heightSpin->value()); };
    connect(m_widthSpin, &QSpinBox::valueChanged, this, applySize);
    connect(m_heightSpin, &QSpinBox::valueChanged, this, applySize);

    connect(clearButton, &QPushButton::clicked, &m_viewModel, &KernelViewModel::clear);
    connect(fillButton, &QPushButton::clicked, &m_viewModel, &KernelViewModel::fill);
    connect(invertButton, &QPushButton::clicked, &m_viewModel, &KernelViewModel::invert);
    connect(flipHButton, &QPushButton::clicked, &m_viewModel, &KernelViewModel::flipHorizontal);
    connect(flipVButton, &QPushButton::clicked, &m_viewModel, &KernelViewModel::flipVertical);

    connect(moveLeftButton, &QToolButton::clicked, &m_viewModel, &KernelViewModel::moveLeft);
    connect(moveRightButton, &QToolButton::clicked, &m_viewModel, &KernelViewModel::moveRight);
    connect(moveUpButton, &QToolButton::clicked, &m_viewModel, &KernelViewModel::moveUp);
    connect(moveDownButton, &QToolButton::clicked, &m_viewModel, &KernelViewModel::moveDown);

    connect(m_grid, &KernelGridView::cellPainted, &m_viewModel, &KernelViewModel::setCell);
    connect(m_grid, &KernelGridView::anchorPicked, &m_viewModel, &KernelViewModel::setAnchor);

    // Anchor button <-> grid pick mode (the grid leaves the mode after one click)
    connect(m_anchorButton, &QPushButton::toggled, m_grid, &KernelGridView::setAnchorPickMode);
    connect(m_grid, &KernelGridView::anchorPickFinished, m_anchorButton, [this] {
        m_anchorButton->setChecked(false);
        });

    // View model -> view (state)
    connect(&m_viewModel, &KernelViewModel::kernelChanged, this, &KernelPanel::syncKernel);
    connect(&m_viewModel, &KernelViewModel::presetChanged, this, &KernelPanel::syncPreset);

    syncKernel();
    syncPreset();
}

void KernelPanel::syncKernel()
{
    const Kernel& kernel = m_viewModel.kernel();

    const QSignalBlocker blockWidth(m_widthSpin);
    const QSignalBlocker blockHeight(m_heightSpin);
    m_widthSpin->setValue(kernel.width());
    m_heightSpin->setValue(kernel.height());

    m_grid->setKernel(kernel);
}

void KernelPanel::syncPreset()
{
    const std::optional<KernelPreset> preset = m_viewModel.preset();
    const int data = preset ? static_cast<int>(*preset) : -1;

    // setCurrentIndex does not emit activated(), so there is no feedback loop
    m_presetCombo->setCurrentIndex(m_presetCombo->findData(data));
}