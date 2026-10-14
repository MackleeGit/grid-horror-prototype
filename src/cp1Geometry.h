#ifndef CP1_GEOMETRY_H
#define CP1_GEOMETRY_H

// CP1 Pure C++ Architectural Pipeline & Offline Geometry
// Modularized into:
//   - src/level/HouseGrid.hpp        (Symbolic house layout and cell parser)
//   - src/pipeline/BlueprintRenderer.hpp (Offline CPU floor plan and 3D wireframe renderers)
//   - src/algorithms/MatrixMath3D.hpp    (Pure C++ 4x4 matrix and projection math)
//   - src/algorithms/SoftwareCanvas.hpp  (CPU buffer and zero-dependency PNG export)
// All line drawing and polygon fills invoke src/algorithms/LineDrawing.hpp and PolygonOps.hpp.

#include "level/HouseGrid.hpp"
#include "algorithms/MatrixMath3D.hpp"
#include "pipeline/BlueprintRenderer.hpp"

#endif // CP1_GEOMETRY_H