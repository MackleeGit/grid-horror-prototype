#ifndef CIRCLE_DRAWING_HPP
#define CIRCLE_DRAWING_HPP

#include "SoftwareCanvas.hpp"
#include "LineDrawing.hpp"
#include <cmath>

/*
================================================================================
PURE C++ MIDPOINT CIRCLE ALGORITHM
================================================================================
(a) What concept does this introduce?
    The Midpoint Circle Algorithm — a scan-conversion technique that computes 
    pixels for a circular arc by tracking a decision parameter P = 1 - r, taking 
    advantage of 8-way circular symmetry to calculate only one octant (45 degrees).

(b) Why is this the correct way to do it at this stage?
    Direct evaluation of x^2 + y^2 = r^2 involves square roots, while trigonometric 
    evaluation (r*cos(theta), r*sin(theta)) is slow. Midpoint circle uses fast integer 
    addition and decision parameter updates.

(c) What would break if skipped?
    Player position indicators, sound detection footprints, and monster proximity radar 
    pulses could not be rendered in the software minimap HUD.
================================================================================
*/

class CircleDrawing {
private:
    // Utility to plot 8 symmetric points relative to center (xc, yc)
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
    // Outline Circle using Midpoint Circle Algorithm
    static void drawCircle(SoftwareCanvas& canvas, int xc, int yc, int r, uint32_t color) {
        int x = 0;
        int y = r;
        int p = 1 - r; // Initial decision parameter

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

    // Filled Circle using Midpoint Circle with horizontal scanlines
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
