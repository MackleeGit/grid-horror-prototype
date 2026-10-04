#ifndef CLIPPING_HPP
#define CLIPPING_HPP

#include "SoftwareCanvas.hpp"
#include "LineDrawing.hpp"

/*
================================================================================
PURE C++ COHEN-SUTHERLAND LINE CLIPPING ALGORITHM
================================================================================
(a) What concept does this introduce?
    Cohen-Sutherland Line Clipping — divides 2D clipping space into nine 9-regions 
    using 4-bit outcodes (INSIDE, LEFT, RIGHT, BOTTOM, TOP) to quickly identify 
    trivially accepted/rejected line segments and calculate intersection points.

(b) Why is this the correct way to do it at this stage?
    Drawing primitive lines beyond view viewport/HUD bounds causes pixel overflow 
    or wasteful rasterization. Cohen-Sutherland performs bitwise outcode tests for 
    O(1) trivial rejection before executing line intersection division math.

(c) What would break if skipped?
    Lines drawn on the HUD (such as raycast sightlines extending outside the 
    minimap window) would spill over into the main 3D viewport.
================================================================================
*/

class CohenSutherlandClipping {
private:
    static constexpr int INSIDE = 0; // 0000
    static constexpr int LEFT   = 1; // 0001
    static constexpr int RIGHT  = 2; // 0010
    static constexpr int BOTTOM = 4; // 0100
    static constexpr int TOP    = 8; // 1000

    static int computeOutCode(float x, float y, float xMin, float yMin, float xMax, float yMax) {
        int code = INSIDE;
        if (x < xMin)      code |= LEFT;
        else if (x > xMax) code |= RIGHT;
        if (y < yMin)      code |= BOTTOM;
        else if (y > yMax) code |= TOP;
        return code;
    }

public:
    static bool clipAndDrawLine(SoftwareCanvas& canvas, 
                                float x0, float y0, float x1, float y1,
                                float xMin, float yMin, float xMax, float yMax,
                                uint32_t color) {
        int code0 = computeOutCode(x0, y0, xMin, yMin, xMax, yMax);
        int code1 = computeOutCode(x1, y1, xMin, yMin, xMax, yMax);
        bool accept = false;

        while (true) {
            if ((code0 | code1) == 0) {
                // Trivial accept: both points are inside window
                accept = true;
                break;
            } else if (code0 & code1) {
                // Trivial reject: both points share an outside region
                break;
            } else {
                // Calculate line intersection with window border
                float x = 0.0f, y = 0.0f;
                int outcodeOut = code0 ? code0 : code1;

                if (outcodeOut & TOP) {
                    x = x0 + (x1 - x0) * (yMax - y0) / (y1 - y0);
                    y = yMax;
                } else if (outcodeOut & BOTTOM) {
                    x = x0 + (x1 - x0) * (yMin - y0) / (y1 - y0);
                    y = yMin;
                } else if (outcodeOut & RIGHT) {
                    y = y0 + (y1 - y0) * (xMax - x0) / (x1 - x0);
                    x = xMax;
                } else if (outcodeOut & LEFT) {
                    y = y0 + (y1 - y0) * (xMin - x0) / (x1 - x0);
                    x = xMin;
                }

                if (outcodeOut == code0) {
                    x0 = x;
                    y0 = y;
                    code0 = computeOutCode(x0, y0, xMin, yMin, xMax, yMax);
                } else {
                    x1 = x;
                    y1 = y;
                    code1 = computeOutCode(x1, y1, xMin, yMin, xMax, yMax);
                }
            }
        }

        if (accept) {
            LineDrawing::drawLine(canvas, static_cast<int>(x0), static_cast<int>(y0),
                                         static_cast<int>(x1), static_cast<int>(y1), color);
        }
        return accept;
    }
};

#endif // CLIPPING_HPP
