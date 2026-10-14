#ifndef LINE_DRAWING_HPP
#define LINE_DRAWING_HPP

#include "SoftwareCanvas.hpp"
#include <cmath>
#include <cstdlib>

// Bresenham's line algorithm — integer-only, uses an error term to
// decide which pixel to step to next. No floating point needed.
class LineDrawing {
public:
    static void drawLine(SoftwareCanvas& canvas, int x0, int y0, int x1, int y1, uint32_t color) {
        int dx = std::abs(x1 - x0);
        int dy = std::abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;

        int err = dx - dy;

        while (true) {
            canvas.setPixel(x0, y0, color);

            if (x0 == x1 && y0 == y1) break;

            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }
    }
};

#endif // LINE_DRAWING_HPP
