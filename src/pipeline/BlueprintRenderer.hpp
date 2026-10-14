#ifndef BLUEPRINT_RENDERER_HPP
#define BLUEPRINT_RENDERER_HPP

#include "../algorithms/SoftwareCanvas.hpp"
#include "../algorithms/LineDrawing.hpp"
#include "../algorithms/PolygonOps.hpp"
#include "../algorithms/MatrixMath3D.hpp"
#include "../level/HouseGrid.hpp"
#include <iostream>
#include <vector>
#include <cmath>

namespace cp1 {

// Helper to construct axis-aligned rectangular polygon vertices
inline std::vector<Point2D> makeRect(float x, float y, float w, float h) {
    return {
        { x,     y     },
        { x + w, y     },
        { x + w, y + h },
        { x,     y + h }
    };
}

// 1) Top-down architectural floor plan renderer
// Uses Polygon2D::scanlineFill() for rooms/tiles and LineDrawing::drawLine() for grid lines
inline void renderFloorPlan(const Grid& g, SoftwareCanvas& canvas, int s) {
    const uint32_t floorA     = rgba(34, 34, 40);
    const uint32_t floorB     = rgba(40, 40, 47);
    const uint32_t gridLine   = rgba(58, 58, 68);
    const uint32_t wallFill   = rgba(112, 86, 62);
    const uint32_t wallEdge   = rgba(180, 150, 110);
    const uint32_t closetFill = rgba(44, 92, 58);
    const uint32_t closetEdge = rgba(110, 190, 130);
    const uint32_t fuseFill   = rgba(245, 205, 60);
    const uint32_t fuseEdge   = rgba(255, 245, 170);

    int rows = static_cast<int>(g.size());
    int cols = static_cast<int>(g[0].size());

    // Tile fills
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            float x = static_cast<float>(c * s);
            float y = static_cast<float>(r * s);
            char ch = cellAt(g, c, r);

            if (ch == '1') {
                Polygon2D wall(makeRect(x, y, static_cast<float>(s), static_cast<float>(s)));
                wall.scanlineFill(canvas, wallFill);
                continue;
            }

            Polygon2D floor(makeRect(x, y, static_cast<float>(s), static_cast<float>(s)));
            floor.scanlineFill(canvas, (c + r) % 2 ? floorB : floorA);

            if (ch == 'H') {
                // Closet: inset square + door indicator triangle
                Polygon2D closet(makeRect(x + 4, y + 4, static_cast<float>(s - 8), static_cast<float>(s - 8)));
                closet.scanlineFill(canvas, closetFill);
                closet.drawOutline(canvas, closetEdge);

                std::vector<Point2D> door = {
                    { x + s * 0.5f,        y + s - 10.0f },
                    { x + s * 0.5f - 6.0f, y + s - 4.0f  },
                    { x + s * 0.5f + 6.0f, y + s - 4.0f  }
                };
                Polygon2D doorPoly(door);
                doorPoly.scanlineFill(canvas, closetEdge);
            } else if (ch == 'F') {
                // Fuse: scanline-filled diamond polygon
                float cx = x + s * 0.5f;
                float cy = y + s * 0.5f;
                float rad = s * 0.28f;
                std::vector<Point2D> diamond = {
                    { cx, cy - rad },
                    { cx + rad, cy },
                    { cx, cy + rad },
                    { cx - rad, cy }
                };
                Polygon2D fusePoly(diamond);
                fusePoly.scanlineFill(canvas, fuseFill);
                fusePoly.drawOutline(canvas, fuseEdge);
            }
        }
    }

    // Grid lines via Bresenham algorithm
    for (int c = 0; c <= cols; ++c) {
        LineDrawing::drawLine(canvas, c * s, 0, c * s, rows * s - 1, gridLine);
    }
    for (int r = 0; r <= rows; ++r) {
        LineDrawing::drawLine(canvas, 0, r * s, cols * s - 1, r * s, gridLine);
    }

    // Wall borders on top of grid lines
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (cellAt(g, c, r) == '1') {
                Polygon2D wall(makeRect(static_cast<float>(c * s), static_cast<float>(r * s),
                                        static_cast<float>(s - 1), static_cast<float>(s - 1)));
                wall.drawOutline(canvas, wallEdge);
            }
        }
    }
}

