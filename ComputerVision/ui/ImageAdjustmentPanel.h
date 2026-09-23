#pragma once

#include <QWidget>
#include <QFutureWatcher>
#include "../image/ImageProcessor.h"

class ImageModel;
class QSlider;
class QSpinBox;
class QPushButton;
class QTimer;
class QVBoxLayout;

struct SliderRow
{
    QSlider* slider;
    QSpinBox* spin;
};

class ImageAdjustmentPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ImageAdjustmentPanel(ImageModel* model, ImageProcessor* processor, QWidget* parent = nullptr);

private slots:
    void scheduleUpdate();   // every slider/spinbox tick lands here - just (re)starts the debounce timer
    void applyAdjustments(); // kicks off processing on a worker thread
    void onProcessingFinished();
    void onReset();

private:
    SliderRow addSliderRow(QVBoxLayout* layout, const QString& label, int min, int max, int initial, const QString& suffix);

    ImageModel* m_model;
    ImageProcessor* m_processor;
    QTimer* m_debounceTimer;
    QFutureWatcher<QImage>* m_watcher;
    bool m_pendingReprocess = false;

    QSlider* m_brightnessSlider;
    QSpinBox* m_brightnessSpin;
    QSlider* m_contrastSlider;
    QSpinBox* m_contrastSpin;
    QSlider* m_saturationSlider;
    QSpinBox* m_saturationSpin;
	QSlider* m_gammaSlider;
	QSpinBox* m_gammaSpin;
    QPushButton* m_grayscaleButton;
    QPushButton* m_invertButton;
    QPushButton* m_autoContrastButton;
    QPushButton* m_resetButton;
};