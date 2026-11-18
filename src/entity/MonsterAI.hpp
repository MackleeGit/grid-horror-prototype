
#ifndef MONSTER_AI_HPP
#define MONSTER_AI_HPP

#include "../level/HouseGrid.hpp"
#include "../algorithms/CurveDrawing.hpp"
#include "../gameplay/GameState.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <queue>
#include <vector>

enum class MonsterMode {
    PATROL,
    CHASE
};

class MonsterAI {
private:
    struct GridCell {
        int col;
        int row;
    };

    bool returningToPatrol = false;

    GridCell worldToCell(
        const glm::vec3& worldPos,
        const cp1::Grid& grid
    ) const {
        int rows = static_cast<int>(grid.size());
        int cols = rows > 0
            ? static_cast<int>(grid[0].size())
            : 0;

        return {
            static_cast<int>(
                std::floor(worldPos.x + cols * 0.5f + 0.5f)
            ),
            static_cast<int>(
                std::floor(worldPos.z + rows * 0.5f + 0.5f)
            )
        };
    }

    glm::vec2 cellToWorld(
        const GridCell& cell,
        const cp1::Grid& grid
    ) const {
        int rows = static_cast<int>(grid.size());
        int cols = rows > 0
            ? static_cast<int>(grid[0].size())
            : 0;

        return glm::vec2(
            static_cast<float>(cell.col) - cols * 0.5f,
            static_cast<float>(cell.row) - rows * 0.5f
        );
    }

    bool isWalkable(
        int col,
        int row,
        const cp1::Grid& grid
    ) const {
        return cp1::cellAt(grid, col, row) != '1';
    }

    bool isWalkableWorld(
        const glm::vec3& worldPos,
        const cp1::Grid& grid
    ) const {
        GridCell cell = worldToCell(worldPos, grid);

        return isWalkable(cell.col, cell.row, grid);
    }

    std::vector<GridCell> findPath(
        const glm::vec3& targetPos,
        const cp1::Grid& grid
    ) const {
        std::vector<GridCell> path;

        if (grid.empty() || grid[0].empty()) {
            return path;
        }

        int rows = static_cast<int>(grid.size());
        int cols = static_cast<int>(grid[0].size());

        GridCell start = worldToCell(position, grid);
        GridCell goal = worldToCell(targetPos, grid);

        if (!isWalkable(start.col, start.row, grid) ||
            !isWalkable(goal.col, goal.row, grid)) {
            return path;
        }

        if (start.col == goal.col &&
            start.row == goal.row) {
            return path;
        }

        const int totalCells = rows * cols;

        const int directions[4][2] = {
            { 1, 0 },
            { -1, 0 },
            { 0, 1 },
            { 0, -1 }
        };

        auto toIndex = [cols](int col, int row) {
            return row * cols + col;
        };

        std::vector<int> parent(totalCells, -1);
        std::vector<bool> visited(totalCells, false);

        std::queue<GridCell> pending;

        int startIndex = toIndex(start.col, start.row);
        int goalIndex = toIndex(goal.col, goal.row);

        pending.push(start);
        visited[startIndex] = true;

        while (!pending.empty()) {
            GridCell current = pending.front();
            pending.pop();

            int currentIndex =
                toIndex(current.col, current.row);

            if (currentIndex == goalIndex) {
                break;
            }

            for (const auto& direction : directions) {
                int nextCol = current.col + direction[0];
                int nextRow = current.row + direction[1];

                if (!isWalkable(nextCol, nextRow, grid)) {
                    continue;
                }

                int nextIndex = toIndex(nextCol, nextRow);

                if (visited[nextIndex]) {
                    continue;
                }

                visited[nextIndex] = true;
                parent[nextIndex] = currentIndex;

                pending.push({ nextCol, nextRow });
            }
        }

        if (!visited[goalIndex]) {
            return path;
        }

        int currentIndex = goalIndex;

        while (currentIndex != startIndex) {
            int col = currentIndex % cols;
            int row = currentIndex / cols;

            path.push_back({ col, row });

            currentIndex = parent[currentIndex];

            if (currentIndex < 0) {
                path.clear();
                return path;
            }
        }

        std::reverse(path.begin(), path.end());

        return path;
    }

    void moveToward(
        const glm::vec2& target,
        float speed,
        float deltaTime,
        const cp1::Grid& grid
    ) {
        glm::vec2 current(position.x, position.z);

        glm::vec2 difference = target - current;

        float distance = glm::length(difference);

        if (distance < 0.001f) {
            return;
        }

        float step = std::max(0.0f, speed * deltaTime);

        float movement = std::min(step, distance);

        glm::vec2 direction = difference / distance;

        glm::vec2 nextPosition =
            current + direction * movement;

        glm::vec3 candidate(
            nextPosition.x,
            0.0f,
            nextPosition.y
        );

        if (isWalkableWorld(candidate, grid)) {
            position.x = nextPosition.x;
            position.z = nextPosition.y;
        }
    }

