#include "view/KernelGridView.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPalette>

#include <algorithm>

namespace {
constexpr int kMaxCellSize = 40; // keeps small kernels from becoming huge
constexpr int kMinCellSizeForLines = 6;
} // namespace

KernelGridView::KernelGridView(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QSize KernelGridView::sizeHint() const
{
    return QSize(240, 240);
}

QSize KernelGridView::minimumSizeHint() const
{
    return QSize(120, 120);
}

void KernelGridView::setKernel(const Kernel &kernel)
{
    m_kernel = kernel;
    update();
}

void KernelGridView::setAnchorPickMode(bool enabled)
{
    m_pickAnchor = enabled;
    if (enabled)
        setCursor(Qt::CrossCursor);
    else
        unsetCursor();
}

int KernelGridView::cellSize() const
{
    const int byWidth = width() / m_kernel.width();
    const int byHeight = height() / m_kernel.height();
    return std::clamp(std::min(byWidth, byHeight), 1, kMaxCellSize);
}

QRect KernelGridView::gridRect() const
{
    const int cell = cellSize();
    const QSize size(cell * m_kernel.width(), cell * m_kernel.height());
    const QPoint topLeft((width() - size.width()) / 2, (height() - size.height()) / 2);
    return QRect(topLeft, size);
}

QPoint KernelGridView::cellAt(const QPoint &pos) const
{
    const QRect area = gridRect();
    if (!area.contains(pos))
        return QPoint(-1, -1);

    const int cell = cellSize();
    return QPoint((pos.x() - area.left()) / cell, (pos.y() - area.top()) / cell);
}

void KernelGridView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    const QRect area = gridRect();
    const int cell = cellSize();
    const QColor activeColor = palette().color(QPalette::Highlight);
    const QColor inactiveColor = palette().color(QPalette::Base);
    const QColor lineColor = palette().color(QPalette::Mid);

    for (int y = 0; y < m_kernel.height(); ++y) {
        for (int x = 0; x < m_kernel.width(); ++x) {
            const QRect rect(area.left() + x * cell, area.top() + y * cell, cell, cell);
            painter.fillRect(rect, m_kernel.at(x, y) ? activeColor : inactiveColor);
        }
    }

    painter.setPen(lineColor);
    if (cell >= kMinCellSizeForLines) {
        for (int x = 0; x <= m_kernel.width(); ++x)
            painter.drawLine(area.left() + x * cell, area.top(), area.left() + x * cell, area.bottom() + 1);
        for (int y = 0; y <= m_kernel.height(); ++y)
            painter.drawLine(area.left(), area.top() + y * cell, area.right() + 1, area.top() + y * cell);
    } else {
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(area); // only a frame: lines would hide tiny cells
    }

    // Anchor: thin red circle on its cell
    const QPoint anchor = m_kernel.anchor();
    const QPointF center(area.left() + (anchor.x() + 0.5) * cell, area.top() + (anchor.y() + 0.5) * cell);
    const double radius = std::max(1.5, cell * 0.3);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::red, 1.5));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(center, radius, radius);
}

void KernelGridView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    const QPoint cell = cellAt(event->position().toPoint());
    if (cell.x() < 0)
        return;

    if (m_pickAnchor) {
        emit anchorPicked(cell.x(), cell.y());
        setAnchorPickMode(false);
        emit anchorPickFinished();
        return;
    }

    m_painting = true;
    m_paintValue = !m_kernel.at(cell.x(), cell.y()); // the first cell decides the drag value
    m_lastCell = cell;
    emit cellPainted(cell.x(), cell.y(), m_paintValue);
}

void KernelGridView::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_painting)
        return;

    const QPoint cell = cellAt(event->position().toPoint());
    if (cell.x() < 0 || cell == m_lastCell)
        return;

    m_lastCell = cell;
    emit cellPainted(cell.x(), cell.y(), m_paintValue);
}

void KernelGridView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_painting = false;
        m_lastCell = QPoint(-1, -1);
    }
}
