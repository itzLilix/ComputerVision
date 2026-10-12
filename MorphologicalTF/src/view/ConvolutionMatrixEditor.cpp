#include "view/ConvolutionMatrixEditor.h"

#include "viewmodel/ConvolutionViewModel.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <optional>

namespace {
constexpr int kCellHeight = 26;
constexpr int kMaxVisibleRows = 9;
} // namespace

// Cell delegate: a spin box editor with enough decimals for weights such as 0.0625,
// a compact display text, and the anchor drawn as a thin red circle (as in KernelGridView).
class MatrixCellDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void setAnchor(const QPoint &anchor) { m_anchor = anchor; }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override
    {
        auto *spin = new QDoubleSpinBox(parent);
        spin->setRange(-1e6, 1e6);
        spin->setDecimals(4);
        spin->setFrame(false);
        return spin;
    }

    QString displayText(const QVariant &value, const QLocale &locale) const override
    {
        return locale.toString(value.toDouble(), 'g', 4);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::paint(painter, option, index);

        if (index.column() != m_anchor.x() || index.row() != m_anchor.y())
            return;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(QPen(Qt::red, 1.5));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QRectF(option.rect).adjusted(2, 2, -2, -2));
        painter->restore();
    }

private:
    QPoint m_anchor{-1, -1};
};