    void followPath(
        const glm::vec3& targetPos,
        float deltaTime,
        const cp1::Grid& grid
    ) {
        std::vector<GridCell> path =
            findPath(targetPos, grid);

        if (path.empty()) {
            return;
        }

        glm::vec2 nextTarget =
            cellToWorld(path.front(), grid);

        moveToward(
            nextTarget,
            moveSpeed,
            deltaTime,
            grid
        );
    }

public:
    glm::vec3 position{ 1.0f, 0.0f, -5.0f };

    // Monster movement and difficulty settings
    float moveSpeed = 1.0f;
    float chaseDistance = 5.0f;
    float catchDistance = 1.2f;

    MonsterMode mode = MonsterMode::PATROL;

    Point2D p0{ 1.0f, -5.0f };
    Point2D p1{ 5.0f, -5.0f };
    Point2D p2{ 5.0f, -4.0f };
    Point2D p3{ 1.0f, -4.0f };

    float splineT = 0.0f;
    bool splineForward = true;

    void initialize(
        float spawnX = 1.0f,
        float spawnZ = -5.0f
    ) {
        position = glm::vec3(spawnX, 0.0f, spawnZ);

        mode = MonsterMode::PATROL;

        splineT = 0.0f;
        splineForward = true;

        returningToPatrol = false;
    }

    GameState update(
        float deltaTime,
        const glm::vec3& playerPos,
        bool isPlayerHidden,
        bool allFusesCollected,
        const cp1::Grid& grid
    ) {
        deltaTime = std::clamp(deltaTime, 0.0f, 0.1f);

        glm::vec2 monsterPos(position.x, position.z);
        glm::vec2 playerPosition(playerPos.x, playerPos.z);

        float distToPlayer =
            glm::distance(monsterPos, playerPosition);

        // Check whether the monster has caught the player.
        if (!isPlayerHidden &&
            distToPlayer <= catchDistance) {

            std::cout
                << "[GAME OVER] The entity caught you in the dark!\n";

            return GameState::CAUGHT;
        }

        MonsterMode previousMode = mode;

        // Chase only when the player is within the configured range.
        if (!isPlayerHidden &&
            distToPlayer < chaseDistance) {

            mode = MonsterMode::CHASE;
        }
        else {
            mode = MonsterMode::PATROL;
        }

        if (previousMode == MonsterMode::CHASE &&
            mode == MonsterMode::PATROL) {

            returningToPatrol = true;
        }

        if (mode == MonsterMode::CHASE) {
            returningToPatrol = false;

            followPath(
                playerPos,
                deltaTime,
                grid
            );
        }
        else if (returningToPatrol) {
            glm::vec2 patrolStart(p0.x, p0.y);

            float distanceToPatrol = glm::distance(
                glm::vec2(position.x, position.z),
                patrolStart
            );

            if (distanceToPatrol < 0.1f) {
                position.x = p0.x;
                position.z = p0.y;

                returningToPatrol = false;
            }
            else {
                glm::vec3 patrolTarget(
                    p0.x,
                    0.0f,
                    p0.y
                );

                followPath(
                    patrolTarget,
                    deltaTime,
                    grid
                );
            }
        }
        else {
            // Normal patrol movement along the Bezier curve.
            float tStep = 0.12f * deltaTime;

            if (splineForward) {
                splineT += tStep;

                if (splineT >= 1.0f) {
                    splineT = 1.0f;
                    splineForward = false;
                }
            }
            else {
                splineT -= tStep;

                if (splineT <= 0.0f) {
                    splineT = 0.0f;
                    splineForward = true;
                }
            }

            Point2D nextPos =
                CurveDrawing::deCasteljau(
                    p0,
                    p1,
                    p2,
                    p3,
                    splineT
                );

            glm::vec3 candidate(
                nextPos.x,
                0.0f,
                nextPos.y
            );

            if (isWalkableWorld(candidate, grid)) {
                position.x = nextPos.x;
                position.z = nextPos.y;
            }
        }

        position.y = 0.0f;

        // Victory condition: all fuses collected and player reaches exit.
        if (allFusesCollected &&
            playerPos.z <= -5.0f &&
            std::abs(playerPos.x) < 2.0f) {

            std::cout
                << "[VICTORY] You restored power and escaped the house!\n";

            return GameState::ESCAPED;
        }

        return isPlayerHidden
            ? GameState::HIDING
            : GameState::PLAYING;
    }

    float getProximityFlicker(
        const glm::vec3& playerPos,
        float timeSeconds
    ) const {
        float dist = glm::distance(
            glm::vec2(position.x, position.z),
            glm::vec2(playerPos.x, playerPos.z)
        );

        if (dist > 7.0f) {
            return 1.0f;
        }

        float intensityFactor = dist / 7.0f;

        float jitter =
            std::sin(timeSeconds * 40.0f) *
            std::cos(timeSeconds * 15.0f);

        if (jitter < -0.3f) {
            return std::max(
                0.1f,
                0.1f * intensityFactor
            );
        }

        return std::clamp(
            intensityFactor + 0.2f,
            0.1f,
            1.0f
        );
    }
};

#endif // MONSTER_AI_HPP
