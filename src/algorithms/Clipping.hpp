#ifndef CLIPPING_HPP
#define CLIPPING_HPP

#include "SoftwareCanvas.hpp"
#include "LineDrawing.hpp"

// Cohen-Sutherland line clipping — assigns a 4-bit outcode to each endpoint
// (LEFT/RIGHT/BOTTOM/TOP) and iteratively clips the line to the window boundary.
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
                // Both endpoints inside — trivial accept
                accept = true;
                break;
            } else if (code0 & code1) {
                // Both on same outside side — trivial reject
                break;
            } else {
                // Clip against the boundary the outside endpoint is beyond
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
                    x0 = x; y0 = y;
                    code0 = computeOutCode(x0, y0, xMin, yMin, xMax, yMax);
                } else {
                    x1 = x; y1 = y;
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
