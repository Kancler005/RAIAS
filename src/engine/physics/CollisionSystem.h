#pragma once

#include <glm/glm.hpp>

class Scene;
struct Entity;

class CollisionSystem
{
public:
    static void Move(
        const Scene& scene,
        Entity& mover,
        const glm::vec3& movement
    );

    static bool WouldCollide(
        const Scene& scene,
        const Entity& mover,
        const glm::vec3& candidatePosition
    );

private:
    static glm::vec2 GetHalfExtentsXZ(
        const Entity& entity
    );

    static bool OverlapsXZ(
        const Entity& mover,
        const glm::vec3& moverPosition,
        const Entity& obstacle
    );
};