ConvolutionMatrixEditor::ConvolutionMatrixEditor(ConvolutionViewModel &viewModel, QWidget *parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    // Preset selector: "Custom" has item data -1, real presets store their enum value
    m_presetCombo = new QComboBox(this);
    m_presetCombo->addItem(tr("Custom"), -1);
    for (ConvolutionPreset preset : ConvolutionPresets::all())
        m_presetCombo->addItem(ConvolutionPresets::displayName(preset), static_cast<int>(preset));

    m_widthSpin = new QSpinBox(this);
    m_widthSpin->setRange(ConvolutionMatrix::kMinSize, ConvolutionMatrix::kMaxSize);
    m_widthSpin->setToolTip(tr("Width"));
    m_heightSpin = new QSpinBox(this);
    m_heightSpin->setRange(ConvolutionMatrix::kMinSize, ConvolutionMatrix::kMaxSize);
    m_heightSpin->setToolTip(tr("Height"));

    auto *sizeRow = new QHBoxLayout;
    sizeRow->addWidget(m_widthSpin);
    sizeRow->addWidget(new QLabel(QStringLiteral("x"), this));
    sizeRow->addWidget(m_heightSpin);

    // Only used by the presets that depend on a direction (Motion Blur, Emboss)
    m_angleSpin = new QDoubleSpinBox(this);
    m_angleSpin->setRange(0.0, 359.0);
    m_angleSpin->setDecimals(0);
    m_angleSpin->setSingleStep(15.0);
    m_angleSpin->setWrapping(true);
    m_angleSpin->setSuffix(QStringLiteral(" \u00b0"));
    m_angleSpin->setKeyboardTracking(false);
    m_angleSpin->setToolTip(tr("Direction of the blur or of the relief, counterclockwise from the right"));

    auto *form = new QFormLayout;
    form->addRow(tr("Preset:"), m_presetCombo);
    form->addRow(tr("Size:"), sizeRow);
    form->addRow(tr("Angle:"), m_angleSpin);

    // Weight table: the anchor cell is marked by the delegate
    m_table = new QTableWidget(this);
    m_delegate = new MatrixCellDelegate(m_table);
    m_table->setItemDelegate(m_delegate);
    m_table->horizontalHeader()->hide();
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->horizontalHeader()->setMinimumSectionSize(48);
    m_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_table->verticalHeader()->setDefaultSectionSize(kCellHeight);

    auto *clearButton = new QPushButton(tr("Clear"), this);
    auto *fillButton = new QPushButton(tr("Fill"), this);
    fillButton->setToolTip(tr("Set all weights to 1"));
    auto *negateButton = new QPushButton(tr("Negate"), this);
    negateButton->setToolTip(tr("Change the sign of all weights"));
    auto *flipHButton = new QPushButton(tr("Flip H"), this);
    auto *flipVButton = new QPushButton(tr("Flip V"), this);
    m_anchorButton = new QPushButton(tr("Set Anchor"), this);
    m_anchorButton->setCheckable(true);
    m_anchorButton->setToolTip(tr("Then click a cell to move the anchor there"));

    auto *buttons = new QGridLayout;
    buttons->addWidget(clearButton, 0, 0);
    buttons->addWidget(fillButton, 0, 1);
    buttons->addWidget(negateButton, 0, 2);
    buttons->addWidget(flipHButton, 1, 0);
    buttons->addWidget(flipVButton, 1, 1);
    buttons->addWidget(m_anchorButton, 1, 2);

    // Shift row: four arrow buttons, holding one down repeats the move
    const auto makeArrowButton = [this, clearButton](Qt::ArrowType arrow, const QString &toolTip) {
        auto *button = new QToolButton(this);
        button->setArrowType(arrow);
        button->setToolTip(toolTip);
        button->setAutoRepeat(true);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        button->setFixedHeight(clearButton->sizeHint().height());
        return button;
    };
    auto *moveLeftButton = makeArrowButton(Qt::LeftArrow, tr("Shift left"));
    auto *moveUpButton = makeArrowButton(Qt::UpArrow, tr("Shift up"));
    auto *moveDownButton = makeArrowButton(Qt::DownArrow, tr("Shift down"));
    auto *moveRightButton = makeArrowButton(Qt::RightArrow, tr("Shift right"));

    auto *shiftRow = new QHBoxLayout;
    shiftRow->addWidget(new QLabel(tr("Shift:"), this));
    shiftRow->addWidget(moveLeftButton);
    shiftRow->addWidget(moveUpButton);
    shiftRow->addWidget(moveDownButton);
    shiftRow->addWidget(moveRightButton);

    // Keyboard tracking is off, so a value is applied on Enter / focus loss, not on every key
    // (typing "0.5" must not pass through an invalid divisor of 0)
    m_autoDivisor = new QCheckBox(tr("Auto"), this);
    m_autoDivisor->setToolTip(tr("Use the sum of the weights (1 if the sum is 0)"));
    m_divisorSpin = new QDoubleSpinBox(this);
    m_divisorSpin->setRange(-1e6, 1e6);
    m_divisorSpin->setDecimals(4);
    m_divisorSpin->setKeyboardTracking(false);

    auto *divisorRow = new QHBoxLayout;
    divisorRow->addWidget(m_divisorSpin, 1);
    divisorRow->addWidget(m_autoDivisor);

    m_offsetSpin = new QDoubleSpinBox(this);
    m_offsetSpin->setRange(-255.0, 255.0);
    m_offsetSpin->setDecimals(1);
    m_offsetSpin->setKeyboardTracking(false);

    auto *bottomForm = new QFormLayout;
    bottomForm->addRow(tr("Divisor:"), divisorRow);
    bottomForm->addRow(tr("Offset:"), m_offsetSpin);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(m_table);
    layout->addLayout(buttons);
    layout->addLayout(shiftRow);
    layout->addLayout(bottomForm);

    // View -> view model (commands)
    connect(m_presetCombo, &QComboBox::activated, this, [this](int index) {
        const int data = m_presetCombo->itemData(index).toInt();
        if (data < 0)
            m_viewModel.setCustom();
        else
            m_viewModel.applyPreset(static_cast<ConvolutionPreset>(data));
    });

    const auto applySize = [this] { m_viewModel.setSize(m_widthSpin->value(), m_heightSpin->value()); };
    connect(m_widthSpin, &QSpinBox::valueChanged, this, applySize);
    connect(m_heightSpin, &QSpinBox::valueChanged, this, applySize);
    connect(m_angleSpin, &QDoubleSpinBox::valueChanged, &m_viewModel, &ConvolutionViewModel::setAngle);

    // Table signals are blocked while the view syncs itself, so this only fires for user edits
    connect(m_table, &QTableWidget::itemChanged, this, [this](QTableWidgetItem *item) {
        m_viewModel.setCell(item->column(), item->row(), item->data(Qt::EditRole).toDouble());
    });

    connect(clearButton, &QPushButton::clicked, &m_viewModel, &ConvolutionViewModel::clear);
    connect(fillButton, &QPushButton::clicked, &m_viewModel, &ConvolutionViewModel::fill);
    connect(negateButton, &QPushButton::clicked, &m_viewModel, &ConvolutionViewModel::negate);
    connect(flipHButton, &QPushButton::clicked, &m_viewModel, &ConvolutionViewModel::flipHorizontal);
    connect(flipVButton, &QPushButton::clicked, &m_viewModel, &ConvolutionViewModel::flipVertical);

    connect(moveLeftButton, &QToolButton::clicked, &m_viewModel, &ConvolutionViewModel::moveLeft);
    connect(moveRightButton, &QToolButton::clicked, &m_viewModel, &ConvolutionViewModel::moveRight);
    connect(moveUpButton, &QToolButton::clicked, &m_viewModel, &ConvolutionViewModel::moveUp);
    connect(moveDownButton, &QToolButton::clicked, &m_viewModel, &ConvolutionViewModel::moveDown);

    connect(m_autoDivisor, &QCheckBox::toggled, &m_viewModel, &ConvolutionViewModel::setAutoDivisor);
    connect(m_divisorSpin, &QDoubleSpinBox::valueChanged, &m_viewModel, &ConvolutionViewModel::setDivisor);
    connect(m_offsetSpin, &QDoubleSpinBox::valueChanged, &m_viewModel, &ConvolutionViewModel::setBias);

    // Anchor pick mode: while the button is checked, the next click on a cell picks the anchor
    // instead of editing it (the table cannot edit and paint at once, so editing is switched off)
    const auto defaultEditTriggers = m_table->editTriggers();
    connect(m_anchorButton, &QPushButton::toggled, this, [this, defaultEditTriggers](bool enabled) {
        m_table->setEditTriggers(enabled ? QAbstractItemView::NoEditTriggers : defaultEditTriggers);
        if (enabled)
            m_table->viewport()->setCursor(Qt::CrossCursor);
        else
            m_table->viewport()->unsetCursor();
    });
    connect(m_table, &QTableWidget::cellPressed, this, [this](int row, int column) {
        if (!m_anchorButton->isChecked())
            return;
        m_viewModel.setAnchor(column, row);
        m_anchorButton->setChecked(false); // the mode ends after one click
    });

    // View model -> view (state)
    connect(&m_viewModel, &ConvolutionViewModel::matrixChanged, this, &ConvolutionMatrixEditor::syncMatrix);
    connect(&m_viewModel, &ConvolutionViewModel::presetChanged, this, &ConvolutionMatrixEditor::syncPreset);
    connect(&m_viewModel, &ConvolutionViewModel::autoDivisorChanged, this, &ConvolutionMatrixEditor::syncAutoDivisor);

    syncMatrix();
    syncPreset();
    syncAutoDivisor();
}

