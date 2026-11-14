#ifndef COLLISION_ENGINE_HPP
#define COLLISION_ENGINE_HPP

#include "../level/HouseGrid.hpp"
#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>

// Axis-Aligned Bounding Box (AABB) Collision Engine
// Evaluates player horizontal boundaries against HouseGrid wall cells ('1')
// and computes smooth wall-sliding response along corridor edges.
class CollisionEngine {
public:
    // Physical bounding radius (0.3 units prevents visual mesh clipping into walls)
    static constexpr float PLAYER_RADIUS = 0.3f;
    static constexpr float WALL_HALF_EXTENT = 0.5f;

    struct AABB {
        float minX;
        float maxX;
        float minZ;
        float maxZ;
    };

    static AABB playerBoundsAt(float x, float z) {
        return {
            x - PLAYER_RADIUS,
            x + PLAYER_RADIUS,
            z - PLAYER_RADIUS,
            z + PLAYER_RADIUS
        };
    }

    static AABB wallBoundsAt(int col, int row, int mapCols, int mapRows) {
        float centerX = static_cast<float>(col) - mapCols * 0.5f;
        float centerZ = static_cast<float>(row) - mapRows * 0.5f;
        return {
            centerX - WALL_HALF_EXTENT,
            centerX + WALL_HALF_EXTENT,
            centerZ - WALL_HALF_EXTENT,
            centerZ + WALL_HALF_EXTENT
        };
    }

    static bool overlaps(const AABB& a, const AABB& b) {
        return a.minX < b.maxX && a.maxX > b.minX &&
               a.minZ < b.maxZ && a.maxZ > b.minZ;
    }

    // Checks whether an (x, z) candidate point with PLAYER_RADIUS overlaps any wall cell
    static bool isColliding(float x, float z, const cp1::Grid& grid) {
        if (grid.empty() || grid[0].empty()) return false;

        int mapRows = static_cast<int>(grid.size());
        int mapCols = static_cast<int>(grid[0].size());
        AABB playerBox = playerBoundsAt(x, z);

        // Test the 4 bounding box corners around the player in world space
        const float offsets[2] = { -PLAYER_RADIUS, PLAYER_RADIUS };
        for (float dx : offsets) {
            for (float dz : offsets) {
                float testX = x + dx;
                float testZ = z + dz;

                int col = static_cast<int>(std::floor(testX + mapCols * 0.5f + 0.5f));
                int row = static_cast<int>(std::floor(testZ + mapRows * 0.5f + 0.5f));

                if (cp1::cellAt(grid, col, row) == '1' &&
                    overlaps(playerBox, wallBoundsAt(col, row, mapCols, mapRows))) {
                    return true;
                }
            }
        }
        return false;
    }

    // Resolves diagonal movement with smooth wall-sliding physics:
    // If movement along X collides, keep Z movement (and vice versa)
    static glm::vec3 resolveMovement(const glm::vec3& currentPos,
                                     const glm::vec3& deltaMovement,
                                     const cp1::Grid& grid) {
        glm::vec3 newPos = currentPos;

        // 1. Resolve X movement independently
        float candidateX = currentPos.x + deltaMovement.x;
        if (!isColliding(candidateX, currentPos.z, grid)) {
            newPos.x = candidateX;
        }

        // 2. Resolve Z movement independently
        float candidateZ = currentPos.z + deltaMovement.z;
        if (!isColliding(newPos.x, candidateZ, grid)) {
            newPos.z = candidateZ;
        }

        // Lock eye-height fixed on horizontal floor plane
        newPos.y = 0.0f;
        return newPos;
    }
};

#endif // COLLISION_ENGINE_HPP