// 2) 3D wireframe cube renderer: object -> clip (MVP) -> perspective divide -> viewport,
// rasterized with pure C++ Bresenham lines
inline void wireCube(SoftwareCanvas& canvas, const Mat4& mvp, uint32_t color) {
    static const Vec3 v[8] = {
        {-.5f,-.5f,-.5f}, {.5f,-.5f,-.5f}, {.5f,.5f,-.5f}, {-.5f,.5f,-.5f},
        {-.5f,-.5f, .5f}, {.5f,-.5f, .5f}, {.5f,.5f, .5f}, {-.5f,.5f, .5f}
    };
    static const int e[12][2] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    int sx[8], sy[8];
    bool ok[8];

    for (int i = 0; i < 8; ++i) {
        Vec4 q = mvp * Vec4{v[i].x, v[i].y, v[i].z, 1.0f};
        ok[i] = (q.w > 0.01f);
        if (!ok[i]) continue;
        sx[i] = static_cast<int>(std::lround((q.x / q.w * 0.5f + 0.5f) * canvas.width));
        sy[i] = static_cast<int>(std::lround((1.0f - (q.y / q.w * 0.5f + 0.5f)) * canvas.height));
    }

    for (const auto& ed : e) {
        if (ok[ed[0]] && ok[ed[1]]) {
            LineDrawing::drawLine(canvas, sx[ed[0]], sy[ed[0]], sx[ed[1]], sy[ed[1]], color);
        }
    }
}

inline Mat4 wallModel(int col, int row, float height = 1.0f) {
    return translate(col + 0.5f, height / 2.0f, row + 0.5f) * scale(1.0f, height, 1.0f);
}

// 3D wireframe scene renderer
inline void renderWallWireframe(const Grid& g, SoftwareCanvas& canvas) {
    int rows = static_cast<int>(g.size());
    int cols = static_cast<int>(g[0].size());

    Mat4 proj = perspective(45.0f * 3.14159265f / 180.0f,
                            static_cast<float>(canvas.width) / canvas.height, 0.1f, 100.0f);
    Vec3 ctr{cols / 2.0f, 0.0f, rows / 2.0f};
    Mat4 vp = proj * lookAt({ctr.x + 6.0f, 17.0f, ctr.z + 15.0f}, ctr, {0.0f, 1.0f, 0.0f});

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            char ch = cellAt(g, c, r);
            if (ch == '1') {
                wireCube(canvas, vp * wallModel(c, r), rgba(200, 170, 120));
            } else if (ch == 'F') {
                wireCube(canvas, vp * translate(c + 0.5f, 0.2f, r + 0.5f) * rotateY(0.785f) * scale(0.3f, 0.3f, 0.3f),
                         rgba(255, 220, 70));
            } else if (ch == 'H') {
                wireCube(canvas, vp * translate(c + 0.5f, 0.5f, r + 0.5f) * scale(0.8f, 1.0f, 0.8f),
                         rgba(90, 200, 120));
            }
        }
    }
}

// Verification entry point — executes offline CPU rendering and exports PNG blueprints
inline void runGeometryDemo() {
    Grid g = defaultHouse();
    const int cell = 48;

    SoftwareCanvas plan(static_cast<int>(g[0].size()) * cell, static_cast<int>(g.size()) * cell, rgba(0, 0, 0));
    renderFloorPlan(g, plan, cell);

    SoftwareCanvas wire(960, 640, rgba(10, 10, 14));
    renderWallWireframe(g, wire);

    bool ok = plan.savePNG("cp1_floorplan.png") && wire.savePNG("cp1_walls_wireframe.png");
    std::cout << (ok ? "[CP1 Blueprint Pipeline] Generated cp1_floorplan.png and cp1_walls_wireframe.png\n"
                     : "[CP1 Blueprint Pipeline] Failed to export PNG blueprints\n");
}

} // namespace cp1

#endif // BLUEPRINT_RENDERER_HPP
