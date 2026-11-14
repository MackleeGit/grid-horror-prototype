#ifndef COLLISION_ENGINE_HPP
#define COLLISION_ENGINE_HPP

#include "../level/HouseGrid.hpp"
#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>

class CollisionEngine {
public:
    static constexpr float PLAYER_RADIUS = 0.3f;

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
};

#endif // COLLISION_ENGINE_HPP
