#pragma once

#include <QList>
#include <QPoint>

#include <vector>

// Convolution matrix: a width x height matrix of real weights plus an anchor cell,
// a divisor and a bias:  result = (sum of weight * pixel) / divisor + bias.
// A plain value type: cheap to copy, no Qt object machinery.
// Coordinates are (x, y) = (column, row), the origin is the top-left cell.
class ConvolutionMatrix
{
public:
    // A non-zero cell as an offset from the anchor
    struct Tap {
        QPoint offset;
        double weight;
    };

    static constexpr int kMinSize = 1;
    static constexpr int kMaxSize = 101;

    // Identity matrix (only the anchor cell is 1), the anchor is in the center (index size / 2)
    explicit ConvolutionMatrix(int width = 3, int height = 3);

    static int clampSize(int size);

    int width() const { return m_width; }
    int height() const { return m_height; }
    QPoint anchor() const { return m_anchor; }
    double divisor() const { return m_divisor; }
    double bias() const { return m_bias; }

    bool contains(int x, int y) const; // is (x, y) inside the matrix
    double at(int x, int y) const;     // 0 when out of range
    double sum() const;                // sum of all weights
    bool isEmpty() const;              // all weights are zero

    // Out-of-range coordinates are ignored by the setters
    void set(int x, int y, double weight);
    void setAnchor(int x, int y);
    void setDivisor(double divisor); // zero is ignored
    void setBias(double bias);
    void normalize();                // divisor = sum of weights (1 if the sum is about zero)

    // Keeps the overlapping cells (top-left aligned); added cells are 0.
    // The anchor keeps its cell index and is clamped if it falls outside.
    void resize(int width, int height);

    void clear();             // all weights = 0
    void fill(double weight); // all weights = weight
    void flipHorizontal();    // the anchor is mirrored together with the cells
    void flipVertical();
    void negate();             // every weight changes its sign
    void move(int dx, int dy); // moves the non-zero cells by (dx, dy), clamped so none leaves the matrix

    // The matrix rotated by 180 degrees (this is what true convolution needs)
    ConvolutionMatrix reflected() const;

    // Non-zero cells as offsets from the anchor, e.g. {(-1, 0), 0.5}, {(0, 0), 1.0}
    QList<Tap> taps() const;

    bool operator==(const ConvolutionMatrix &other) const;
    bool operator!=(const ConvolutionMatrix &other) const { return !(*this == other); }

private:
    int index(int x, int y) const { return y * m_width + x; }

    int m_width;
    int m_height;
    std::vector<double> m_cells; // row-major
    QPoint m_anchor;
    double m_divisor = 1.0;
    double m_bias = 0.0;
};
