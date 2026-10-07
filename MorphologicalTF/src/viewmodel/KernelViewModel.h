#pragma once

#include "model/Kernel.h"
#include "model/KernelPresets.h"

#include <QObject>

#include <functional>
#include <optional>

// Owns the current structuring element and the "which preset is it" label.
// Transformations will later read the kernel through kernel().
class KernelViewModel : public QObject
{
    Q_OBJECT

public:
    explicit KernelViewModel(QObject *parent = nullptr);

    const Kernel &kernel() const { return m_kernel; }

    // The preset the matrix still matches; std::nullopt means "Custom"
    std::optional<KernelPreset> preset() const { return m_preset; }

public slots:
    // Regenerates the shape at the current size (the anchor returns to the center)
    void applyPreset(KernelPreset preset);
    void setCustom();

    // A preset is regenerated at the new size; a custom matrix is resized in place
    void setSize(int width, int height);

    // Any change of the cells switches the label to "Custom"
    void setCell(int x, int y, bool active);
    void clear();
    void fill();
    void invert();
    void flipHorizontal();
    void flipVertical();
	void move(int dx, int dy);
    void moveRight() { return move(1, 0); };
	void moveLeft() { return move(-1, 0); };
	void moveDown() { return move(0, 1); };
	void moveUp() { return move(0, -1); };

    // Moving the anchor does not change the label
    void setAnchor(int x, int y);

signals:
    void kernelChanged();
    void presetChanged();

private:
    // Applies the change to a copy; does nothing if the kernel stays identical
    void edit(const std::function<void(Kernel &)> &change);

    Kernel m_kernel;
    std::optional<KernelPreset> m_preset;
};
