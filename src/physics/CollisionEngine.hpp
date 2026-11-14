#ifndef COLLISION_ENGINE_HPP
#define COLLISION_ENGINE_HPP

#include "../level/HouseGrid.hpp"
#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>

class CollisionEngine {
public:
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

    static bool isColliding(float x, float z, const cp1::Grid& grid) {
        int mapRows = static_cast<int>(grid.size());
        int mapCols = static_cast<int>(grid[0].size());
        AABB playerBox = playerBoundsAt(x, z);

        int col = static_cast<int>(std::floor(x + mapCols * 0.5f + 0.5f));
        int row = static_cast<int>(std::floor(z + mapRows * 0.5f + 0.5f));
        if (cp1::cellAt(grid, col, row) == '1' &&
            overlaps(playerBox, wallBoundsAt(col, row, mapCols, mapRows))) {
            return true;
        }
        return false;
    }

    static glm::vec3 resolveMovement(const glm::vec3& currentPos,
                                     const glm::vec3& deltaMovement,
                                     const cp1::Grid& grid) {
        glm::vec3 newPos = currentPos;
        float candidateX = currentPos.x + deltaMovement.x;
        if (!isColliding(candidateX, currentPos.z, grid)) {
            newPos.x = candidateX;
        }

        float candidateZ = currentPos.z + deltaMovement.z;
        if (!isColliding(currentPos.x, candidateZ, grid)) {
            newPos.z = candidateZ;
        }

        newPos.y = 0.0f;
        return newPos;
    }
};

#endif // COLLISION_ENGINE_HPP
