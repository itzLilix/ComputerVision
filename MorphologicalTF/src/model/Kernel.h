#pragma once

#include <QList>
#include <QPoint>

#include <cstdint>
#include <vector>

// Structuring element: a width x height binary matrix plus an anchor cell.
// A plain value type: cheap to copy, no Qt object machinery.
// Coordinates are (x, y) = (column, row), the origin is the top-left cell.
class Kernel
{
public:
    static constexpr int kMinSize = 1;
    static constexpr int kMaxSize = 101;

    // All cells active, the anchor is in the center (index size / 2)
    explicit Kernel(int width = 3, int height = 3);

    static int clampSize(int size);

    int width() const { return m_width; }
    int height() const { return m_height; }
    QPoint anchor() const { return m_anchor; }

    bool contains(int x, int y) const; // is (x, y) inside the matrix
    bool at(int x, int y) const;       // false when out of range
    int activeCount() const;
    bool isEmpty() const { return activeCount() == 0; }

    // Out-of-range coordinates are ignored by the setters
    void set(int x, int y, bool active);
    void setAnchor(int x, int y);

    // Keeps the overlapping cells (top-left aligned); added cells are inactive.
    // The anchor keeps its cell index and is clamped if it falls outside.
    void resize(int width, int height);

    void clear();
    void fill();
    void invert();
    void flipHorizontal(); // the anchor is mirrored together with the cells
    void flipVertical();
    void move(int dx, int dy); // moves the kernel by (dx, dy)

    // The kernel rotated by 180 degrees (this is what dilation needs)
    Kernel reflected() const;

    // Active cells as offsets from the anchor, e.g. (-1, 0), (0, 0), (1, 0)
    QList<QPoint> offsets() const;

    bool operator==(const Kernel &other) const;
    bool operator!=(const Kernel &other) const { return !(*this == other); }

private:
    int index(int x, int y) const { return y * m_width + x; }

    int m_width;
    int m_height;
    std::vector<std::uint8_t> m_cells; // row-major, 0 or 1
    QPoint m_anchor;
};
