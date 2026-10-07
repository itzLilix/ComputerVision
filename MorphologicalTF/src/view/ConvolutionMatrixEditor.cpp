#include "view/ConvolutionMatrixEditor.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace {

constexpr int kMaxSize = 15;
constexpr int kCellHeight = 26;
constexpr int kMaxVisibleRows = 9;

// Cell editor: a spin box with enough decimals for weights such as 0.0625
class WeightDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override
    {
        auto *spin = new QDoubleSpinBox(parent);
        spin->setRange(-1e6, 1e6);
        spin->setDecimals(4);
        spin->setFrame(false);
        return spin;
    }
};

struct MatrixPreset
{
    const char *name;
    int size; // square matrix
    QVector<double> weights;
    double offset;
};

const QList<MatrixPreset> &presets()
{
    static const QList<MatrixPreset> list = {
        {"Identity",      3, {0, 0, 0, 0, 1, 0, 0, 0, 0}, 0},
        {"Box Blur",      3, {1, 1, 1, 1, 1, 1, 1, 1, 1}, 0},
        {"Gaussian Blur", 3, {1, 2, 1, 2, 4, 2, 1, 2, 1}, 0},
        {"Sharpen",       3, {0, -1, 0, -1, 5, -1, 0, -1, 0}, 0},
        {"Edge Detect",   3, {-1, -1, -1, -1, 8, -1, -1, -1, -1}, 0},
        {"Emboss",        3, {-1, 0, 0, 0, 0, 0, 0, 0, 1}, 128},
    };
    return list;
}

} // namespace

ConvolutionMatrixEditor::ConvolutionMatrixEditor(QWidget *parent)
    : QWidget(parent)
{
    m_presetCombo = new QComboBox(this);
    m_presetCombo->addItem(tr("Custom"));
    for (const MatrixPreset &preset : presets())
        m_presetCombo->addItem(QString::fromLatin1(preset.name));

    m_widthSpin = new QSpinBox(this);
    m_widthSpin->setRange(1, kMaxSize);
    m_widthSpin->setToolTip(tr("Width"));
    m_heightSpin = new QSpinBox(this);
    m_heightSpin->setRange(1, kMaxSize);
    m_heightSpin->setToolTip(tr("Height"));

    auto *sizeRow = new QHBoxLayout;
    sizeRow->addWidget(m_widthSpin);
    sizeRow->addWidget(new QLabel(QStringLiteral("x"), this));
    sizeRow->addWidget(m_heightSpin);

    m_table = new QTableWidget(this);
    m_table->setItemDelegate(new WeightDelegate(m_table));
    m_table->horizontalHeader()->hide();
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->horizontalHeader()->setMinimumSectionSize(34);
    m_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_table->verticalHeader()->setDefaultSectionSize(kCellHeight);

    m_autoDivisor = new QCheckBox(tr("Auto"), this);
    m_autoDivisor->setToolTip(tr("Use the sum of the weights (1 if the sum is 0)"));
    m_divisorSpin = new QDoubleSpinBox(this);
    m_divisorSpin->setRange(-1e6, 1e6);
    m_divisorSpin->setDecimals(4);
    m_divisorSpin->setValue(1.0);

    auto *divisorRow = new QHBoxLayout;
    divisorRow->addWidget(m_divisorSpin, 1);
    divisorRow->addWidget(m_autoDivisor);

    m_offsetSpin = new QDoubleSpinBox(this);
    m_offsetSpin->setRange(-255.0, 255.0);
    m_offsetSpin->setDecimals(1);

    auto *form = new QFormLayout;
    form->setContentsMargins(0, 0, 0, 0);
    form->addRow(tr("Preset:"), m_presetCombo);
    form->addRow(tr("Size:"), sizeRow);

    auto *bottomForm = new QFormLayout;
    bottomForm->setContentsMargins(0, 0, 0, 0);
    bottomForm->addRow(tr("Divisor:"), divisorRow);
    bottomForm->addRow(tr("Offset:"), m_offsetSpin);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(form);
    layout->addWidget(m_table);
    layout->addLayout(bottomForm);

    // Index 0 of the combo is "Custom" and does nothing when picked
    connect(m_presetCombo, &QComboBox::activated, this, [this](int index) {
        if (index > 0)
            applyPreset(index - 1);
    });

    const auto onSizeChanged = [this] {
        setMatrixSize(m_widthSpin->value(), m_heightSpin->value());
        markCustom();
        updateDivisor();
    };
    connect(m_widthSpin, &QSpinBox::valueChanged, this, onSizeChanged);
    connect(m_heightSpin, &QSpinBox::valueChanged, this, onSizeChanged);

    // Programmatic changes block these signals, so they only fire for user edits
    connect(m_table, &QTableWidget::itemChanged, this, [this] {
        markCustom();
        updateDivisor();
    });
    connect(m_autoDivisor, &QCheckBox::toggled, this, [this] { updateDivisor(); });
    connect(m_divisorSpin, &QDoubleSpinBox::valueChanged, this, [this] { markCustom(); });
    connect(m_offsetSpin, &QDoubleSpinBox::valueChanged, this, [this] { markCustom(); });

    applyPreset(0); // Identity: a neutral starting point
}

