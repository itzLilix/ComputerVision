#include "HistogramWidget.h"

#include <QVBoxLayout>
#include <QPen>
#include <QLegend>
#include <QLegendMarker>
#include <algorithm>

namespace
{
    constexpr qreal kBeforeOpacity = 0.2;
    constexpr qreal kAfterOpacity = 1.0;
}

HistogramWidget::HistogramWidget(QWidget* parent)
    : QWidget(parent)
{
    m_chart = new QChart();
    m_chart->setTitle("Channel Histogram");
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);

    auto makeSeries = [this](const QString& name, const QColor& color, qreal opacity, bool inLegend)
        {
            auto* series = new QLineSeries();
            series->setName(name);

            QPen pen(color);
            pen.setWidth(2);
            series->setPen(pen);
            series->setOpacity(opacity);
            series->setVisible(false);

            m_chart->addSeries(series);
            //const auto markers = m_chart->legend()->markers(series);
            //if (!markers.isEmpty())
            //    markers.first()->setVisible(inLegend);

            return series;
        };

    // current
    m_afterRed = makeSeries("R", QColor(220, 40, 40), kAfterOpacity, true);
    m_afterGreen = makeSeries("G", QColor(40, 180, 60), kAfterOpacity, true);
    m_afterBlue = makeSeries("B", QColor(40, 90, 220), kAfterOpacity, true);

    // original
    m_beforeRed = makeSeries("R (before)", QColor(220, 40, 40), kBeforeOpacity, false);
    m_beforeGreen = makeSeries("G (before)", QColor(40, 180, 60), kBeforeOpacity, false);
    m_beforeBlue = makeSeries("B (before)", QColor(40, 90, 220), kBeforeOpacity, false);

    m_axisX = new QValueAxis();
    m_axisX->setRange(0, 255);
    m_axisX->setTitleText("Value");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    m_axisY = new QValueAxis();
    m_axisY->setRange(0, 1);
    m_axisY->setTitleText("Count");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    const auto allSeries = { m_afterRed, m_afterGreen, m_afterBlue,
                              m_beforeRed, m_beforeGreen, m_beforeBlue };
    for (auto* series : allSeries)
    {
        series->attachAxis(m_axisX);
        series->attachAxis(m_axisY);
    }

    m_chartView = new QChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_chartView);
}

HistogramWidget::ChannelBins HistogramWidget::computeHistogram(const QImage& image)
{
    ChannelBins bins;
    if (image.isNull())
        return bins;

    const QImage img = image.convertToFormat(QImage::Format_ARGB32);
    const int width = img.width();
    const int height = img.height();

    for (int y = 0; y < height; ++y)
    {
        const auto* pixels = reinterpret_cast<const QRgb*>(img.constScanLine(y));
        for (int x = 0; x < width; ++x)
        {
            const QRgb pixel = pixels[x];
            ++bins.red[qRed(pixel)];
            ++bins.green[qGreen(pixel)];
            ++bins.blue[qBlue(pixel)];
        }
    }
    return bins;
}

int HistogramWidget::maxBin(const ChannelBins& bins)
{
    int result = 1;
    for (int i = 0; i < 256; ++i)
        result = std::max({ result, bins.red[i], bins.green[i], bins.blue[i] });
    return result;
}

void HistogramWidget::updateSeries(QLineSeries* r, QLineSeries* g, QLineSeries* b, const ChannelBins& bins)
{
    QList<QPointF> redPoints, greenPoints, bluePoints;
    redPoints.reserve(256);
    greenPoints.reserve(256);
    bluePoints.reserve(256);

    for (int i = 0; i < 256; ++i)
    {
        redPoints.append(QPointF(i, bins.red[i]));
        greenPoints.append(QPointF(i, bins.green[i]));
        bluePoints.append(QPointF(i, bins.blue[i]));
    }

    r->replace(redPoints);
    g->replace(greenPoints);
    b->replace(bluePoints);
}

void HistogramWidget::rescaleYAxis()
{
    m_axisY->setRange(0, std::max(m_maxBeforeCount, m_maxAfterCount));
}

void HistogramWidget::onImageLoaded(const QImage& image)
{
    const ChannelBins bins = computeHistogram(image);
    m_maxBeforeCount = m_maxAfterCount = maxBin(bins);

    updateSeries(m_beforeRed, m_beforeGreen, m_beforeBlue, bins);
    updateSeries(m_afterRed, m_afterGreen, m_afterBlue, bins);

    for (auto* series : { m_beforeRed, m_beforeGreen, m_beforeBlue })
        series->setVisible(false);
    for (auto* series : { m_afterRed, m_afterGreen, m_afterBlue })
        series->setVisible(true);

    rescaleYAxis();
}

void HistogramWidget::onImageChanged(const QImage& image)
{
    const ChannelBins bins = computeHistogram(image);
    m_maxAfterCount = maxBin(bins);
    updateSeries(m_afterRed, m_afterGreen, m_afterBlue, bins);

    for (auto* series : { m_beforeRed, m_beforeGreen, m_beforeBlue })
        series->setVisible(true);

    rescaleYAxis();
}