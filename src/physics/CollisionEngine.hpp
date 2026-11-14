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
};

#endif // COLLISION_ENGINE_HPP
