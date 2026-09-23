#pragma once

#include <QWidget>
#include <QImage>
#include <array>

#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>

class HistogramWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HistogramWidget(QWidget* parent = nullptr);

public slots:
    void onImageLoaded(const QImage& image);
    void onImageChanged(const QImage& image);

private:
    using Bins = std::array<int, 256>;

    struct ChannelBins
    {
        Bins red{};
        Bins green{};
        Bins blue{};
    };

    static ChannelBins computeHistogram(const QImage& image);
    static int maxBin(const ChannelBins& bins);

    void updateSeries(QLineSeries* r, QLineSeries* g, QLineSeries* b, const ChannelBins& bins);
    void rescaleYAxis();

    QChart* m_chart;
    QChartView* m_chartView;
    QValueAxis* m_axisX;
    QValueAxis* m_axisY;

    // "before" (original) — semi-transparent, hidden until an edit exists to compare against.
    QLineSeries* m_beforeRed;
    QLineSeries* m_beforeGreen;
    QLineSeries* m_beforeBlue;

    // "after" (current) — fully opaque.
    QLineSeries* m_afterRed;
    QLineSeries* m_afterGreen;
    QLineSeries* m_afterBlue;

    int m_maxBeforeCount = 1;
    int m_maxAfterCount = 1;
};