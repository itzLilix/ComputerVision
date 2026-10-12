#pragma once

#include "model/ConvolutionMatrix.h"
#include "model/ConvolutionPresets.h"

#include <QObject>

#include <optional>

// State and commands of the convolution matrix editor. Mirrors KernelViewModel:
// the view sends commands (slots) and redraws itself on the change signals.
//
// "Custom" means no preset (preset() is empty): any manual edit of the matrix, the anchor,
// the divisor or the offset switches to it. Changing the size or the angle while a preset
// is active rebuilds the preset instead. Pass matrix() to FilterProcessor::convolve().
class ConvolutionViewModel : public QObject
{
    Q_OBJECT

public:
    explicit ConvolutionViewModel(QObject *parent = nullptr);

    const ConvolutionMatrix &matrix() const { return m_matrix; }
    std::optional<ConvolutionPreset> preset() const { return m_preset; }
    double angle() const { return m_angle; }          // degrees, [0, 360)
    bool autoDivisor() const { return m_autoDivisor; } // divisor follows the sum of the weights

public slots:
    void applyPreset(ConvolutionPreset preset);
    void setCustom();
    void setAngle(double degrees); // only has an effect while MotionBlur or Emboss is active

    void setSize(int width, int height);
    void setCell(int x, int y, double weight);
    void setAnchor(int x, int y);

    void setAutoDivisor(bool enabled);
    void setDivisor(double divisor); // ignored while the divisor is automatic
    void setBias(double bias);

    void clear();
    void fill(); // all weights = 1
    void negate();
    void flipHorizontal();
    void flipVertical();
    void moveLeft() { move(-1, 0); }
    void moveRight() { move(1, 0); }
    void moveUp() { move(0, -1); }
    void moveDown() { move(0, 1); }

signals:
    void matrixChanged(); // cells, size, anchor, divisor or offset
    void presetChanged(); // preset or angle
    void autoDivisorChanged();

private:
    void move(int dx, int dy);
    void applyAutoDivisor();
    void commitEdit(); // after a manual change: switch to Custom, keep the divisor up to date, notify

    ConvolutionMatrix m_matrix;
    std::optional<ConvolutionPreset> m_preset; // empty = Custom
    double m_angle = 0.0;
    bool m_autoDivisor = true;
};
