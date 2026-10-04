# Echoes in the Dark

A 3D First-Person Survival Horror Game built in C++17, featuring a custom **Pure C++ Software Rasterization Engine** and an **OpenGL 3.3 Core Profile Graphics Pipeline**.

---

## 1. Project Description

**Echoes in the Dark** is a real-time computer graphics application designed to explore both software-level computer graphics algorithms and hardware-accelerated 3D graphics rendering. 

Set inside a claustrophobic, dark maze, the player navigates a 3D environment in first-person view while monitoring an active tactical HUD minimap. The tactical HUD is rendered entirely by a **pure C++ software engine** calculating pixels into a raw memory buffer, demonstrating core computer graphics algorithms (line drawing, polygon filling, clipping, circles, and Bézier curves) without relying on external graphics API function calls for underlying work.

---

## 2. Group Members & Responsibilities

| Group Member | Primary Responsibilities | Key Contributions |
| :--- | :--- | :--- |
| **Lead Developer** | Architecture, Software Rasterizer, 3D Engine | Implementation of Pure C++ Graphics Algorithms (`src/algorithms/`), OpenGL context setup, Camera math, Build system configuration |
| **Graphics & Technical Writing** | Documentation & Design Verification | Code justification comments, README documentation, testing & progress tracking |

---

## 3. Technologies & Tools Used

- **Programming Language:** C++17 (Strict standard compliance)
- **Build System:** CMake (3.15+) with MinGW-w64 GCC / Ninja
- **Windowing & Input:** GLFW 3 (OS window creation, mouse/keyboard hardware callbacks)
- **OpenGL Extension Loader:** GLAD (Runtime GPU function pointer loader)
- **Mathematics Library:** GLM (OpenGL Mathematics — vectors, matrices, trigonometric transformations)
- **Asset Loading:** `stb_image` (Texture loading and mipmap preparation)
- **Version Control:** Git & GitHub

---

## 4. Graphics Techniques & Algorithms Implemented

This project intentionally combines **Pure C++ Software Rasterization** with **OpenGL Hardware Acceleration**.

### A. Pure C++ Computer Graphics Algorithms (`src/algorithms/`)

These algorithms run strictly on the CPU, directly manipulating a 3D/2D pixel array (`std::vector<uint32_t>`):

1. **Bresenham’s Line Algorithm (`LineDrawing.hpp`)**
   - *Concept:* Incremental integer rasterization algorithm using error decision variables ($p$) to draw discrete 2D lines.
   - *Application:* Renders grid outlines, boundary frames, and sight direction vectors on the tactical minimap.
2. **Midpoint Circle Algorithm (`CircleDrawing.hpp`)**
   - *Concept:* Calculates 8-way symmetric arc points using integer decision parameters ($P = 1 - r$).
   - *Application:* Renders player position indicators, filled entity markers, and expanding sonar radar pulse rings.
3. **Polygon Representation, Matrix Transformations & Scanline Fill (`PolygonOps.hpp`)**
   - *Concept:* Homogeneous 2D affine transformations (translation, rotation, scaling) and scanline parity filling between edge intersections.
   - *Application:* Represents 2D wall obstacles and fills solid room structures in software memory.
4. **Cohen-Sutherland Line Clipping Algorithm (`Clipping.hpp`)**
   - *Concept:* 4-bit region outcodes (INSIDE, LEFT, RIGHT, BOTTOM, TOP) for $O(1)$ line acceptance/rejection and edge intersection clipping.
   - *Application:* Clips player sightlines and radar vectors within HUD viewport boundaries.
5. **De Casteljau’s Cubic Bézier Curves (`CurveDrawing.hpp`)**
   - *Concept:* Recursive linear interpolation (`lerp`) across 4 control points ($P_0, P_1, P_2, P_3$) evaluating parametric spline points at $t \in [0, 1]$.
   - *Application:* Evaluates smooth curved AI patrol routes for entity pathing.

---

### B. Hardware-Accelerated 3D Pipeline (`src/main.cpp`)

1. **FPS Trigonometric Camera Math:** Converts mouse pitch and yaw Euler angles to 3D Cartesian direction vectors:
   $$\begin{aligned}
   x &= \cos(\text{yaw}) \cdot \cos(\text{pitch}) \\
   y &= \sin(\text{pitch}) \\
   z &= \sin(\text{yaw}) \cdot \cos(\text{pitch})
   \end{aligned}$$
2. **Model-View-Projection (MVP) Pipeline:** Computes per-frame 4x4 matrix chains:
   $$\text{gl\_Position} = \text{Projection} \times \text{View} \times \text{Model} \times \vec{v}$$
3. **Buffer Management (VBO, VAO, EBO):** Configures interleaved vertex position and UV coordinate buffers.
4. **Texture Mapping & Mipmapping:** Samples 2D textures using normalized UV coordinates.

---

## 5. How to Compile and Run

### Prerequisites
- GCC / MinGW-w64 (supporting C++17)
- CMake 3.15+
- Ninja (or Make)

### Build Instructions

```powershell
# 1. Generate build files
cmake -B build -G Ninja

# 2. Compile executable
cmake --build build

# 3. Launch application
.\build\EchoesInTheDark.exe
```

---

## 6. Controls & Controls Shortcuts

- **W / A / S / D:** Translate camera forward, left, backward, right
- **Mouse:** Rotate camera (FPS Look)
- **F2:** Toggle Software Minimap HUD Overlay ON/OFF
- **F3:** Toggle Real-time Telemetry (Coordinates, Yaw, Pitch, FPS)
- **ESC:** Exit application

---

## 7. Current Progress Summary

- ✅ **Phase 1 (Foundation):** FPS Camera, GLFW window context, Delta Time, input handling, and F3 telemetry.
- ✅ **Phase 2 (Level Geometry):** 3D cube wall instancing, floor quads, EBO indexing, texture mapping.
- ✅ **Pure C++ Algorithms Module:** Software canvas, Bresenham lines, Midpoint circles, Scanline polygon fill, Cohen-Sutherland clipping, and De Casteljau Bézier curves.
- ✅ **Tactical Minimap HUD:** Software-rasterized 2D minimap integrated directly into the application loop.

---

## 8. Known Limitations & Issues

- Wall collision (AABB) is scheduled for Phase 4 (Game Logic & Collisions).
- Flashlight spotlight GLSL shaders scheduled for Phase 3 (Shaders & Lighting).
