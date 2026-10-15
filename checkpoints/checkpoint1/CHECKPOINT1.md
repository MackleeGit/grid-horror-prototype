# 3D SURVIVAL HORROR APPLICATION
## Checkpoint 1 – Technical Foundation: Pure C++ Graphics Pipeline

**Submission Package:** Checkpoint 1 Evaluation Guide  
**Project Title:** 3D SURVIVAL HORROR APPLICATION *(Working Title: Echoes in the Dark)*  
**Repository Path:** `checkpoints/checkpoint1/`  

---

## 1. Purpose & Checkpoint 1 Fulfillment Overview

The purpose of Checkpoint 1 is to demonstrate meaningful technical progress and establish a working foundation in **Pure C++ Computer Graphics Algorithms**. 

In this project, all fundamental graphics algorithms run strictly on the **CPU** without calling external rendering libraries. Algorithms operate on raw CPU pixel arrays (`std::vector<uint32_t>`) encapsulated within `src/algorithms/SoftwareCanvas.hpp`.

To evaluate these algorithms objectively without forcing the evaluator to navigate an interactive 3D scene, **Amy Asembo Ojuka** developed an **offline CPU blueprint generation pipeline** (`checkpoints/checkpoint1/pipeline/BlueprintRenderer.hpp`). Upon program execution, this pipeline executes pure C++ algorithms to render architectural floor plans, 3D wireframe projections, and HUD overlays, saving the PNGs directly into `checkpoints/checkpoint1/cp1_output/`.

---

## 2. Directory Architecture & Algorithm Storage

The project strictly decouples pure algorithmic mathematics from hardware-accelerated presentation:

```text
horror-project/
├── checkpoints/
│   └── checkpoint1/
│       ├── CHECKPOINT1.md            <-- This evaluation guide
│       ├── cp1Geometry.h             <-- CP1 pipeline entry point
│       ├── pipeline/
│       │   └── BlueprintRenderer.hpp <-- Amy's pure C++ offline renderer
│       └── cp1_output/               <-- Auto-generated verification PNGs
│           ├── cp1_floorplan.png       (Scanline polygon fill + Bresenham lines)
│           ├── cp1_walls_wireframe.png (Pure C++ 3D MVP math + Bresenham lines)
│           ├── cp1_minimap_hud.png     (Integrated algorithms)
│           └── cp1_gameplay_3d_hud.png (Hybrid hardware + software runtime)
├── src/
│   ├── algorithms/                   <-- PURE C++ GRAPHICS ALGORITHMS (Zero OpenGL)
│   │   ├── SoftwareCanvas.hpp        (Raw CPU pixel memory buffer & PNG export)
│   │   ├── LineDrawing.hpp           (Bresenham's line algorithm)
│   │   ├── PolygonOps.hpp            (Polygon transforms & scanline fill)
│   │   ├── Clipping.hpp              (Cohen-Sutherland line clipping)
│   │   ├── CircleDrawing.hpp         (Midpoint circle algorithm)
│   │   ├── EllipseDrawing.hpp        (Midpoint ellipse algorithm)
│   │   ├── CurveDrawing.hpp          (De Casteljau Bézier curve)
│   │   └── MatrixMath3D.hpp          (Pure C++ 4x4 matrix and projection math)
│   ├── level/
│   │   └── HouseGrid.hpp             (Symbolic 2D maze parser)
│   └── main.cpp                      (Hybrid execution & window loop)
```

---

## 3. Syllabus Mapping & Algorithm Implementation Catalog

Graphics techniques are implemented in pure C++ in `src/algorithms/`:

| Concept| Algorithm Implemented | Source File & Class | Mathematical Method / Formula | Project Role & Demonstration Output |
| :--- | :--- | :--- | :--- | :--- |
| **Line Drawing & Operations** | **Bresenham’s Line Algorithm** | [`LineDrawing.hpp`](../../src/algorithms/LineDrawing.hpp)<br>`LineDrawing::drawLine()` | Integer error decision variable:<br>$p_{k+1} = p_k + 2\Delta y - 2\Delta x (y_{k+1}-y_k)$<br>Zero floating point division. | • Architectural floor plan grid lines.<br>• 12 edges per 3D cube in wireframe view.<br>• Output: `cp1_floorplan.png`, `cp1_walls_wireframe.png`. |
| **Polygon Representation & Operations** | **2D Affine Transforms & Polygon Bounds** | [`PolygonOps.hpp`](../../src/algorithms/PolygonOps.hpp)<br>`Polygon2D`<br>`translate(), rotate(), scale()` | $2\text{D}$ matrix operations:<br>$\begin{bmatrix} x' \\ y' \end{bmatrix} = \begin{bmatrix} \cos\theta & -\sin\theta \\ \sin\theta & \cos\theta \end{bmatrix} \begin{bmatrix} x \\ y \end{bmatrix}$ | • 2D wall block representation.<br>• Closet doors and rotated diamond fuse items.<br>• Output: `cp1_floorplan.png`. |
| **Polygon Filling** | **Scanline Polygon Fill** | [`PolygonOps.hpp`](../../src/algorithms/PolygonOps.hpp)<br>`Polygon2D::scanlineFill()` | Edge Table (ET) & parity rule:<br>Calculates edge intersections for each scanline $y$, sorts $x$-intercepts, fills pairs. | • Solid wall fills.<br>• Room floor tiling and objective markers.<br>• Output: `cp1_floorplan.png`, `cp1_minimap_hud.png`. |
| **Polygon & Line Clipping** | **Cohen-Sutherland Line Clipping** | [`Clipping.hpp`](../../src/algorithms/Clipping.hpp)<br>`CohenSutherlandClipping::clipAndDrawLine()` | 4-bit region outcodes (TOP, BOTTOM, RIGHT, LEFT):<br>Trivial accept `code0 \| code1 == 0`<br>Trivial reject `code0 & code1 != 0` | • Clips sightlines and radar vectors to HUD viewport boundaries.<br>• Output: `cp1_minimap_hud.png`. |
| **Circle Representation** | **Midpoint Circle Algorithm** | [`CircleDrawing.hpp`](../../src/algorithms/CircleDrawing.hpp)<br>`CircleDrawing::drawCircle()`, `drawFilledCircle()` | Integer decision parameter:<br>$p_0 = 1 - r$<br>Exploits 8-way symmetric point plotting. | • Player position indicator dot.<br>• Sonar radar echo pulse rings.<br>• Output: `cp1_minimap_hud.png`. |
| **Ellipse Representation** | **Midpoint Ellipse Algorithm** | [`EllipseDrawing.hpp`](../../src/algorithms/EllipseDrawing.hpp)<br>`EllipseDrawing::drawEllipse()`, `drawFilledEllipse()` | Two-region decision parameters:<br>Region 1 ($2r_y^2 x \le 2r_x^2 y$): $p_{10} = r_y^2 - r_x^2 r_y + \frac{1}{4}r_x^2$<br>Region 2: $p_{20} = r_y^2(x+\frac{1}{2})^2 + r_x^2(y-1)^2 - r_x^2 r_y^2$<br>4-way symmetry with scanline fill. | • Sprinting/crouching acoustic footstep sound radius.<br>• Angled flashlight floor projection footprint. |
| **Curves & Splines** | **De Casteljau’s Cubic Bézier Curves** | [`CurveDrawing.hpp`](../../src/algorithms/CurveDrawing.hpp)<br>`CurveDrawing::deCasteljau()`, `drawCubicBezier()` | Recursive linear interpolation (`lerp`) across 4 control points $P_0, P_1, P_2, P_3$ at parameter $t \in [0, 1]$. | • Monster AI patrol splines and stalker wandering paths.<br>• Natural path interpolation. |

