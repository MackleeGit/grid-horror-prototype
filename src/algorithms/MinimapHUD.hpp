#ifndef MINIMAP_HUD_HPP
#define MINIMAP_HUD_HPP

#include "SoftwareCanvas.hpp"
#include "LineDrawing.hpp"
#include "CircleDrawing.hpp"
#include "PolygonOps.hpp"
#include "Clipping.hpp"
#include "CurveDrawing.hpp"
#include <vector>
#include <cmath>

/*
================================================================================
MINIMAP HUD SYSTEM — INTEGRATING ALL 5 CORE GRAPHICS ALGORITHMS
================================================================================
This class orchestrates all five pure C++ computer graphics algorithms to render
a real-time 2D tactical HUD minimap in software memory:

1. Scanline Polygon Fill & 2D Matrix Transforms: Renders solid wall blocks 
   transformed from grid map space to software HUD coordinates.
2. Cohen-Sutherland Line Clipping: Clips the player's 3D forward sight vector 
   and radar sweep lines strictly within the minimap viewport bounds.
3. Bresenham's Line Drawing: Draws grid lines, sight lines, and HUD borders.
4. Midpoint Circle Algorithm: Renders the player position indicator and pulsing 
   sonar/radar rings.
5. De Casteljau's Bézier Curves: Evaluates and renders the smooth curved patrol 
   path for enemy entity movement.
================================================================================
*/

class MinimapHUD {
public:
    SoftwareCanvas canvas;

    MinimapHUD(int width = 200, int height = 200) : canvas(width, height) {}

    void render(float playerX, float playerZ, float yawDegrees, float timeSeconds,
                const int map[10][10], int mapWidth, int mapHeight) {
        // 1. Clear background (Dark Slate color: 0x14181DFF)
        canvas.clear(0x14181DFF);

        float tileW = static_cast<float>(canvas.width) / mapWidth;
        float tileH = static_cast<float>(canvas.height) / mapHeight;

        // 2. POLYGON REPRESENTATION, MATRIX TRANSFORMS, & SCANLINE FILL: Wall Blocks
        uint32_t wallColor = 0x3A4756FF;
        uint32_t wallBorderColor = 0x5C6E82FF;

        for (int r = 0; r < mapHeight; ++r) {
            for (int c = 0; c < mapWidth; ++c) {
                if (map[r][c] == 1) {
                    // Define wall cube as a 2D polygon (4 vertices)
                    std::vector<Point2D> wallVerts = {
                        { c * tileW + 1.0f, r * tileH + 1.0f },
                        { (c + 1) * tileW - 1.0f, r * tileH + 1.0f },
                        { (c + 1) * tileW - 1.0f, (r + 1) * tileH - 1.0f },
                        { c * tileW + 1.0f, (r + 1) * tileH - 1.0f }
                    };
                    Polygon2D wallPoly(wallVerts);
                    
                    // Fill using Pure C++ Scanline Algorithm
                    wallPoly.scanlineFill(canvas, wallColor);
                    // Outline using Pure C++ Bresenham Line Algorithm
                    wallPoly.drawOutline(canvas, wallBorderColor);
                }
            }
        }

        // 3. BRESENHAM'S LINE DRAWING: Grid Lines
        uint32_t gridColor = 0x222B35FF;
        for (int c = 0; c <= mapWidth; ++c) {
            int x = static_cast<int>(c * tileW);
            LineDrawing::drawLine(canvas, x, 0, x, canvas.height - 1, gridColor);
        }
        for (int r = 0; r <= mapHeight; ++r) {
            int y = static_cast<int>(r * tileH);
            LineDrawing::drawLine(canvas, 0, y, canvas.width - 1, y, gridColor);
        }

        // Map world coordinates (-5..5) to canvas coordinates (0..canvas.width)
        float mapPlayerX = (playerX + 5.0f) * (canvas.width / 10.0f);
        float mapPlayerY = (playerZ + 5.0f) * (canvas.height / 10.0f);

        // 4. DE CASTELJAU BÉZIER CURVES: Entity AI Patrol Spline
        Point2D p0 = { 20.0f, 20.0f };
        Point2D p1 = { 180.0f, 40.0f };
        Point2D p2 = { 20.0f, 160.0f };
        Point2D p3 = { 180.0f, 180.0f };
        CurveDrawing::drawCubicBezier(canvas, p0, p1, p2, p3, 30, 0x883333FF);

        // Calculate animated position along Bézier curve for entity dot
        float curveT = std::fmod(timeSeconds * 0.2f, 1.0f);
        Point2D entityPos = CurveDrawing::deCasteljau(p0, p1, p2, p3, curveT);

        // MIDPOINT CIRCLE: Render Entity indicator dot & pulse
        CircleDrawing::drawFilledCircle(canvas, static_cast<int>(entityPos.x), static_cast<int>(entityPos.y), 4, 0xFF3333FF);

        // 5. COHEN-SUTHERLAND LINE CLIPPING: Player Forward Direction Raycast Line
        float yawRad = yawDegrees * 3.14159265f / 180.0f;
        float rayLen = 40.0f;
        float targetX = mapPlayerX + std::cos(yawRad) * rayLen;
        float targetY = mapPlayerY + std::sin(yawRad) * rayLen;

        // Clip sight ray strictly within HUD interior window (10, 10) to (width-10, height-10)
        CohenSutherlandClipping::clipAndDrawLine(canvas, 
                                                 mapPlayerX, mapPlayerY, targetX, targetY,
                                                 5.0f, 5.0f, canvas.width - 5.0f, canvas.height - 5.0f,
                                                 0x00FFCCFF);

        // 6. MIDPOINT CIRCLE ALGORITHM: Player Position Indicator & Sonar Radar Rings
        // Pulsing radar ring
        int pulseRadius = static_cast<int>(std::fmod(timeSeconds * 20.0f, 30.0f)) + 5;
        CircleDrawing::drawCircle(canvas, static_cast<int>(mapPlayerX), static_cast<int>(mapPlayerY), pulseRadius, 0x00FF8844);
        
        // Solid Player Dot
        CircleDrawing::drawFilledCircle(canvas, static_cast<int>(mapPlayerX), static_cast<int>(mapPlayerY), 4, 0x00FF88FF);

        // HUD Outer Border
        Polygon2D borderPoly({
            { 0, 0 }, { static_cast<float>(canvas.width - 1), 0 },
            { static_cast<float>(canvas.width - 1), static_cast<float>(canvas.height - 1) },
            { 0, static_cast<float>(canvas.height - 1) }
        });
        borderPoly.drawOutline(canvas, 0x00FF88FF);

        // Upload CPU pixel array to GPU texture
        canvas.uploadToGPU();
    }
};

#endif // MINIMAP_HUD_HPP
