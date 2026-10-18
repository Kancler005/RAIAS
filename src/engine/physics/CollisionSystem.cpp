#include "CollisionSystem.h"

#include "engine/scene/Scene.h"

#include <cmath>

glm::vec2 CollisionSystem::GetHalfExtentsXZ(
    const Entity& entity
)
{
    const float halfX =
        std::abs(entity.transform.scale.x) * 0.5f;

    const float halfZ =
        std::abs(entity.transform.scale.z) * 0.5f;

    const float angle =
        glm::radians(
            entity.transform.rotation.y
        );

    const float cosine =
        std::abs(
            std::cos(angle)
        );

    const float sine =
        std::abs(
            std::sin(angle)
        );

    const float worldHalfX =
        cosine * halfX +
        sine * halfZ;

    const float worldHalfZ =
        sine * halfX +
        cosine * halfZ;

    return {
        worldHalfX,
        worldHalfZ
    };
}

bool CollisionSystem::OverlapsXZ(
    const Entity& mover,
    const glm::vec3& moverPosition,
    const Entity& obstacle
)
{
    const glm::vec2 moverHalf =
        GetHalfExtentsXZ(
            mover
        );

    const glm::vec2 obstacleHalf =
        GetHalfExtentsXZ(
            obstacle
        );

    const float deltaX =
        std::abs(
            moverPosition.x -
            obstacle.transform.position.x
        );

    const float deltaZ =
        std::abs(
            moverPosition.z -
            obstacle.transform.position.z
        );

    const bool overlapX =
        deltaX <
        (
            moverHalf.x +
            obstacleHalf.x
        );

    const bool overlapZ =
        deltaZ <
        (
            moverHalf.y +
            obstacleHalf.y
        );

    return
        overlapX &&
        overlapZ;
}

bool CollisionSystem::WouldCollide(
    const Scene& scene,
    const Entity& mover,
    const glm::vec3& candidatePosition
)
{
    for (
        const Entity& obstacle :
        scene.GetEntities()
    )
    {
        if (&obstacle == &mover)
        {
            continue;
        }

        if (!obstacle.collidable)
        {
            continue;
        }

        if (
            OverlapsXZ(
                mover,
                candidatePosition,
                obstacle
            )
        )
        {
            return true;
        }
    }

    return false;
}

void CollisionSystem::Move(
    const Scene& scene,
    Entity& mover,
    const glm::vec3& movement
)
{
    glm::vec3 candidate =
        mover.transform.position;

    candidate.x +=
        movement.x;

    if (
        !WouldCollide(
            scene,
            mover,
            candidate
        )
    )
    {
        mover.transform.position.x =
            candidate.x;
    }

    candidate =
        mover.transform.position;

    candidate.z +=
        movement.z;

    if (
        !WouldCollide(
            scene,
            mover,
            candidate
        )
    )
    {
        mover.transform.position.z =
            candidate.z;
    }
}