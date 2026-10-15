# 3D SURVIVAL HORROR APPLICATION
*(Working Title: Echoes in the Dark)*

A real-time 3D First-Person Survival Horror Game built in standard C++17, featuring an offline **Pure C++ Software Rasterization Engine** and a hardware-accelerated **OpenGL 3.3 Core Profile Graphics Pipeline**.

> [!IMPORTANT]
> **Checkpoint Evaluation Navigation:**
> For Checkpoint 1, please refer directly to the dedicated technical guide:
> **[`checkpoints/checkpoint1/CHECKPOINT1.md`](checkpoints/checkpoint1/CHECKPOINT1.md)**
> All pure C++ algorithmic output images are located in:
> **[`checkpoints/checkpoint1/cp1_output/`](checkpoints/checkpoint1/cp1_output/)**

---

## 1. Project Description

**3D SURVIVAL HORROR APPLICATION** is a real-time computer graphics project designed to explore foundational computer graphics mathematics, CPU software rasterization algorithms, and modern GPU rendering pipelines.

Set within an oppressive, pitch-black maze, the player navigates an enclosed 3D environment from a first-person perspective while avoiding an emergent stalker entity. The project intentionally decouples theoretical computer graphics algorithms from hardware APIs:
1. **Pure C++ Software Graphics Engine (`src/algorithms/`):** Directly manipulates raw CPU pixel memory buffers (`SoftwareCanvas`) using textbook algorithms (Bresenham’s lines, Scanline polygon filling, 2D affine transforms, Cohen-Sutherland clipping, Midpoint circles, Midpoint ellipses, and De Casteljau Bézier curves) without calling external graphics libraries.
2. **Offline Blueprint & Demonstration Pipeline (`checkpoints/checkpoint1/pipeline/BlueprintRenderer.hpp`):** Executes on startup to rasterize the architectural floor plan and 3D wireframe directly into timestamped PNG files.
3. **Hardware-Accelerated 3D Engine (`src/main.cpp`):** Renders the textured first-person 3D world with OpenGL 3.3 Core Profile while presenting the software minimap HUD as an integrated screen-space overlay.

---

## 2. Group Members, Foundational Duties & Work Allocation

### Context on Initial Commit Distribution
The project's initial commit history reflects a deliberate architectural approach: **Macklee Nderitu Gitonga (168000)** undertook the foundational systems architecture, CMake/Ninja build configuration, OpenGL context bootstrapping, and the initial pure C++ software rasterization buffer (`SoftwareCanvas`). This early stage work was necessary to establish a stable, compile-ready platform for the work to come. With the core foundation secured, subsequent and upcoming milestone tasks are distributed across all group members.

### Member Roles & Responsibility Matrix