void ConvolutionMatrixEditor::syncMatrix()
{
    const ConvolutionMatrix &matrix = m_viewModel.matrix();

    {
        const QSignalBlocker blockWidth(m_widthSpin);
        const QSignalBlocker blockHeight(m_heightSpin);
        const QSignalBlocker blockDivisor(m_divisorSpin);
        const QSignalBlocker blockOffset(m_offsetSpin);
        m_widthSpin->setValue(matrix.width());
        m_heightSpin->setValue(matrix.height());
        m_divisorSpin->setValue(matrix.divisor());
        m_offsetSpin->setValue(matrix.bias());
    }

    const QSignalBlocker blockTable(m_table);
    m_table->setColumnCount(matrix.width());
    m_table->setRowCount(matrix.height());

    // Only the cells that differ are touched, so editing one cell does not disturb the others
    for (int row = 0; row < matrix.height(); ++row) {
        for (int column = 0; column < matrix.width(); ++column) {
            const double weight = matrix.at(column, row);
            QTableWidgetItem *item = m_table->item(row, column);
            if (!item) {
                item = new QTableWidgetItem;
                item->setTextAlignment(Qt::AlignCenter);
                item->setData(Qt::EditRole, weight);
                m_table->setItem(row, column, item);
            } else if (item->data(Qt::EditRole).toDouble() != weight) {
                item->setData(Qt::EditRole, weight);
            }
        }
    }

    m_delegate->setAnchor(matrix.anchor());
    m_table->viewport()->update();
    updateTableHeight();
}

void ConvolutionMatrixEditor::syncPreset()
{
    const std::optional<ConvolutionPreset> preset = m_viewModel.preset();
    const int data = preset ? static_cast<int>(*preset) : -1;

    // setCurrentIndex does not emit activated(), so there is no feedback loop
    m_presetCombo->setCurrentIndex(m_presetCombo->findData(data));

    m_angleSpin->setEnabled(preset && ConvolutionPresets::usesAngle(*preset));
    const QSignalBlocker blockAngle(m_angleSpin);
    m_angleSpin->setValue(m_viewModel.angle());
}

void ConvolutionMatrixEditor::syncAutoDivisor()
{
    const bool automatic = m_viewModel.autoDivisor();

    const QSignalBlocker blockAuto(m_autoDivisor);
    m_autoDivisor->setChecked(automatic);
    m_divisorSpin->setEnabled(!automatic);
}

void ConvolutionMatrixEditor::updateTableHeight()
{
    // Room for the rows plus a horizontal scroll bar, so the layout does not jump
    const int rows = std::min(m_table->rowCount(), kMaxVisibleRows);
    const int scrollBar = style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    m_table->setFixedHeight(rows * kCellHeight + 2 * m_table->frameWidth() + scrollBar);
}
