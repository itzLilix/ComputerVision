#include "viewmodel/KernelViewModel.h"

KernelViewModel::KernelViewModel(QObject *parent)
    : QObject(parent)
    , m_kernel(KernelPresets::create(KernelPreset::Rectangle, 3, 3))
    , m_preset(KernelPreset::Rectangle)
{
}

void KernelViewModel::applyPreset(KernelPreset preset)
{
    m_kernel = KernelPresets::create(preset, m_kernel.width(), m_kernel.height());
    m_preset = preset;
    emit kernelChanged();
    emit presetChanged();
}

void KernelViewModel::setCustom()
{
    if (!m_preset)
        return;
    m_preset.reset();
    emit presetChanged();
}

void KernelViewModel::setSize(int width, int height)
{
    const int newWidth = Kernel::clampSize(width);
    const int newHeight = Kernel::clampSize(height);
    if (newWidth == m_kernel.width() && newHeight == m_kernel.height())
        return;

    if (m_preset)
        m_kernel = KernelPresets::create(*m_preset, newWidth, newHeight);
    else
        m_kernel.resize(newWidth, newHeight);
    emit kernelChanged();
}

void KernelViewModel::setCell(int x, int y, bool active)
{
    edit([=](Kernel &kernel) { kernel.set(x, y, active); });
}

void KernelViewModel::clear()
{
    edit([](Kernel &kernel) { kernel.clear(); });
}

void KernelViewModel::fill()
{
    edit([](Kernel &kernel) { kernel.fill(); });
}

void KernelViewModel::invert()
{
    edit([](Kernel &kernel) { kernel.invert(); });
}

void KernelViewModel::flipHorizontal()
{
    edit([](Kernel &kernel) { kernel.flipHorizontal(); });
}

void KernelViewModel::flipVertical()
{
    edit([](Kernel &kernel) { kernel.flipVertical(); });
}

void KernelViewModel::move(int dx, int dy)
{
	edit([=](Kernel& kernel) { kernel.move(dx, dy); });
}

void KernelViewModel::setAnchor(int x, int y)
{
    if (!m_kernel.contains(x, y) || m_kernel.anchor() == QPoint(x, y))
        return;
    m_kernel.setAnchor(x, y);
    emit kernelChanged();
}

void KernelViewModel::edit(const std::function<void(Kernel &)> &change)
{
    Kernel edited = m_kernel;
    change(edited);
    if (edited == m_kernel)
        return; // e.g. flipping a symmetric preset keeps its label

    m_kernel = std::move(edited);
    emit kernelChanged();

    if (m_preset) {
        m_preset.reset();
        emit presetChanged();
    }
}
