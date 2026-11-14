#ifndef COLLISION_ENGINE_HPP
#define COLLISION_ENGINE_HPP

#include "../level/HouseGrid.hpp"
#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>

class CollisionEngine {
public:
    struct AABB {
        float minX;
        float maxX;
        float minZ;
        float maxZ;
    };
};

#endif // COLLISION_ENGINE_HPP
