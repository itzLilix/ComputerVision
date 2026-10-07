#pragma once

#include "model/MorphologyOp.h"

#include <QObject>
#include <QVector>

class ImageModel;
class KernelViewModel;

// Commands behind the Processing panel.
// Contract for every apply*() slot:
//  - read ImageModel::current(), compute the result, hand it over via ImageModel::setCurrent();
//  - do nothing when there is no image;
//  - on bad input or failure emit errorOccurred() (the main window shows it in a message box).
// Angles are in degrees: 0 = horizontal, 90 = vertical, counter-clockwise as seen on screen.
class ProcessingViewModel : public QObject
{
    Q_OBJECT

public:
    ProcessingViewModel(ImageModel &imageModel, const KernelViewModel &kernelViewModel,
                        QObject *parent = nullptr);

    // True when an image is loaded and no operation is running.
    // Emit canProcessChanged() whenever this value may have changed
    // (image loaded/closed, an asynchronous operation started/finished).
    bool canProcess() const;

public slots:
    // Uses kernelViewModel.kernel(); report an error if the kernel is empty.
    void applyMorphology(MorphologyOp op);

    // amount: 0.1 .. 10, 1.0 = normal strength
    void applySharpen(double amount);

    // length: 1 .. 200 pixels along the direction given by angleDegrees (0 .. 359)
    void applyMotionBlur(int length, double angleDegrees);

    // angleDegrees: direction of the relief (0 .. 359); strength: 0.1 .. 10, 1.0 = normal
    void applyEmboss(double angleDegrees, double strength);

    // windowSize: odd, 3 .. 31 (a windowSize x windowSize square window)
    void applyMedian(int windowSize);

    // weights: row-major, width * height values; the anchor is the cell (width / 2, height / 2).
    // Per channel: result = sum(weight * pixel) / divisor + offset, clamped to 0 .. 255.
    // divisor is never zero; offset is in -255 .. 255.
    void applyCustomFilter(int width, int height, const QVector<double> &weights,
                           double divisor, double offset);

signals:
    void canProcessChanged();
    void errorOccurred(const QString &message);

private:
    ImageModel &m_imageModel;
    const KernelViewModel &m_kernelViewModel;
    bool m_busy = false; // set while an asynchronous operation is running
};
