#ifndef LINE_DRAWING_HPP
#define LINE_DRAWING_HPP

#include "SoftwareCanvas.hpp"
#include <cmath>
#include <cstdlib>

/*
================================================================================
PURE C++ BRESENHAM'S LINE DRAWING ALGORITHM
================================================================================
(a) What concept does this introduce?
    Bresenham's Line Algorithm — an efficient incremental rasterization algorithm 
    that uses integer-only addition, subtraction, and bit shifting to draw a line 
    between two endpoints (x0, y0) and (x1, y1) on a discrete 2D grid.

(b) Why is this the correct way to do it at this stage?
    Floating-point line equations (y = mx + c) require expensive floating-point 
    multiplication and rounding for every pixel. Bresenham's algorithm maintains 
    an integer error term 'p' to decide which pixel grid neighbor to step into.

(c) What would break if skipped?
    Without custom line drawing, drawing 2D vector elements (minimap grid outlines,
    radar sight vectors, waypoints) would rely entirely on GPU primitive assembly.
================================================================================
*/

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
