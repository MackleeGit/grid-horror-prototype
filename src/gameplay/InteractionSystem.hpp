
#ifndef INTERACTION_SYSTEM_HPP
#define INTERACTION_SYSTEM_HPP

#include "../level/HouseGrid.hpp"

#include <glm/glm.hpp>

#include <cmath>
#include <iostream>
#include <vector>

// Represents one fuse objective in the game world.
struct FuseItem {
    glm::vec3 position;
    bool collected = false;
};

class InteractionSystem {
public:
    std::vector<FuseItem> fuses;

    int fusesCollected = 0;
    const int totalFuses = 3;

    bool isPlayerHidden = false;

    // Scan the house grid and register every fuse ('F').
    void initialize(const cp1::Grid& grid) {
        fuses.clear();
        fusesCollected = 0;
        isPlayerHidden = false;

        if (grid.empty() || grid[0].empty()) {
            std::cerr << "[INTERACTION] Cannot initialize: empty house grid.\n";
            return;
        }

        const int mapRows = static_cast<int>(grid.size());
        const int mapCols = static_cast<int>(grid[0].size());

        for (int row = 0; row < mapRows; ++row) {
            for (int col = 0; col < mapCols; ++col) {

                if (cp1::cellAt(grid, col, row) == 'F') {

                    // Match the world-coordinate convention in main.cpp.
                    float worldX =
                        static_cast<float>(col) - mapCols * 0.5f;

                    float worldZ =
                        static_cast<float>(row) - mapRows * 0.5f;

                    fuses.push_back({
                        glm::vec3(worldX, 0.0f, worldZ),
                        false
                    });
                }
            }
        }

        std::cout << "[INTERACTION] Registered "
                  << fuses.size() << " fuse objectives.\n";
    }

    // Called when the player presses E.
    void interact(const glm::vec3& playerPos, const cp1::Grid& grid) {

        if (grid.empty() || grid[0].empty()) {
            return;
        }

        // 1. Check for nearby uncollected fuses.
        for (auto& fuse : fuses) {

            if (!fuse.collected) {

                // Ignore vertical distance because this is a ground-level objective.
                glm::vec2 playerXZ(playerPos.x, playerPos.z);
                glm::vec2 fuseXZ(fuse.position.x, fuse.position.z);

                float distance = glm::distance(playerXZ, fuseXZ);

                if (distance <= 1.2f) {

                    fuse.collected = true;
                    fusesCollected++;

                    std::cout << "[OBJECTIVE] Collected Fuse! ("
                              << fusesCollected << "/"
                              << totalFuses << ")\n";

                    return;
                }
            }
        }

        // 2. Check whether the player is standing on a closet tile.
        const int mapRows = static_cast<int>(grid.size());
        const int mapCols = static_cast<int>(grid[0].size());

        int col = static_cast<int>(
            std::floor(playerPos.x + mapCols * 0.5f + 0.5f)
        );

        int row = static_cast<int>(
            std::floor(playerPos.z + mapRows * 0.5f + 0.5f)
        );

        if (cp1::cellAt(grid, col, row) == 'H') {

            isPlayerHidden = !isPlayerHidden;

            if (isPlayerHidden) {
                std::cout
                    << "[HIDING] Entered closet. Safe from entity.\n";
            } else {
                std::cout
                    << "[HIDING] Exited closet.\n";
            }
        }
    }

    // Returns true when all three fuse objectives have been collected.
    bool allFusesCollected() const {
        return fusesCollected >= totalFuses;
    }
};

#endif // INTERACTION_SYSTEM_HPP
