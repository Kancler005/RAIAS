#include "EnemySystem.h"

#include "GameState.h"

#include "engine/physics/CollisionSystem.h"
#include "engine/scene/Scene.h"

#include <glm/glm.hpp>

// =========================================================
// Enemy AI
// =========================================================

void EnemySystem::Update(
    Scene& scene,
    const GameState& gameState,
    float deltaTime
)
{
    Entity* player =
        scene.FindEntityById(
            gameState.playerId
        );

    if (
        player == nullptr
    )
    {
        return;
    }

    constexpr float enemySpeed =
        1.25f;

    constexpr float aggroRange =
        6.0f;

    constexpr float stopDistance =
        0.9f;

    const float aggroRangeSquared =
        aggroRange *
        aggroRange;

    const float stopDistanceSquared =
        stopDistance *
        stopDistance;

    for (
        const EntityId enemyId :
        gameState.enemyIds
    )
    {
        Entity* enemy =
            scene.FindEntityById(
                enemyId
            );

        if (
            enemy == nullptr
        )
        {
            continue;
        }

        glm::vec3 direction =
            player->transform.position -
            enemy->transform.position;

        direction.y =
            0.0f;

        const float distanceSquared =
            glm::dot(
                direction,
                direction
            );

        // Enemy too far away.
        if (
            distanceSquared >
            aggroRangeSquared
        )
        {
            continue;
        }

        // Enemy reached player.
        if (
            distanceSquared <=
            stopDistanceSquared
        )
        {
            continue;
        }

        direction =
            glm::normalize(
                direction
            );

        const glm::vec3 movement =
            direction *
            enemySpeed *
            deltaTime;

        CollisionSystem::Move(
            scene,
            *enemy,
            movement
        );
    }
}