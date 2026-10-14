#ifndef CURVE_DRAWING_HPP
#define CURVE_DRAWING_HPP

#include "SoftwareCanvas.hpp"
#include "LineDrawing.hpp"
#include "PolygonOps.hpp"
#include <vector>

// De Casteljau's algorithm for cubic Bézier curves — recursively lerps between
// control points P0..P3 at parameter t to find a point on the curve.
class CurveDrawing {
private:
    static Point2D lerp(const Point2D& p0, const Point2D& p1, float t) {
        return { p0.x + t * (p1.x - p0.x), p0.y + t * (p1.y - p0.y) };
    }

public:
    // Evaluate a single point on the curve at t in [0, 1]
    static Point2D deCasteljau(const Point2D& p0, const Point2D& p1,
                               const Point2D& p2, const Point2D& p3, float t) {
        Point2D q0 = lerp(p0, p1, t);
        Point2D q1 = lerp(p1, p2, t);
        Point2D q2 = lerp(p2, p3, t);

        Point2D r0 = lerp(q0, q1, t);
        Point2D r1 = lerp(q1, q2, t);

        return lerp(r0, r1, t);
    }

    // Draw the curve by connecting N evenly-spaced evaluated points with Bresenham lines
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
