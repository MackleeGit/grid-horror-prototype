#ifndef ELLIPSE_DRAWING_HPP
#define ELLIPSE_DRAWING_HPP

#include "SoftwareCanvas.hpp"
#include "LineDrawing.hpp"
#include <cmath>
#include <cstdint>

/**
 * ============================================================================
 * MIDPOINT ELLIPSE ALGORITHM (PURE C++)
 * ============================================================================
 * Evaluates discrete raster positions along an ellipse boundary with semi-major
 * axis rx and semi-minor axis ry, centered at (xc, yc), using integer decision
 * parameters and 4-way symmetry.
 *
 * Mathematical Formulation:
 * -------------------------
 * Implicit ellipse equation:
 *   f(x, y) = ry^2 * x^2 + rx^2 * y^2 - rx^2 * ry^2 = 0
 *
 * Region 1: dy/dx >= -1  (Starts at (0, ry), steps x by +1)
 *   Condition: 2 * ry^2 * x <= 2 * rx^2 * y
 *   Initial decision parameter:
 *     p1_0 = ry^2 - rx^2 * ry + 0.25 * rx^2
 *   Update rules:
 *     If p1_k < 0:  p1_{k+1} = p1_k + 2 * ry^2 * x_{k+1} + ry^2
 *     Else:         p1_{k+1} = p1_k + 2 * ry^2 * x_{k+1} - 2 * rx^2 * y_{k+1} + ry^2
 *
 * Region 2: dy/dx < -1  (Steps y by -1 until y == 0)
 *   Initial decision parameter:
 *     p2_0 = ry^2 * (x0 + 0.5)^2 + rx^2 * (y0 - 1)^2 - rx^2 * ry^2
 *   Update rules:
 *     If p2_k > 0:  p2_{k+1} = p2_k - 2 * rx^2 * y_{k+1} + rx^2
 *     Else:         p2_{k+1} = p2_k + 2 * ry^2 * x_{k+1} - 2 * rx^2 * y_{k+1} + rx^2
 *
 * Project Application:
 * -------------------
 * - Acoustic Sound Radius: Elliptical sound propagation footprint when player
 *   sprints or crouches along directional headings.
 * - Projected Flashlight Footprint: Oblique angle projection of flashlight
 *   spotlight cone onto the floor plane.
 * ============================================================================
 */
class EllipseDrawing {
private:
    static void plot4SymmetricPoints(SoftwareCanvas& canvas, int xc, int yc, int x, int y, uint32_t color) {
        canvas.setPixel(xc + x, yc + y, color);
        canvas.setPixel(xc - x, yc + y, color);
        canvas.setPixel(xc + x, yc - y, color);
        canvas.setPixel(xc - x, yc - y, color);
    }

public:
    // Outline Midpoint Ellipse Algorithm
    static void drawEllipse(SoftwareCanvas& canvas, int xc, int yc, int rx, int ry, uint32_t color) {
        if (rx <= 0 || ry <= 0) return;

        float rxSq = static_cast<float>(rx * rx);
        float rySq = static_cast<float>(ry * ry);
        float twoRxSq = 2.0f * rxSq;
        float twoRySq = 2.0f * rySq;

        // --- Region 1 ---
        float x = 0.0f;
        float y = static_cast<float>(ry);
        float p1 = rySq - (rxSq * ry) + (0.25f * rxSq);
        float dx = twoRySq * x;
        float dy = twoRxSq * y;

        while (dx < dy) {
            plot4SymmetricPoints(canvas, xc, yc, static_cast<int>(x), static_cast<int>(y), color);
            x += 1.0f;
            dx += twoRySq;
            if (p1 < 0.0f) {
                p1 += dx + rySq;
            } else {
                y -= 1.0f;
                dy -= twoRxSq;
                p1 += dx - dy + rySq;
            }
        }

        // --- Region 2 ---
        float p2 = (rySq * (x + 0.5f) * (x + 0.5f)) +
                   (rxSq * (y - 1.0f) * (y - 1.0f)) -
                   (rxSq * rySq);

        while (y >= 0.0f) {
            plot4SymmetricPoints(canvas, xc, yc, static_cast<int>(x), static_cast<int>(y), color);
            y -= 1.0f;
            dy -= twoRxSq;
            if (p2 > 0.0f) {
                p2 += rxSq - dy;
            } else {
                x += 1.0f;
                dx += twoRySq;
                p2 += dx - dy + rxSq;
            }
        }
    }

    // Filled Midpoint Ellipse: rasterizes horizontal scanlines across symmetric pairs
    static void drawFilledEllipse(SoftwareCanvas& canvas, int xc, int yc, int rx, int ry, uint32_t color) {
        if (rx <= 0 || ry <= 0) return;

        float rxSq = static_cast<float>(rx * rx);
        float rySq = static_cast<float>(ry * ry);
        float twoRxSq = 2.0f * rxSq;
        float twoRySq = 2.0f * rySq;

        auto fillScanlinePair = [&](int curY, int curX) {
            LineDrawing::drawLine(canvas, xc - curX, yc + curY, xc + curX, yc + curY, color);
            if (curY != 0) {
                LineDrawing::drawLine(canvas, xc - curX, yc - curY, xc + curX, yc - curY, color);
            }
        };

        // --- Region 1 ---
        float x = 0.0f;
        float y = static_cast<float>(ry);
        float p1 = rySq - (rxSq * ry) + (0.25f * rxSq);
        float dx = twoRySq * x;
        float dy = twoRxSq * y;

        int lastFilledY = -1;

        while (dx < dy) {
            if (static_cast<int>(y) != lastFilledY) {
                fillScanlinePair(static_cast<int>(y), static_cast<int>(x));
                lastFilledY = static_cast<int>(y);
            }
            x += 1.0f;
            dx += twoRySq;
            if (p1 < 0.0f) {
                p1 += dx + rySq;
            } else {
                y -= 1.0f;
                dy -= twoRxSq;
                p1 += dx - dy + rySq;
            }
        }

        // --- Region 2 ---
        float p2 = (rySq * (x + 0.5f) * (x + 0.5f)) +
                   (rxSq * (y - 1.0f) * (y - 1.0f)) -
                   (rxSq * rySq);

        while (y >= 0.0f) {
            fillScanlinePair(static_cast<int>(y), static_cast<int>(x));
            y -= 1.0f;
            dy -= twoRxSq;
            if (p2 > 0.0f) {
                p2 += rxSq - dy;
            } else {
                x += 1.0f;
                dx += twoRySq;
                p2 += dx - dy + rxSq;
            }
        }
    }
};

#endif // ELLIPSE_DRAWING_HPP
