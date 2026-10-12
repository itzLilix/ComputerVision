#include "viewmodel/ConvolutionViewModel.h"

#include <cmath>

ConvolutionViewModel::ConvolutionViewModel(QObject *parent)
    : QObject(parent)
{
    // Starts as a 3x3 identity matrix (Custom): a neutral starting point
}

void ConvolutionViewModel::applyPreset(ConvolutionPreset preset)
{
    if (ConvolutionPresets::usesAngle(preset))
        m_angle = ConvolutionPresets::defaultAngle(preset);

    m_preset = preset;
    m_matrix = ConvolutionPresets::create(preset, m_matrix.width(), m_matrix.height(), m_angle);
    applyAutoDivisor();

    emit matrixChanged();
    emit presetChanged();
}

void ConvolutionViewModel::setCustom()
{
    if (!m_preset)
        return;

    m_preset.reset();
    emit presetChanged();
}

void ConvolutionViewModel::setAngle(double degrees)
{
    if (!m_preset || !ConvolutionPresets::usesAngle(*m_preset))
        return;

    degrees = std::fmod(degrees, 360.0);
    if (degrees < 0.0)
        degrees += 360.0;
    if (degrees == m_angle)
        return;

    m_angle = degrees;
    m_matrix = ConvolutionPresets::create(*m_preset, m_matrix.width(), m_matrix.height(), m_angle);
    applyAutoDivisor();

    emit matrixChanged();
    emit presetChanged();
}

void ConvolutionViewModel::setSize(int width, int height)
{
    width = ConvolutionMatrix::clampSize(width);
    height = ConvolutionMatrix::clampSize(height);
    if (width == m_matrix.width() && height == m_matrix.height())
        return;

    if (m_preset)
        m_matrix = ConvolutionPresets::create(*m_preset, width, height, m_angle); // preset works for any size
    else
        m_matrix.resize(width, height);

    applyAutoDivisor();
    emit matrixChanged();
}

void ConvolutionViewModel::setCell(int x, int y, double weight)
{
    if (!m_matrix.contains(x, y) || m_matrix.at(x, y) == weight)
        return;

    m_matrix.set(x, y, weight);
    commitEdit();
}

void ConvolutionViewModel::setAnchor(int x, int y)
{
    if (!m_matrix.contains(x, y) || m_matrix.anchor() == QPoint(x, y))
        return;

    m_matrix.setAnchor(x, y);
    commitEdit();
}

void ConvolutionViewModel::setAutoDivisor(bool enabled)
{
    if (enabled == m_autoDivisor)
        return;

    m_autoDivisor = enabled;
    emit autoDivisorChanged();

    if (enabled) {
        applyAutoDivisor();
        emit matrixChanged();
    }
}

void ConvolutionViewModel::setDivisor(double divisor)
{
    if (m_autoDivisor)
        return;

    if (divisor == 0.0) {
        emit matrixChanged(); // rejected: the view goes back to the real value
        return;
    }
    if (divisor == m_matrix.divisor())
        return;

    m_matrix.setDivisor(divisor);
    commitEdit();
}

void ConvolutionViewModel::setBias(double bias)
{
    if (bias == m_matrix.bias())
        return;

    m_matrix.setBias(bias);
    commitEdit();
}

void ConvolutionViewModel::clear()
{
    m_matrix.clear();
    commitEdit();
}

void ConvolutionViewModel::fill()
{
    m_matrix.fill(1.0);
    commitEdit();
}

void ConvolutionViewModel::negate()
{
    m_matrix.negate();
    commitEdit();
}

void ConvolutionViewModel::flipHorizontal()
{
    m_matrix.flipHorizontal();
    commitEdit();
}

void ConvolutionViewModel::flipVertical()
{
    m_matrix.flipVertical();
    commitEdit();
}

void ConvolutionViewModel::move(int dx, int dy)
{
    m_matrix.move(dx, dy);
    commitEdit();
}

void ConvolutionViewModel::applyAutoDivisor()
{
    if (m_autoDivisor)
        m_matrix.normalize();
}

void ConvolutionViewModel::commitEdit()
{
    const bool wasPreset = m_preset.has_value();
    m_preset.reset();
    applyAutoDivisor();

    emit matrixChanged();
    if (wasPreset)
        emit presetChanged();
}
