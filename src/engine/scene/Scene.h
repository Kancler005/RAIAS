#pragma once

#include "Entity.h"

#include <deque>
#include <string>

class Scene
{
public:

    Entity& CreateEntity(
        const std::string& name
    );

    Entity* FindEntity(
        const std::string& name
    );

    const Entity* FindEntity(
        const std::string& name
    ) const;

    std::deque<Entity>& GetEntities();

    const std::deque<Entity>& GetEntities() const;

private:

    std::deque<Entity> entities;
};