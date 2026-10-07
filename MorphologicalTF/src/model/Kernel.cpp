#include "model/Kernel.h"

#include <algorithm>

Kernel::Kernel(int width, int height)
    : m_width(clampSize(width))
    , m_height(clampSize(height))
    , m_cells(static_cast<std::size_t>(m_width) * m_height, 1)
    , m_anchor(m_width / 2, m_height / 2)
{
}

int Kernel::clampSize(int size)
{
    return std::clamp(size, kMinSize, kMaxSize);
}

bool Kernel::contains(int x, int y) const
{
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

bool Kernel::at(int x, int y) const
{
    return contains(x, y) && m_cells[index(x, y)] != 0;
}

int Kernel::activeCount() const
{
    return static_cast<int>(std::count_if(m_cells.begin(), m_cells.end(),
                                          [](std::uint8_t cell) { return cell != 0; }));
}

void Kernel::set(int x, int y, bool active)
{
    if (contains(x, y))
        m_cells[index(x, y)] = active ? 1 : 0;
}

void Kernel::setAnchor(int x, int y)
{
    if (contains(x, y))
        m_anchor = QPoint(x, y);
}

void Kernel::resize(int width, int height)
{
    const int newWidth = clampSize(width);
    const int newHeight = clampSize(height);

    std::vector<std::uint8_t> cells(static_cast<std::size_t>(newWidth) * newHeight, 0);
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

void Kernel::clear()
{
    std::fill(m_cells.begin(), m_cells.end(), 0);
}

void Kernel::fill()
{
    std::fill(m_cells.begin(), m_cells.end(), 1);
}

void Kernel::invert()
{
    for (std::uint8_t &cell : m_cells)
        cell = cell ? 0 : 1;
}

void Kernel::flipHorizontal()
{
    for (int y = 0; y < m_height; ++y) {
        auto row = m_cells.begin() + static_cast<std::ptrdiff_t>(y) * m_width;
        std::reverse(row, row + m_width);
    }
    m_anchor.setX(m_width - 1 - m_anchor.x());
}

void Kernel::flipVertical()
{
    for (int y = 0; y < m_height / 2; ++y) {
        auto top = m_cells.begin() + static_cast<std::ptrdiff_t>(y) * m_width;
        auto bottom = m_cells.begin() + static_cast<std::ptrdiff_t>(m_height - 1 - y) * m_width;
        std::swap_ranges(top, top + m_width, bottom);
    }
    m_anchor.setY(m_height - 1 - m_anchor.y());
}

void  Kernel::move(int dx, int dy)
{
    if (dx == 0 && dy == 0) return;

    int minX = m_width;
    int maxX = -1;
    int minY = m_height;
    int maxY = -1;

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if (m_cells[y * m_width + x] != 0) {
                minX = std::min(minX, x);
                maxX = std::max(maxX, x);
                minY = std::min(minY, y);
                maxY = std::max(maxY, y);
            }
        }
    }

    if (maxX == -1)
        return; // matrix contains no 1s

    dx = std::clamp(dx, -minX, m_width - 1 - maxX);
    dy = std::clamp(dy, -minY, m_height - 1 - maxY);

    std::vector<uint8_t> shifted(m_cells.size(), 0);

    for (int y = minY; y < maxY + 1; ++y) {
        for (int x = minX; x < maxX + 1; ++x) {
            if (m_cells[y * m_width + x] == 1) {
                int newX = x + dx;
                int newY = y + dy;

                shifted[newY * m_width + newX] = 1;
            }
        }
    }

    m_cells = std::move(shifted);

	m_anchor.setX(std::clamp(m_anchor.x() + dx, 0, m_width - 1));
	m_anchor.setY(std::clamp(m_anchor.y() + dy, 0, m_height - 1));
}


Kernel Kernel::reflected() const
{
    Kernel result = *this;
    result.flipHorizontal();
    result.flipVertical();
    return result;
}

QList<QPoint> Kernel::offsets() const
{
    QList<QPoint> result;
    result.reserve(activeCount());
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if (m_cells[index(x, y)])
                result.append(QPoint(x - m_anchor.x(), y - m_anchor.y()));
        }
    }
    return result;
}

bool Kernel::operator==(const Kernel &other) const
{
    return m_width == other.m_width && m_height == other.m_height
        && m_anchor == other.m_anchor && m_cells == other.m_cells;
}