int ConvolutionMatrixEditor::matrixWidth() const
{
    return m_table->columnCount();
}

int ConvolutionMatrixEditor::matrixHeight() const
{
    return m_table->rowCount();
}

QVector<double> ConvolutionMatrixEditor::weights() const
{
    QVector<double> result;
    result.reserve(matrixWidth() * matrixHeight());
    for (int row = 0; row < matrixHeight(); ++row) {
        for (int column = 0; column < matrixWidth(); ++column) {
            const QTableWidgetItem *item = m_table->item(row, column);
            result.append(item ? item->data(Qt::EditRole).toDouble() : 0.0);
        }
    }
    return result;
}

double ConvolutionMatrixEditor::divisor() const
{
    const double value = m_divisorSpin->value();
    return value == 0.0 ? 1.0 : value;
}

double ConvolutionMatrixEditor::offset() const
{
    return m_offsetSpin->value();
}

void ConvolutionMatrixEditor::applyPreset(int presetIndex)
{
    const MatrixPreset &preset = presets().at(presetIndex);

    {
        const QSignalBlocker blockWidth(m_widthSpin);
        const QSignalBlocker blockHeight(m_heightSpin);
        m_widthSpin->setValue(preset.size);
        m_heightSpin->setValue(preset.size);
    }
    setMatrixSize(preset.size, preset.size);

    {
        const QSignalBlocker blockTable(m_table);
        for (int row = 0; row < preset.size; ++row) {
            for (int column = 0; column < preset.size; ++column)
                m_table->item(row, column)->setData(Qt::EditRole, preset.weights.at(row * preset.size + column));
        }
    }
    {
        const QSignalBlocker blockOffset(m_offsetSpin);
        m_offsetSpin->setValue(preset.offset);
    }

    m_autoDivisor->setChecked(true);
    updateDivisor();
    m_presetCombo->setCurrentIndex(presetIndex + 1); // does not emit activated()
}

void ConvolutionMatrixEditor::setMatrixSize(int width, int height)
{
    const QSignalBlocker blockTable(m_table);
    m_table->setRowCount(height);
    m_table->setColumnCount(width);

    // Existing cells keep their values; only the new ones are created
    for (int row = 0; row < height; ++row) {
        for (int column = 0; column < width; ++column) {
            if (m_table->item(row, column))
                continue;
            auto *item = new QTableWidgetItem;
            item->setData(Qt::EditRole, 0.0);
            item->setTextAlignment(Qt::AlignCenter);
            m_table->setItem(row, column, item);
        }
    }
    updateTableHeight();
}

void ConvolutionMatrixEditor::markCustom()
{
    m_presetCombo->setCurrentIndex(0);
}

void ConvolutionMatrixEditor::updateDivisor()
{
    const bool automatic = m_autoDivisor->isChecked();
    m_divisorSpin->setEnabled(!automatic);
    if (!automatic)
        return;

    double sum = 0.0;
    for (double weight : weights())
        sum += weight;

    const QSignalBlocker blockDivisor(m_divisorSpin);
    m_divisorSpin->setValue(std::abs(sum) > 1e-9 ? sum : 1.0);
}

void ConvolutionMatrixEditor::updateTableHeight()
{
    // Room for the rows plus a horizontal scroll bar, so the layout does not jump
    const int rows = std::min(m_table->rowCount(), kMaxVisibleRows);
    const int scrollBar = style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    m_table->setFixedHeight(rows * kCellHeight + 2 * m_table->frameWidth() + scrollBar);
}
