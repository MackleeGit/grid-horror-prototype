#ifndef CURVE_DRAWING_HPP
#define CURVE_DRAWING_HPP

#include "SoftwareCanvas.hpp"
#include "LineDrawing.hpp"
#include "PolygonOps.hpp"
#include <vector>

/*
================================================================================
PURE C++ BÉZIER CURVES & SPLINES (DE CASTELJAU'S ALGORITHM)
================================================================================
(a) What concept does this introduce?
    Parametric Curves — specifically Cubic Bézier Curves evaluated using De Casteljau's 
    Algorithm (recursive linear interpolation / lerp between 4 control points P0, P1, P2, P3).

(b) Why is this the correct way to do it at this stage?
    Polynomial expansion of (1-t)^3 P0 + 3(1-t)^2 t P1 + 3(1-t) t^2 P2 + t^3 P3 is 
    algebraically fixed. De Casteljau's algorithm provides a numerically stable 
    geometric interpolation method applicable to splines of arbitrary degree.

(c) What would break if skipped?
    Smooth enemy patrol paths, organic flashlight flicker animation curves, and 
    curved UI elements could not be evaluated or rendered in software.
================================================================================
*/

class CurveDrawing {
private:
    static Point2D lerp(const Point2D& p0, const Point2D& p1, float t) {
        return { p0.x + t * (p1.x - p0.x), p0.y + t * (p1.y - p0.y) };
    }

public:
    // Evaluate single point on Cubic Bézier curve at parameter t in [0, 1] using De Casteljau's
    static Point2D deCasteljau(const Point2D& p0, const Point2D& p1, 
                               const Point2D& p2, const Point2D& p3, float t) {
        Point2D q0 = lerp(p0, p1, t);
        Point2D q1 = lerp(p1, p2, t);
        Point2D q2 = lerp(p2, p3, t);

        Point2D r0 = lerp(q0, q1, t);
        Point2D r1 = lerp(q1, q2, t);

        return lerp(r0, r1, t);
    }

    // Draw Cubic Bézier Curve using N linear subdivisions
    static void drawCubicBezier(SoftwareCanvas& canvas, 
                                const Point2D& p0, const Point2D& p1, 
                                const Point2D& p2, const Point2D& p3, 
                                int segments, uint32_t color) {
        Point2D prevPoint = p0;
        for (int i = 1; i <= segments; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(segments);
            Point2D currPoint = deCasteljau(p0, p1, p2, p3, t);

            LineDrawing::drawLine(canvas, static_cast<int>(prevPoint.x), static_cast<int>(prevPoint.y),
                                         static_cast<int>(currPoint.x), static_cast<int>(currPoint.y), color);
            prevPoint = currPoint;
        }
    }
};

#endif // CURVE_DRAWING_HPP