---

## 4. Offline Blueprint Pipeline Demonstration

We implemented an **offline geometry verification suite** in `checkpoints/checkpoint1/pipeline/BlueprintRenderer.hpp`. When the application initializes, it executes `cp1::runGeometryDemo()` to generate two standalone image proofs:

### 1. Top-Down Architectural Blueprint (`cp1_floorplan.png`)
- **Algorithms Used:** Scanline Polygon Fill (`Polygon2D::scanlineFill`) and Bresenham’s Line Algorithm (`LineDrawing::drawLine`).
- **How it works:**
  - Iterates over the symbolic layout parsed by `HouseGrid.hpp`.
  - For each cell, constructs a 4-vertex `Polygon2D` rectangle and invokes scanline filling to color wall blocks and checkered floor tiles.
  - Constructs diamond polygons for fuses (`'F'`) and triangular door indicators for closets (`'H'`).
  - Uses Bresenham lines to draw the architectural coordinate grid and wall borders.

### 2. 3D Software Wireframe Scene (`cp1_walls_wireframe.png`)
- **Algorithms Used:** Pure C++ 3D Matrix Math (`MatrixMath3D.hpp`) and Bresenham’s Line Algorithm (`LineDrawing::drawLine`).
- **How it works:**
  - Defines unit 3D cube vertices ($v \in \mathbb{R}^3$) for every wall in the maze.
  - Calculates a full Model-View-Projection (MVP) transform chain on the CPU:
    $$\text{clipPos} = \text{Projection} \times \text{View} \times \text{Model} \times \begin{bmatrix} x \\ y \\ z \\ 1 \end{bmatrix}$$
  - Performs perspective division ($x_{\text{ndc}} = x_{\text{clip}} / w_{\text{clip}}$, $y_{\text{ndc}} = y_{\text{clip}} / w_{\text{clip}}$) and viewport mapping to screen pixels.
  - Rasterizes all 12 wireframe edges of every 3D wall, fuse, and closet using pure C++ Bresenham lines into `SoftwareCanvas`.

---

## 5. Planned Project Applications 
The algorithms implemented for Checkpoint 1 directly unlock the upcoming milestones:

1. **Bresenham Grid Raycasting (Enemy Line-of-Sight & Sound Occlusion - Phase 5):**
   - We will use Bresenham’s integer stepping to cast 2D sight rays across `HouseGrid` between the monster and the player. If an integer step encounters a wall cell (`'1'`), line-of-sight is blocked.
2. **Cohen-Sutherland Vision Cone Clipping (Phase 4 & 5):**
   - The monster’s forward field of view (FOV) will be clipped against room boundaries to cull perception checks in adjacent rooms.
3. **Midpoint Ellipse Acoustic Propagation (Phase 4):**
   - Player movement creates an acoustic ellipse whose semi-major axis aligns with the player's movement velocity vector (running elongates the ellipse forward). If the monster intersects this ellipse, it switches from patrol to chase.
4. **De Casteljau Bézier Curve Patrol Paths (Phase 5):**
   - AI entities follow smooth, natural patrol splines through corridors rather than robotic grid-locked lines.

---

## 6. How to Compile and Verify

The project uses CMake and Ninja for clean, cross-platform builds:

```powershell
# 1. Configure build directory
cmake -B build -G Ninja

# 2. Compile executable
cmake --build build

# 3. Run application (automatically generates all CP1 images before opening window)
.\build\EchoesInTheDark.exe
```

Upon launching, the terminal confirms:
```text
[CP1 Blueprint Pipeline] Generated checkpoints/checkpoint1/cp1_output/ PNGs
```
All evidence files are immediately available in [`checkpoints/checkpoint1/cp1_output/`]
