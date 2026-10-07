#pragma once

#include <QVector>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class QTableWidget;

// Editor for the user-defined convolution filter: a matrix of weights, a divisor,
// an offset and a few ready-made presets. The anchor is the cell (width / 2, height / 2).
class ConvolutionMatrixEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ConvolutionMatrixEditor(QWidget *parent = nullptr);

    int matrixWidth() const;
    int matrixHeight() const;
    QVector<double> weights() const; // row-major, matrixWidth() * matrixHeight() values
    double divisor() const;          // the effective value (resolved from "Auto"), never zero
    double offset() const;

private:
    void applyPreset(int presetIndex);
    void setMatrixSize(int width, int height); // keeps the overlapping values, new cells are 0
    void markCustom();
    void updateDivisor();
    void updateTableHeight();

    QComboBox *m_presetCombo = nullptr;
    QSpinBox *m_widthSpin = nullptr;
    QSpinBox *m_heightSpin = nullptr;
    QTableWidget *m_table = nullptr;
    QCheckBox *m_autoDivisor = nullptr;
    QDoubleSpinBox *m_divisorSpin = nullptr;
    QDoubleSpinBox *m_offsetSpin = nullptr;
};
