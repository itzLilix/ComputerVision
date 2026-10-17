#include <QtConcurrent>
#include <QSet>
#include <array>
#include <numeric>
#include <vector>

namespace SlidingHistogram {

    struct Histogram {
        std::array<int, 256> fine{};
        std::array<int, 16>  coarse{};

        void update(int value, int delta) {
            fine[value] += delta;
            coarse[value >> 4] += delta;
        }

        // Value at 0-based sorted position `target`. Requires 0 <= target < count.
        int select(int target) const {
            int c = 0;
            while (target >= coarse[c]) { target -= coarse[c]; ++c; }
            int v = c * 16;
            while (target >= fine[v]) { target -= fine[v]; ++v; }
            return v;
        }
    };

    // Calls emit(index, histR, histG, histB, count) for every pixel.
    // Rows run in parallel; emit must only write to its own pixel.
    template <typename OnPixel>
    void slideRows(const QRgb* src, int width, int height,
        const QList<QPoint>& offsets, OnPixel onPixel)
    {
        // Pixels that leave / enter the window when it moves one step right
        const QSet<QPoint> shape(offsets.begin(), offsets.end());
        std::vector<QPoint> leaving, entering;
        for (const QPoint& o : offsets) {
            if (!shape.contains(o - QPoint(1, 0))) leaving.push_back(o);
            if (!shape.contains(o + QPoint(1, 0))) entering.push_back(o);
        }

        auto processRow = [&](int y) {
            Histogram hr, hg, hb;
            int count = 0;

            auto apply = [&](int cx, const auto& list, int delta) {
                for (const QPoint& o : list) {
                    const int sx = cx + o.x();
                    const int sy = y + o.y();
                    if (sx < 0 || sx >= width || sy < 0 || sy >= height)
                        continue;

                    const QRgb p = src[sy * width + sx];
                    hr.update(qRed(p), delta);
                    hg.update(qGreen(p), delta);
                    hb.update(qBlue(p), delta);
                    count += delta;
                }
                };

            apply(0, offsets, +1);

            for (int x = 0; x < width; ++x) {
                if (x > 0) {
                    apply(x - 1, leaving, -1);
                    apply(x, entering, +1);
                }
                onPixel(y * width + x, hr, hg, hb, count);
            }
            };

        std::vector<int> rows(height);
        std::iota(rows.begin(), rows.end(), 0);
        QtConcurrent::blockingMap(rows, processRow);
    }

} // namespace SlidingHistogram