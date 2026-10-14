#ifndef MINIMAP_HUD_HPP
#define MINIMAP_HUD_HPP

#include "SoftwareCanvas.hpp"
#include "LineDrawing.hpp"
#include "CircleDrawing.hpp"
#include "PolygonOps.hpp"
#include "Clipping.hpp"
#include "CurveDrawing.hpp"
#include "../level/HouseGrid.hpp"
#include <vector>
#include <cmath>

// Minimap HUD — renders the 2D tactical overlay entirely in software using all five
// pure C++ algorithms. Each frame the CPU writes into a pixel buffer which is then
// uploaded to a GL texture and drawn as a screen-space quad.
class MinimapHUD {
public:
    SoftwareCanvas canvas;

    MinimapHUD(int width = 200, int height = 200) : canvas(width, height) {}

    void render(float playerX, float playerZ, float yawDegrees, float timeSeconds,
                const cp1::Grid& grid, bool upload = false) {

        canvas.clear(rgba(20, 24, 29, 230));

        int mapRows = static_cast<int>(grid.size());
        int mapCols = static_cast<int>(grid[0].size());

        float tileW = static_cast<float>(canvas.width) / mapCols;
        float tileH = static_cast<float>(canvas.height) / mapRows;

        // 1) Scanline polygon fill — solid wall blocks, closets, and fuse objectives
        const uint32_t wallColor       = rgba(58, 71, 86);
        const uint32_t wallBorderColor = rgba(92, 110, 130);
        const uint32_t closetColor     = rgba(44, 92, 58);
        const uint32_t fuseColor       = rgba(245, 205, 60);

        for (int r = 0; r < mapRows; ++r) {
            for (int c = 0; c < mapCols; ++c) {
                char ch = cp1::cellAt(grid, c, r);
                float x0 = c * tileW;
                float y0 = r * tileH;

                if (ch == '1') {
                    std::vector<Point2D> wallVerts = {
                        { x0 + 1.0f,         y0 + 1.0f         },
                        { x0 + tileW - 1.0f, y0 + 1.0f         },
                        { x0 + tileW - 1.0f, y0 + tileH - 1.0f },
                        { x0 + 1.0f,         y0 + tileH - 1.0f }
                    };
                    Polygon2D wallPoly(wallVerts);
                    wallPoly.scanlineFill(canvas, wallColor);
                    wallPoly.drawOutline(canvas, wallBorderColor);
                } else if (ch == 'H') {
                    // Closet marker
                    std::vector<Point2D> closetVerts = {
                        { x0 + 2.0f,         y0 + 2.0f         },
                        { x0 + tileW - 2.0f, y0 + 2.0f         },
                        { x0 + tileW - 2.0f, y0 + tileH - 2.0f },
                        { x0 + 2.0f,         y0 + tileH - 2.0f }
                    };
                    Polygon2D closetPoly(closetVerts);
                    closetPoly.scanlineFill(canvas, closetColor);
                } else if (ch == 'F') {
                    // Fuse marker: scanline-filled diamond
                    float cx = x0 + tileW * 0.5f;
                    float cy = y0 + tileH * 0.5f;
                    float rad = std::min(tileW, tileH) * 0.35f;
                    std::vector<Point2D> diamond = {
                        { cx, cy - rad },
                        { cx + rad, cy },
                        { cx, cy + rad },
                        { cx - rad, cy }
                    };
                    Polygon2D fusePoly(diamond);
                    fusePoly.scanlineFill(canvas, fuseColor);
                }
            }
        }

        // 2) Bresenham lines — tactical grid
        const uint32_t gridColor = rgba(34, 43, 53);
        for (int c = 0; c <= mapCols; ++c) {
            int x = static_cast<int>(c * tileW);
            LineDrawing::drawLine(canvas, x, 0, x, canvas.height - 1, gridColor);
        }
        for (int r = 0; r <= mapRows; ++r) {
            int y = static_cast<int>(r * tileH);
            LineDrawing::drawLine(canvas, 0, y, canvas.width - 1, y, gridColor);
        }

        // Map centered world coords (col - cols/2, row - rows/2) to canvas coordinates
        float mapPlayerX = (playerX + mapCols * 0.5f) * tileW;
        float mapPlayerY = (playerZ + mapRows * 0.5f) * tileH;

        // 3) De Casteljau Bézier — entity search path / acoustic sensor curve
        Point2D p0 = { 20.0f,  20.0f  };
        Point2D p1 = { canvas.width - 20.0f, 40.0f  };
        Point2D p2 = { 20.0f,  canvas.height - 40.0f };
        Point2D p3 = { canvas.width - 20.0f, canvas.height - 20.0f };
        CurveDrawing::drawCubicBezier(canvas, p0, p1, p2, p3, 30, rgba(136, 51, 51));

        // Animated entity ping along the Bézier curve
        float curveT = std::fmod(timeSeconds * 0.2f, 1.0f);
        Point2D entityPos = CurveDrawing::deCasteljau(p0, p1, p2, p3, curveT);
        CircleDrawing::drawFilledCircle(canvas, static_cast<int>(entityPos.x), static_cast<int>(entityPos.y), 3, rgba(255, 51, 51));

        // 4) Cohen-Sutherland clipping — player sightline ray, clipped to HUD inner bounds
        float yawRad = yawDegrees * 3.14159265f / 180.0f;
        float rayLen = 35.0f;
        float targetX = mapPlayerX + std::cos(yawRad) * rayLen;
        float targetY = mapPlayerY + std::sin(yawRad) * rayLen;
        CohenSutherlandClipping::clipAndDrawLine(canvas,
            mapPlayerX, mapPlayerY, targetX, targetY,
            4.0f, 4.0f, canvas.width - 4.0f, canvas.height - 4.0f,
            rgba(0, 255, 204));

        // 5) Midpoint circle — player position indicator and pulsing radar echo
        int pulseRadius = static_cast<int>(std::fmod(timeSeconds * 20.0f, 25.0f)) + 4;
        CircleDrawing::drawCircle(canvas, static_cast<int>(mapPlayerX), static_cast<int>(mapPlayerY), pulseRadius, rgba(0, 255, 136, 120));
        CircleDrawing::drawFilledCircle(canvas, static_cast<int>(mapPlayerX), static_cast<int>(mapPlayerY), 3, rgba(0, 255, 136));

        // Outer HUD border
        Polygon2D borderPoly({
            { 0, 0 },
            { static_cast<float>(canvas.width - 1), 0 },
            { static_cast<float>(canvas.width - 1), static_cast<float>(canvas.height - 1) },
            { 0, static_cast<float>(canvas.height - 1) }
        });
        borderPoly.drawOutline(canvas, rgba(0, 255, 136));

        if (upload) {
            canvas.uploadToGPU();
        }
    }
};

#endif // MINIMAP_HUD_HPP
