#include "ImageAdjustmentPanel.h"
#include "../image/ImageModel.h"

#include <QSlider>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QtConcurrent>

ImageAdjustmentPanel::ImageAdjustmentPanel(ImageModel* model, ImageProcessor* processor, QWidget* parent)
    : QWidget(parent)
	, m_model(model), m_processor(processor)
{
    auto* layout = new QVBoxLayout(this);

    auto brightnessRow = addSliderRow(layout, "Brightness", -100, 100, 0, "");
    m_brightnessSlider = brightnessRow.slider;
    m_brightnessSpin = brightnessRow.spin;

    auto contrastRow = addSliderRow(layout, "Contrast", 0, 200, 100, "%");
    m_contrastSlider = contrastRow.slider;
    m_contrastSpin = contrastRow.spin;

    auto saturationRow = addSliderRow(layout, "Saturation", 0, 200, 100, "%");
    m_saturationSlider = saturationRow.slider;
    m_saturationSpin = saturationRow.spin;

	auto gammaRow = addSliderRow(layout, "Gamma", 1, 400, 100, "%");
	m_gammaSlider = gammaRow.slider;
    m_gammaSpin = gammaRow.spin;

    m_grayscaleButton = new QPushButton("Grayscale");
    m_grayscaleButton->setCheckable(true);
    m_invertButton = new QPushButton("Invert");
    m_invertButton->setCheckable(true);
    m_autoContrastButton = new QPushButton("Auto Contrast");
    m_autoContrastButton->setCheckable(true);
    m_resetButton = new QPushButton("Reset");

    layout->addWidget(m_grayscaleButton);
    layout->addWidget(m_invertButton);
    layout->addWidget(m_autoContrastButton);
    layout->addWidget(m_resetButton);

    connect(m_grayscaleButton, &QPushButton::toggled, this, &ImageAdjustmentPanel::scheduleUpdate);
    connect(m_invertButton, &QPushButton::toggled, this, &ImageAdjustmentPanel::scheduleUpdate);
    connect(m_autoContrastButton, &QPushButton::toggled, this, &ImageAdjustmentPanel::scheduleUpdate);
    connect(m_resetButton, &QPushButton::clicked, this, &ImageAdjustmentPanel::onReset);

    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(120);
    connect(m_debounceTimer, &QTimer::timeout, this, &ImageAdjustmentPanel::applyAdjustments);

    m_watcher = new QFutureWatcher<QImage>(this);
    connect(m_watcher, &QFutureWatcher<QImage>::finished, this, &ImageAdjustmentPanel::onProcessingFinished);

    connect(m_model, &ImageModel::newImageLoaded, this, &ImageAdjustmentPanel::onReset);
	connect(m_model, &ImageModel::imageClosed, this, &ImageAdjustmentPanel::onReset);
}

SliderRow ImageAdjustmentPanel::addSliderRow(QVBoxLayout* layout, const QString& label, int min, int max, int initial, const QString& suffix)
{
    auto* row = new QHBoxLayout;

    auto* slider = new QSlider(Qt::Horizontal);
    slider->setRange(min, max);
    slider->setValue(initial);

    auto* spin = new QSpinBox;
    spin->setRange(min, max);
    spin->setValue(initial);
    spin->setSuffix(suffix);

    row->addWidget(new QLabel(label));
    row->addWidget(slider, 1);
    row->addWidget(spin);
    layout->addLayout(row);

    connect(slider, &QSlider::valueChanged, spin, &QSpinBox::setValue);
    connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), slider, &QSlider::setValue);
    connect(slider, &QSlider::valueChanged, this, &ImageAdjustmentPanel::scheduleUpdate);

    return { slider, spin };
}

void ImageAdjustmentPanel::scheduleUpdate()
{
    m_debounceTimer->start();
}

void ImageAdjustmentPanel::applyAdjustments()
{
    if (m_watcher->isRunning()) {
        m_pendingReprocess = true;
        return;
    }

    QImage source = m_model->originalImage();
    int brightness = m_brightnessSlider->value();
    double contrast = m_contrastSlider->value() / 100.0;
    double saturation = m_saturationSlider->value() / 100.0;
	double gamma = m_gammaSlider->value() / 100.0;
    bool grayscale = m_grayscaleButton->isChecked();
    bool invert = m_invertButton->isChecked();
	bool autoContrast = m_autoContrastButton->isChecked();

    QFuture<QImage> future = QtConcurrent::run([this, source, brightness, contrast, saturation, gamma, grayscale, invert, autoContrast]() {
        QImage result = source;
        if (brightness != 0)
            result = m_processor->adjustBrightness(result, brightness);
        if (!qFuzzyCompare(contrast, 1.0))
            result = m_processor->adjustContrast(result, contrast);
        if (!qFuzzyCompare(saturation, 1.0))
            result = m_processor->adjustSaturation(result, saturation);
        if (!qFuzzyCompare(gamma, 1.0))
            result = m_processor->gammaCorrection(result, gamma);
        if (grayscale)
            result = m_processor->convertToGrayscale(result);
        if (invert)
            result = m_processor->invertColors(result);
		if (autoContrast)
			result = m_processor->linearCorrection(result);
        return result;
        });

    m_watcher->setFuture(future);
}

void ImageAdjustmentPanel::onProcessingFinished()
{
    m_model->setImage(m_watcher->result());

    if (m_pendingReprocess) {
        m_pendingReprocess = false;
        applyAdjustments();
    }
}

void ImageAdjustmentPanel::onReset()
{
    m_brightnessSlider->setValue(0);
    m_contrastSlider->setValue(100);
    m_saturationSlider->setValue(100);
	m_gammaSlider->setValue(100);
    m_grayscaleButton->setChecked(false);
    m_invertButton->setChecked(false);
	m_autoContrastButton->setChecked(false);
    applyAdjustments();
}