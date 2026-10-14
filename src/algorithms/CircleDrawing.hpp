#ifndef CIRCLE_DRAWING_HPP
#define CIRCLE_DRAWING_HPP

#include "SoftwareCanvas.hpp"
#include "LineDrawing.hpp"
#include <cmath>

// Midpoint circle algorithm — tracks a decision parameter p = 1 - r and
// exploits 8-way symmetry so only one octant needs to be computed.
class CircleDrawing {
private:
    // Plot one point in each of the 8 symmetric positions around (xc, yc)
    static void draw8SymmetricPoints(SoftwareCanvas& canvas, int xc, int yc, int x, int y, uint32_t color) {
        canvas.setPixel(xc + x, yc + y, color);
        canvas.setPixel(xc - x, yc + y, color);
        canvas.setPixel(xc + x, yc - y, color);
        canvas.setPixel(xc - x, yc - y, color);
        canvas.setPixel(xc + y, yc + x, color);
        canvas.setPixel(xc - y, yc + x, color);
        canvas.setPixel(xc + y, yc - x, color);
        canvas.setPixel(xc - y, yc - x, color);
    }

public:
    // Outline circle
    static void drawCircle(SoftwareCanvas& canvas, int xc, int yc, int r, uint32_t color) {
        int x = 0;
        int y = r;
        int p = 1 - r;

        draw8SymmetricPoints(canvas, xc, yc, x, y, color);

        while (x < y) {
            x++;
            if (p < 0) {
                p += 2 * x + 1;
            } else {
                y--;
                p += 2 * (x - y) + 1;
            }
            draw8SymmetricPoints(canvas, xc, yc, x, y, color);
        }
    }

    // Filled circle — same algorithm, horizontal scanlines between symmetric points
    static void drawFilledCircle(SoftwareCanvas& canvas, int xc, int yc, int r, uint32_t color) {
        int x = 0;
        int y = r;
        int p = 1 - r;

        auto fillHorizontalScanline = [&](int yVal, int xLeft, int xRight) {
            LineDrawing::drawLine(canvas, xLeft, yVal, xRight, yVal, color);
        };

        fillHorizontalScanline(yc + y, xc - x, xc + x);
        fillHorizontalScanline(yc - y, xc - x, xc + x);
        fillHorizontalScanline(yc + x, xc - y, xc + y);
        fillHorizontalScanline(yc - x, xc - y, xc + y);

        while (x < y) {
            x++;
            if (p < 0) {
                p += 2 * x + 1;
            } else {
                y--;
                p += 2 * (x - y) + 1;
            }
            fillHorizontalScanline(yc + y, xc - x, xc + x);
            fillHorizontalScanline(yc - y, xc - x, xc + x);
            fillHorizontalScanline(yc + x, xc - y, xc + y);
            fillHorizontalScanline(yc - x, xc - y, xc + y);
        }
    }
};

#endif // CIRCLE_DRAWING_HPP
