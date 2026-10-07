#pragma once

#include "model/Kernel.h"

#include <QWidget>

// Dumb widget that draws a kernel as a grid of square cells and reports user input.
// Click or drag to paint cells; the first cell touched decides whether the drag
// activates or deactivates. The anchor is drawn as a thin red circle.
class KernelGridView : public QWidget
{
    Q_OBJECT

public:
    explicit KernelGridView(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void setKernel(const Kernel &kernel);

    // While enabled, the next click picks the anchor instead of painting
    void setAnchorPickMode(bool enabled);

signals:
    void cellPainted(int x, int y, bool active);
    void anchorPicked(int x, int y);
    void anchorPickFinished();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    int cellSize() const;
    QRect gridRect() const;
    QPoint cellAt(const QPoint &pos) const; // (-1, -1) when outside the grid

    Kernel m_kernel;
    bool m_pickAnchor = false;
    bool m_painting = false;
    bool m_paintValue = false;
    QPoint m_lastCell{-1, -1};
};
