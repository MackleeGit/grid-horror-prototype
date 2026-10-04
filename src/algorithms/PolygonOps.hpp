#ifndef POLYGON_OPS_HPP
#define POLYGON_OPS_HPP

#include "SoftwareCanvas.hpp"
#include "LineDrawing.hpp"
#include <vector>
#include <algorithm>
#include <cmath>

/*
================================================================================
PURE C++ POLYGON REPRESENTATION, TRANSFORMATIONS, & SCANLINE FILL
================================================================================
(a) What concept does this introduce?
    1. Polygon Representation & 2D Affine Transformations (Translation, Scaling, 
       Rotation via 2D Transformation Matrices).
    2. Scanline Polygon Filling — determining interior pixels of an arbitrary polygon 
       by calculating edge intersections along scanline Y-coordinates.

(b) Why is this the correct way to do it at this stage?
    Polygon transformation shows understanding of homogeneous 2D matrix math.
    Scanline fill avoids flood-fill stack overflows and efficiently fills solid geometry.

(c) What would break if skipped?
    Wall blocks and room outlines on the 2D software minimap HUD could not be 
    transformed, positioned, or filled cleanly in software pixel space.
================================================================================
*/

struct Point2D {
    float x;
    float y;
};

class Polygon2D {
public:
    std::vector<Point2D> vertices;

    Polygon2D() = default;
    Polygon2D(const std::vector<Point2D>& verts) : vertices(verts) {}

    // Translate polygon by (dx, dy)
    void translate(float dx, float dy) {
        for (auto& v : vertices) {
            v.x += dx;
            v.y += dy;
        }
    }

    // Scale polygon centered at origin by (sx, sy)
    void scale(float sx, float sy) {
        for (auto& v : vertices) {
            v.x *= sx;
            v.y *= sy;
        }
    }

    // Rotate polygon around origin by angle (radians)
    void rotate(float radians) {
        float cosA = std::cos(radians);
        float sinA = std::sin(radians);
        for (auto& v : vertices) {
            float newX = v.x * cosA - v.y * sinA;
            float newY = v.x * sinA + v.y * cosA;
            v.x = newX;
            v.y = newY;
        }
    }

    // Draw outline using Bresenham lines
    void drawOutline(SoftwareCanvas& canvas, uint32_t color) const {
        if (vertices.size() < 2) return;
        for (size_t i = 0; i < vertices.size(); ++i) {
            const Point2D& p1 = vertices[i];
            const Point2D& p2 = vertices[(i + 1) % vertices.size()];
            LineDrawing::drawLine(canvas, static_cast<int>(p1.x), static_cast<int>(p1.y),
                                         static_cast<int>(p2.x), static_cast<int>(p2.y), color);
        }
    }

    // Pure C++ Scanline Polygon Fill Algorithm
    void scanlineFill(SoftwareCanvas& canvas, uint32_t color) const {
        if (vertices.size() < 3) return;

        // Find bounding Y coordinates of the polygon
        float minY = vertices[0].y;
        float maxY = vertices[0].y;
        for (const auto& v : vertices) {
            if (v.y < minY) minY = v.y;
            if (v.y > maxY) maxY = v.y;
        }

        int startY = std::max(0, static_cast<int>(std::ceil(minY)));
        int endY = std::min(canvas.height - 1, static_cast<int>(std::floor(maxY)));

        // Iterate across every scanline Y
        for (int y = startY; y <= endY; ++y) {
            std::vector<int> nodeX;
            size_t n = vertices.size();

            for (size_t i = 0; i < n; ++i) {
                Point2D p1 = vertices[i];
                Point2D p2 = vertices[(i + 1) % n];

                if (p1.y > p2.y) std::swap(p1, p2);

                // Check if scanline y intersects edge (p1, p2)
                if (y >= p1.y && y < p2.y) {
                    if (std::abs(p2.y - p1.y) > 0.0001f) {
                        int xInt = static_cast<int>(p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y));
                        nodeX.push_back(xInt);
                    }
                }
            }

            // Sort intersection X points from left to right
            std::sort(nodeX.begin(), nodeX.end());

            // Fill pixel spans between pairs of intersections (parity rule)
            for (size_t i = 0; i + 1 < nodeX.size(); i += 2) {
                int leftX = std::max(0, nodeX[i]);
                int rightX = std::min(canvas.width - 1, nodeX[i + 1]);
                for (int x = leftX; x <= rightX; ++x) {
                    canvas.setPixel(x, y, color);
                }
            }
        }
    }
};

#endif // POLYGON_OPS_HPP