| Group Member & Student ID | Core Area | Completed Contributions (Checkpoint 1) | Allocated Upcoming Responsibilities (Phases 3–5) |
| :--- | :--- | :--- | :--- |
| **Macklee Nderitu Gitonga**<br>`168000` | **System Architecture & Engine Core (Lead)** | • Configured CMake (3.15+) and Ninja build systems.<br>• Implemented CPU software canvas buffer (`SoftwareCanvas.hpp`) and zero-dependency PNG export.<br>• Implemented Bresenham integer line rasterizer (`LineDrawing.hpp`).<br>• Integrated GLFW/GLAD runtime and hybrid OpenGL/CPU main loop. | • System integration & cross-platform performance profiling.<br>• Phase 5 milestone coordination & code review. |
| **Amy Asembo Ojuka**<br>`162396` | **Level Geometry & Offline Blueprint Pipeline** | • Authored symbolic level grid parser (`HouseGrid.hpp`) mapping wall, closet, and fuse cells.<br>• Built offline CPU Blueprint Pipeline (`checkpoints/checkpoint1/pipeline/BlueprintRenderer.hpp`).<br>• Implemented pure C++ 3D wireframe scene renderer with software MVP matrix transforms (`MatrixMath3D.hpp`).<br>• Automated PNG verification drops for Checkpoint 1 marking. | • Expanding 3D maze geometry with multiple room layouts.<br>• 3D prop geometry instancing (fuse boxes, closet doors). |
| **Nicole Kadesa Agufana**<br>`169786` | **Interaction Systems & Mathematical Extensions** | • Implemented FPS camera spherical trigonometry (Yaw/Pitch to 3D Cartesian front vector).<br>• Configured WASD keyboard motion with vertical eye-height locking.<br>• Added pitch angle clamping ($[-89^\circ, 89^\circ]$) to eliminate gimbal flips.<br>• Built F3 real-time telemetry display (coordinates, angles, FPS). | • **Interaction System:** `E` key raycast/proximity checks for closet hiding and fuse collection.<br>• **Acoustic Math:** Midpoint Ellipse algorithm implementation for directional player noise propagation. |
| **Bilgis Nthoki Nzembi**<br>`169393` | **Shaders & Flashlight Lighting (Phase 3)** | • Extracted modular GLSL shader architecture (`shaders/scene.vert`, `shaders/scene.frag`, `shaders/hud.vert`, `shaders/hud.frag`).<br>• Authored automated C++ `Shader` loader class (`src/Shader.hpp`) with compilation/linking diagnostics.<br>• Configured screen-space HUD quad shaders. | • **Flashlight Cone GLSL Math:** Inner `cutOff` and outer `outerCutOff` spotlight attenuation in fragment shader.<br>• **Dynamic Lighting:** Ambient floor clamping (0.05) and uniform float `flickerFactor` modulation. |
| **Nancy Nduta Njoroge**<br>`166993` | **Collision Engine & Objectives (Phase 4)** | • Verified pure C++ polygon affine transformations (`PolygonOps.hpp` translation, rotation, scaling).<br>• Tested Cohen-Sutherland line clipping edge cases against rectangular viewports.<br>• Prepared AABB bounding box collision specifications. | • **AABB Collision Engine:** 2D/3D Axis-Aligned Bounding Box checks against wall grid in $(X, Z)$ space to eliminate wall-clipping.<br>• **Objectives:** Fuse inventory manager (3/3 required) and Exit-door victory unlock trigger. |
| **Phoebe Saniam Mokoit**<br>`168322` | **Entity AI & Game State Loop (Phase 5)** | • Midpoint Circle algorithm verification (8-way symmetry outline and scanline fill in `CircleDrawing.hpp`).<br>• Collation of Checkpoint 1 evidence outputs and technical documentation formatting.<br>• Designed entity pursuit and proximity state machine architecture. | • **Monster Entity AI:** Direction vector calculations and corridor pursuit steering.<br>• **Game State Machine:** State transitions (`PLAYING`, `HIDING`, `CAUGHT`, `ESCAPED`), 1.5-unit proximity death check, and Win/Loss HUD screens. |

---

## 3. Technologies & Tools Used

- **Programming Language:** C++17 (Strict ISO standard compliance)
- **Build System:** CMake (3.15+) with MinGW-w64 GCC and Ninja build generator
- **Windowing & Input:** GLFW 3.3+ (Window creation, mouse trapping, raw keyboard polling)
- **OpenGL Extension Loader:** GLAD (Runtime GPU function pointer dispatch)
- **Mathematics Library:** GLM (OpenGL Mathematics — 3D vectors, matrices, projections)
- **Texture Loading:** `stb_image` (Header-only texture parser)
- **Version Control:** Git & GitHub

---

## 4. Graphics Techniques & Algorithms Implemented (Checkpoint 1)

This project strictly adheres to the pure C++ requirement for Checkpoint 1. All algorithms in `src/algorithms/` run on the CPU and set raw pixel data without any graphics API calls:

```
               [ src/algorithms/ ]
                        |
        +---------------+---------------+
        |                               |
[ Offline Blueprint Renderer ]   [ Tactical Minimap HUD ]
 (Amy: cp1_output/ PNGs)          (Real-time in-game overlay)
```

### Pure C++ Algorithm Catalog

1. **Bresenham’s Line Algorithm (`src/algorithms/LineDrawing.hpp`)**
   - *Mathematical Formulation:* Incremental integer decision variable $p_k = 2\Delta y - \Delta x$ with zero floating-point division.
   - *Application:* Renders architectural coordinate grids (`cp1_floorplan.png`), all 12 wireframe edges of 3D cubes (`cp1_walls_wireframe.png`), and boundary frames.
2. **2D Polygon Representation & Affine Transformations (`src/algorithms/PolygonOps.hpp`)**
   - *Mathematical Formulation:* Homogeneous 2D matrix transformations (translation by $(\Delta x, \Delta y)$, rotation by $\theta$, and scaling by $(s_x, s_y)$).
   - *Application:* Constructs wall boundary polygons, triangular closet indicators, and diamond-shaped objective items.
3. **Scanline Polygon Fill (`src/algorithms/PolygonOps.hpp`)**
   - *Mathematical Formulation:* Edge Table (ET) intersection sorting and parity (even-odd) filling across active horizontal scanlines.
   - *Application:* Fills solid wall blocks and checkered room floor tiles in CPU memory.
