#pragma once

struct Bounds
{
    int minDx = 0;
    int maxDx = 0;
    int minDy = 0;
    int maxDy = 0;

    void include(const QPoint& p)
    {
        minDx = std::min(minDx, p.x());
        maxDx = std::max(maxDx, p.x());
        minDy = std::min(minDy, p.y());
        maxDy = std::max(maxDy, p.y());
    }
};