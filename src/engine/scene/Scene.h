#pragma once

#include "Entity.h"

#include <deque>
#include <string>

class Scene
{
public:
    Entity& CreateEntity(
        const std::string& name,
        const std::string& prefab = "cube"
    );

    Entity& CreateEntityWithId(
        EntityId id,
        const std::string& name,
        const std::string& prefab = "cube"
    );

    Entity* FindEntity(
        const std::string& name
    );

    const Entity* FindEntity(
        const std::string& name
    ) const;

    Entity* FindEntityById(
        EntityId id
    );

    const Entity* FindEntityById(
        EntityId id
    ) const;

    std::deque<Entity>& GetEntities();

    const std::deque<Entity>& GetEntities() const;

private:
    EntityId nextEntityId =
        1;

    std::deque<Entity> entities;
};