4. **Cohen-Sutherland Line Clipping (`src/algorithms/Clipping.hpp`)**
   - *Mathematical Formulation:* 4-bit region outcodes ($\text{TOP}=1000_2, \text{BOTTOM}=0100_2, \text{RIGHT}=0010_2, \text{LEFT}=0001_2$) enabling $O(1)$ trivial acceptance/rejection and parametric edge boundary clipping.
   - *Application:* Clips player forward sightlines and radar vectors to HUD viewport boundaries.
5. **Midpoint Circle Algorithm (`src/algorithms/CircleDrawing.hpp`)**
   - *Mathematical Formulation:* Integer decision parameter $p = 1 - r$ evaluating octant arcs with 8-way symmetry plotting.
   - *Application:* Renders player position blip and expanding sonar radar pulse echoes.
6. **Midpoint Ellipse Algorithm (`src/algorithms/EllipseDrawing.hpp`)**
   - *Mathematical Formulation:* Region 1 ($2r_y^2 x \le 2r_x^2 y$) and Region 2 decision parameters with 4-way symmetry and horizontal scanline fill.
   - *Application:* Models directional player acoustic sound footprints and projected flashlight ground footprints.
7. **De Casteljau’s Cubic Bézier Curves (`src/algorithms/CurveDrawing.hpp`)**
   - *Mathematical Formulation:* Recursive linear interpolation (`lerp`) across 4 control points ($P_0, P_1, P_2, P_3$) at parametric step $t \in [0, 1]$.
   - *Application:* Evaluates smooth patrol splines for upcoming enemy wandering logic.

---

## 5. Demonstration & Evidence Files

When the executable is launched, Amy's offline geometry pipeline executes before opening the window, saving verified PNG blueprints directly into [`checkpoints/checkpoint1/cp1_output/`](checkpoints/checkpoint1/cp1_output/):

- **`cp1_floorplan.png`:** Full 2D top-down architectural layout of the horror maze rendered via Scanline Polygon Fill and Bresenham grid lines.
- **`cp1_walls_wireframe.png`:** Complete 3D perspective wireframe scene computed via pure C++ 4x4 MVP matrix mathematics and Bresenham rasterization.
- **`cp1_minimap_hud.png`:** Static snapshot of the tactical HUD minimap combining Bresenham lines, Scanline polygons, Cohen-Sutherland clipping, and Midpoint circles.
- **`cp1_gameplay_3d_hud.png`:** Real-time 3D OpenGL viewport running concurrently with the pure C++ software minimap HUD.

---

## 6. How to Compile and Run

### Prerequisites
- GCC / MinGW-w64 (C++17 support)
- CMake 3.15+
- Ninja (recommended) or Make

### Build & Execution Steps

```powershell
# 1. Generate Ninja build configuration
cmake -B build -G Ninja

# 2. Compile executable
cmake --build build

# 3. Launch application (generates CP1 output images automatically on startup)
.\build\EchoesInTheDark.exe
```

### In-Game Controls
- **W / A / S / D:** Translate camera forward, left, backward, right
- **Mouse:** 3D first-person camera look (spherical trigonometry)
- **F2:** Toggle Pure C++ Software Minimap HUD overlay ON/OFF
- **F3:** Toggle Real-time Telemetry (Coordinates, Yaw, Pitch, FPS)
- **ESC:** Exit application cleanly

---

## 7. Current Progress Summary

-  **(Foundation):** FPS Camera, GLFW window context, Delta Time, WASD movement, mouse-look, and F3 telemetry.
-  **(Geometry & Shaders):** 3D cube wall instancing, floor quads, procedural brick/stone textures, modular GLSL shaders.
-  **Pure C++ Engine:** Full algorithm suite in `src/algorithms/` (Bresenham lines, Scanline fill, Polygon transforms, Cohen-Sutherland clipping, Midpoint circles, Midpoint ellipses, Bézier curves).
-  **Offline Demonstration Pipeline:** Amy's geometry pipeline automatically exports PNG blueprints to `checkpoints/checkpoint1/cp1_output/`.

---

## 8. Known Limitations & Roadmap

- **Phase 3 (Shaders & Lighting - Bilgis):** Spotlight flashlight cone math (`cutOff`, `outerCutOff`), ambient 0.05 illumination, and uniform `flickerFactor` modulation.
- **Phase 4 (Collision & Interactions - Nancy & Nicole):** AABB wall collision detection in $(X, Z)$ space, closet hiding volumes, fuse pickup items with `E` key interaction.
- **Phase 5 (AI & Game State - Phoebe & Macklee):** Monster pursuit AI, 1.5-unit kill condition, win/loss triggers, and victory exit door.
