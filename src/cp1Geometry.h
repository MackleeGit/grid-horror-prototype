// CP1 pure-C++ demo: NO OpenGL/GLFW/GLM used in here.
// Writes cp1_floorplan.png and cp1_walls_wireframe.png to the build directory.
//
// Contents:  1) Framebuffer + PNG writer   2) Bresenham line
//            3) Polygon outline + scanline fill   4) Mat4 math (translate/scale/lookAt/perspective)
//            5) Grid map parser   6) Floor plan + 3D wall wireframe renderers
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace cp1 {

//  1) FRAMEBUFFER 
struct Color { uint8_t r, g, b; };

struct Framebuffer {
    int w, h;
    std::vector<uint8_t> px; // RGB, row-major

    Framebuffer(int w_, int h_, Color bg) : w(w_), h(h_), px(size_t(w_) * h_ * 3) {
        for (size_t i = 0; i < px.size(); i += 3) { px[i] = bg.r; px[i + 1] = bg.g; px[i + 2] = bg.b; }
    }
    void set(int x, int y, Color c) {
        if (x < 0 || y < 0 || x >= w || y >= h) return;
        size_t i = (size_t(y) * w + x) * 3;
        px[i] = c.r; px[i + 1] = c.g; px[i + 2] = c.b;
    }
};

// minimal PNG writer (uncompressed deflate blocks), so we can screenshot without libraries 
inline uint32_t crc32(const std::vector<uint8_t>& d) {
    static std::array<uint32_t, 256> t = [] {
        std::array<uint32_t, 256> a{};
        for (uint32_t n = 0; n < 256; ++n) {
            uint32_t c = n;
            for (int k = 0; k < 8; ++k) c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            a[n] = c;
        }
        return a;
    }();
    uint32_t c = 0xFFFFFFFFu;
    for (uint8_t b : d) c = t[(c ^ b) & 0xFF] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}
inline void putU32(std::vector<uint8_t>& v, uint32_t x) {
    for (int s = 24; s >= 0; s -= 8) v.push_back(uint8_t(x >> s));
}
inline void chunk(std::ofstream& f, const char* type, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> td(type, type + 4), out;
    td.insert(td.end(), data.begin(), data.end());
    putU32(out, uint32_t(data.size()));
    out.insert(out.end(), td.begin(), td.end());
    putU32(out, crc32(td));
    f.write(reinterpret_cast<const char*>(out.data()), std::streamsize(out.size()));
}
inline bool savePNG(const Framebuffer& fb, const std::string& path) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    const uint8_t sig[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    f.write(reinterpret_cast<const char*>(sig), 8);
    std::vector<uint8_t> ihdr;
    putU32(ihdr, uint32_t(fb.w)); putU32(ihdr, uint32_t(fb.h));
    ihdr.insert(ihdr.end(), {8, 2, 0, 0, 0}); // 8-bit RGB
    chunk(f, "IHDR", ihdr);
    std::vector<uint8_t> raw;
    for (int y = 0; y < fb.h; ++y) {
        raw.push_back(0);
        raw.insert(raw.end(), fb.px.begin() + size_t(y) * fb.w * 3, fb.px.begin() + size_t(y + 1) * fb.w * 3);
    }
    std::vector<uint8_t> z = {0x78, 0x01};
    for (size_t pos = 0; pos < raw.size();) {
        size_t len = std::min<size_t>(65535, raw.size() - pos);
        z.push_back(pos + len == raw.size() ? 1 : 0);
        z.push_back(uint8_t(len & 0xFF)); z.push_back(uint8_t(len >> 8));
        z.push_back(uint8_t(~len & 0xFF)); z.push_back(uint8_t((~len >> 8) & 0xFF));
        z.insert(z.end(), raw.begin() + pos, raw.begin() + pos + len);
        pos += len;
    }
    uint32_t a = 1, b = 0;
    for (uint8_t v : raw) { a = (a + v) % 65521; b = (b + a) % 65521; }
    putU32(z, (b << 16) | a);
    chunk(f, "IDAT", z);
    chunk(f, "IEND", {});
    return bool(f);
}

// 2) BRESENHAM LINE 
// Integer-only. At each step, decide x-step, y-step or both from the error term.
inline void drawLine(Framebuffer& fb, int x0, int y0, int x1, int y1, Color c) {
    int dx = std::abs(x1 - x0), dy = -std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (true) {
        fb.set(x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// 3) POLYGONS
struct Pt { double x, y; };
using Polygon = std::vector<Pt>; // ordered vertices, implicitly closed

inline Polygon rect(double x, double y, double w, double h) {
    return {{x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}};
}
inline void polyOutline(Framebuffer& fb, const Polygon& p, Color c) {
    for (size_t i = 0; i < p.size(); ++i) {
        const Pt &a = p[i], &b = p[(i + 1) % p.size()];
        drawLine(fb, int(std::lround(a.x)), int(std::lround(a.y)), int(std::lround(b.x)), int(std::lround(b.y)), c);
    }
}
// Scanline fill, even-odd rule (works for convex and concave polygons):
// for each row, find edge crossings, sort by x, fill between pairs.
inline void polyFill(Framebuffer& fb, const Polygon& p, Color c) {
    if (p.size() < 3) return;
    double minY = p[0].y, maxY = p[0].y;
    for (const Pt& v : p) { minY = std::min(minY, v.y); maxY = std::max(maxY, v.y); }
    std::vector<double> xs;
    for (int y = int(std::floor(minY)); y <= int(std::ceil(maxY)); ++y) {
        double sy = y + 0.5; // sample at pixel centre
        xs.clear();
        for (size_t i = 0; i < p.size(); ++i) {
            const Pt &a = p[i], &b = p[(i + 1) % p.size()];
            if (a.y == b.y) continue;
            if (sy >= std::min(a.y, b.y) && sy < std::max(a.y, b.y))
                xs.push_back(a.x + (sy - a.y) * (b.x - a.x) / (b.y - a.y));
        }
        std::sort(xs.begin(), xs.end());
        for (size_t i = 0; i + 1 < xs.size(); i += 2)
            for (int x = int(std::ceil(xs[i] - 0.5)); x <= int(std::ceil(xs[i + 1] - 0.5)) - 1; ++x) fb.set(x, y, c);
    }
}

// 4) MATRIX MATH 
struct Vec3 { float x, y, z; };
struct Vec4 { float x, y, z, w; };
struct Mat4 {
    float m[4][4]{};
    static Mat4 identity() { Mat4 r; for (int i = 0; i < 4; ++i) r.m[i][i] = 1; return r; }
};
inline Mat4 operator*(const Mat4& a, const Mat4& b) {
    Mat4 r;
    for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) for (int k = 0; k < 4; ++k) r.m[i][j] += a.m[i][k] * b.m[k][j];
    return r;
}
inline Vec4 operator*(const Mat4& a, Vec4 v) {
    return {a.m[0][0]*v.x + a.m[0][1]*v.y + a.m[0][2]*v.z + a.m[0][3]*v.w,
            a.m[1][0]*v.x + a.m[1][1]*v.y + a.m[1][2]*v.z + a.m[1][3]*v.w,
            a.m[2][0]*v.x + a.m[2][1]*v.y + a.m[2][2]*v.z + a.m[2][3]*v.w,
            a.m[3][0]*v.x + a.m[3][1]*v.y + a.m[3][2]*v.z + a.m[3][3]*v.w};
}
inline Mat4 translate(float x, float y, float z) { Mat4 r = Mat4::identity(); r.m[0][3] = x; r.m[1][3] = y; r.m[2][3] = z; return r; }
inline Mat4 scale(float x, float y, float z) { Mat4 r = Mat4::identity(); r.m[0][0] = x; r.m[1][1] = y; r.m[2][2] = z; return r; }
inline Mat4 rotateY(float a) {
    Mat4 r = Mat4::identity(); float c = std::cos(a), s = std::sin(a);
    r.m[0][0] = c; r.m[0][2] = s; r.m[2][0] = -s; r.m[2][2] = c; return r;
}
inline float dot(Vec3 a, Vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline Vec3 cross(Vec3 a, Vec3 b) { return {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x}; }
inline Vec3 norm(Vec3 v) { float l = std::sqrt(dot(v, v)); return {v.x / l, v.y / l, v.z / l}; }
// Same maths as glm::lookAt
inline Mat4 lookAt(Vec3 eye, Vec3 ctr, Vec3 up) {
    Vec3 f = norm({ctr.x - eye.x, ctr.y - eye.y, ctr.z - eye.z});
    Vec3 s = norm(cross(f, up)), u = cross(s, f);
    Mat4 r = Mat4::identity();
    r.m[0][0] = s.x;  r.m[0][1] = s.y;  r.m[0][2] = s.z;  r.m[0][3] = -dot(s, eye);
    r.m[1][0] = u.x;  r.m[1][1] = u.y;  r.m[1][2] = u.z;  r.m[1][3] = -dot(u, eye);
    r.m[2][0] = -f.x; r.m[2][1] = -f.y; r.m[2][2] = -f.z; r.m[2][3] = dot(f, eye);
    return r;
}
// Same maths as glm::perspective
inline Mat4 perspective(float fovY, float aspect, float n, float f) {
    float t = 1.0f / std::tan(fovY / 2); Mat4 r;
    r.m[0][0] = t / aspect; r.m[1][1] = t;
    r.m[2][2] = (f + n) / (n - f); r.m[2][3] = 2 * f * n / (n - f); r.m[3][2] = -1;
    return r;
}

// 5) GRID MAP 
// '1' = wall, '0' = floor, 'H' = closet, 'F' = fuse
using Grid = std::vector<std::string>;

inline Grid defaultHouse() {
    return {
        "1111111111111111",
        "1000000100000001",
        "10F00000100H0001",
        "1000111100000001",
        "1000100000011101",
        "1H00100000010001",
        "1000100111010F01",
        "1000000100010001",
        "1011110100000001",
        "1000000000111101",
        "1F0000100000H001",
        "1111111111111111",
    };
}
inline char cellAt(const Grid& g, int col, int row) {
    if (row < 0 || row >= int(g.size()) || col < 0 || col >= int(g[row].size())) return '1'; // outside = wall
    return g[row][col];
}
// Grid -> 3D: column -> world X, row -> world Z, 1 cell = 1 unit.
// Model = Translate(cell centre) * Scale(wall height). One matrix per wall.
inline Mat4 wallModel(int col, int row, float height = 1.0f) {
    return translate(col + 0.5f, height / 2, row + 0.5f) * scale(1, height, 1);
}

// 6) RENDERERS 
// Top-down floor plan: filled polygons (scanline) + Bresenham grid lines.
inline void renderFloorPlan(const Grid& g, Framebuffer& fb, int s) {
    const Color floorA{34, 34, 40}, floorB{40, 40, 47}, gridLine{58, 58, 68};
    const Color wallFill{112, 86, 62}, wallEdge{180, 150, 110};
    const Color closetFill{44, 92, 58}, closetEdge{110, 190, 130};
    const Color fuseFill{245, 205, 60}, fuseEdge{255, 245, 170};
    int rows = int(g.size()), cols = int(g[0].size());

    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            double x = c * s, y = r * s;
            char ch = cellAt(g, c, r);
            if (ch == '1') { polyFill(fb, rect(x, y, s, s), wallFill); continue; }
            polyFill(fb, rect(x, y, s, s), (c + r) % 2 ? floorB : floorA);
            if (ch == 'H') { // closet: inset square + door triangle
                polyFill(fb, rect(x + 4, y + 4, s - 8, s - 8), closetFill);
                polyOutline(fb, rect(x + 4, y + 4, s - 9, s - 9), closetEdge);
                polyFill(fb, {{x + s / 2.0, y + s - 10.0}, {x + s / 2.0 - 6, y + s - 4.0}, {x + s / 2.0 + 6, y + s - 4.0}}, closetEdge);
            } else if (ch == 'F') { // fuse: diamond
                double cx = x + s / 2.0, cy = y + s / 2.0, rad = s * 0.28;
                Polygon d = {{cx, cy - rad}, {cx + rad, cy}, {cx, cy + rad}, {cx - rad, cy}};
                polyFill(fb, d, fuseFill);
                polyOutline(fb, d, fuseEdge);
            }
        }
    for (int c = 0; c <= cols; ++c) drawLine(fb, c * s, 0, c * s, rows * s - 1, gridLine);
    for (int r = 0; r <= rows; ++r) drawLine(fb, 0, r * s, cols * s - 1, r * s, gridLine);
    for (int r = 0; r < rows; ++r) // wall outlines on top of the grid lines
        for (int c = 0; c < cols; ++c)
            if (cellAt(g, c, r) == '1') polyOutline(fb, rect(c * s, r * s, s - 1, s - 1), wallEdge);
}

// Wireframe cube: object -> clip (MVP) -> perspective divide -> viewport, then Bresenham on 12 edges.
inline void wireCube(Framebuffer& fb, const Mat4& mvp, Color col) {
    static const Vec3 v[8] = {{-.5f,-.5f,-.5f},{.5f,-.5f,-.5f},{.5f,.5f,-.5f},{-.5f,.5f,-.5f},
                              {-.5f,-.5f,.5f},{.5f,-.5f,.5f},{.5f,.5f,.5f},{-.5f,.5f,.5f}};
    static const int e[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
    int sx[8], sy[8]; bool ok[8];
    for (int i = 0; i < 8; ++i) {
        Vec4 q = mvp * Vec4{v[i].x, v[i].y, v[i].z, 1};
        ok[i] = q.w > 0.01f; // skip points behind the camera
        if (!ok[i]) continue;
        sx[i] = int(std::lround((q.x / q.w * 0.5f + 0.5f) * fb.w));
        sy[i] = int(std::lround((1 - (q.y / q.w * 0.5f + 0.5f)) * fb.h));
    }
    for (auto& ed : e) if (ok[ed[0]] && ok[ed[1]]) drawLine(fb, sx[ed[0]], sy[ed[0]], sx[ed[1]], sy[ed[1]], col);
}

inline void renderWallWireframe(const Grid& g, Framebuffer& fb) {
    int rows = int(g.size()), cols = int(g[0].size());
    Mat4 proj = perspective(45.0f * 3.14159265f / 180.0f, float(fb.w) / fb.h, 0.1f, 100.0f);
    Vec3 ctr{cols / 2.0f, 0, rows / 2.0f};
    Mat4 vp = proj * lookAt({ctr.x + 6, 17, ctr.z + 15}, ctr, {0, 1, 0});

    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            char ch = cellAt(g, c, r);
            if (ch == '1') wireCube(fb, vp * wallModel(c, r), {200, 170, 120});
            else if (ch == 'F') wireCube(fb, vp * translate(c + .5f, .2f, r + .5f) * rotateY(0.785f) * scale(.3f, .3f, .3f), {255, 220, 70});
            else if (ch == 'H') wireCube(fb, vp * translate(c + .5f, .5f, r + .5f) * scale(.8f, 1, .8f), {90, 200, 120});
        }
}

// ENTRY POINT 
inline void runGeometryDemo() {
    Grid g = defaultHouse();
    const int cell = 48;
    Framebuffer plan(int(g[0].size()) * cell, int(g.size()) * cell, {0, 0, 0});
    renderFloorPlan(g, plan, cell);
    Framebuffer wire(960, 640, {10, 10, 14});
    renderWallWireframe(g, wire);
    bool ok = savePNG(plan, "cp1_floorplan.png") && savePNG(wire, "cp1_walls_wireframe.png");
    std::cout << (ok ? "[CP1] Wrote cp1_floorplan.png and cp1_walls_wireframe.png\n"
                     : "[CP1] Failed to write PNG files\n");
}

} 