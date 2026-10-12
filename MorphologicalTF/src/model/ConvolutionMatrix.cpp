#include "model/ConvolutionMatrix.h"

#include <algorithm>
#include <cmath>

ConvolutionMatrix::ConvolutionMatrix(int width, int height)
    : m_width(clampSize(width))
    , m_height(clampSize(height))
    , m_cells(static_cast<std::size_t>(m_width) * m_height, 0.0)
    , m_anchor(m_width / 2, m_height / 2)
{
    m_cells[index(m_anchor.x(), m_anchor.y())] = 1.0;
}

int ConvolutionMatrix::clampSize(int size)
{
    return std::clamp(size, kMinSize, kMaxSize);
}

bool ConvolutionMatrix::contains(int x, int y) const
{
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

double ConvolutionMatrix::at(int x, int y) const
{
    return contains(x, y) ? m_cells[index(x, y)] : 0.0;
}

double ConvolutionMatrix::sum() const
{
    double total = 0.0;
    for (double cell : m_cells)
        total += cell;
    return total;
}

bool ConvolutionMatrix::isEmpty() const
{
    return std::all_of(m_cells.begin(), m_cells.end(), [](double cell) { return cell == 0.0; });
}

void ConvolutionMatrix::set(int x, int y, double weight)
{
    if (contains(x, y))
        m_cells[index(x, y)] = weight;
}

void ConvolutionMatrix::setAnchor(int x, int y)
{
    if (contains(x, y))
        m_anchor = QPoint(x, y);
}

void ConvolutionMatrix::setDivisor(double divisor)
{
    if (divisor != 0.0)
        m_divisor = divisor;
}

void ConvolutionMatrix::setBias(double bias)
{
    m_bias = bias;
}

void ConvolutionMatrix::normalize()
{
    const double total = sum();
    m_divisor = (std::abs(total) > 1e-9) ? total : 1.0; // a zero-sum matrix can sum to ~1e-17
}

void ConvolutionMatrix::resize(int width, int height)
{
    const int newWidth = clampSize(width);
    const int newHeight = clampSize(height);

    std::vector<double> cells(static_cast<std::size_t>(newWidth) * newHeight, 0.0);
    const int copyWidth = std::min(m_width, newWidth);
    const int copyHeight = std::min(m_height, newHeight);
    for (int y = 0; y < copyHeight; ++y) {
        for (int x = 0; x < copyWidth; ++x)
            cells[static_cast<std::size_t>(y) * newWidth + x] = m_cells[index(x, y)]; // old width
    }

    m_width = newWidth;
    m_height = newHeight;
    m_cells = std::move(cells);
    m_anchor = QPoint(std::min(m_anchor.x(), m_width - 1), std::min(m_anchor.y(), m_height - 1));
}

void ConvolutionMatrix::clear()
{
    std::fill(m_cells.begin(), m_cells.end(), 0.0);
}

void ConvolutionMatrix::fill(double weight)
{
    std::fill(m_cells.begin(), m_cells.end(), weight);
}

void ConvolutionMatrix::flipHorizontal()
{
    for (int y = 0; y < m_height; ++y) {
        auto row = m_cells.begin() + static_cast<std::ptrdiff_t>(y) * m_width;
        std::reverse(row, row + m_width);
    }
    m_anchor.setX(m_width - 1 - m_anchor.x());
}

void ConvolutionMatrix::flipVertical()
{
    for (int y = 0; y < m_height / 2; ++y) {
        auto top = m_cells.begin() + static_cast<std::ptrdiff_t>(y) * m_width;
        auto bottom = m_cells.begin() + static_cast<std::ptrdiff_t>(m_height - 1 - y) * m_width;
        std::swap_ranges(top, top + m_width, bottom);
    }
    m_anchor.setY(m_height - 1 - m_anchor.y());
}

void ConvolutionMatrix::negate()
{
    for (double &cell : m_cells)
        cell = (cell == 0.0) ? 0.0 : -cell; // avoids -0.0
}

void ConvolutionMatrix::move(int dx, int dy)
{
    if (dx == 0 && dy == 0)
        return;

    int minX = m_width;
    int maxX = -1;
    int minY = m_height;
    int maxY = -1;

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if (m_cells[index(x, y)] != 0.0) {
                minX = std::min(minX, x);
                maxX = std::max(maxX, x);
                minY = std::min(minY, y);
                maxY = std::max(maxY, y);
            }
        }
    }

    if (maxX == -1)
        return; // matrix contains only zeros

    dx = std::clamp(dx, -minX, m_width - 1 - maxX);
    dy = std::clamp(dy, -minY, m_height - 1 - maxY);

    std::vector<double> shifted(m_cells.size(), 0.0);
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x)
            shifted[index(x + dx, y + dy)] = m_cells[index(x, y)];
    }

    m_cells = std::move(shifted);
    m_anchor.setX(std::clamp(m_anchor.x() + dx, 0, m_width - 1));
    m_anchor.setY(std::clamp(m_anchor.y() + dy, 0, m_height - 1));
}

ConvolutionMatrix ConvolutionMatrix::reflected() const
{
    ConvolutionMatrix result = *this;
    result.flipHorizontal();
    result.flipVertical();
    return result;
}

QList<ConvolutionMatrix::Tap> ConvolutionMatrix::taps() const
{
    QList<Tap> result;
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            const double weight = m_cells[index(x, y)];
            if (weight != 0.0)
                result.append({QPoint(x - m_anchor.x(), y - m_anchor.y()), weight});
        }
    }
    return result;
}

bool ConvolutionMatrix::operator==(const ConvolutionMatrix &other) const
{
    return m_width == other.m_width && m_height == other.m_height
        && m_anchor == other.m_anchor && m_divisor == other.m_divisor
        && m_bias == other.m_bias && m_cells == other.m_cells;
}
