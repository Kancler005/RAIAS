#include "Scene.h"

Entity& Scene::CreateEntity(
    const std::string& name
)
{
    Entity entity;

    entity.name = name;

    entities.push_back(entity);

    return entities.back();
}

Entity* Scene::FindEntity(
    const std::string& name
)
{
    for (Entity& entity : entities)
    {
        if (entity.name == name)
        {
            return &entity;
        }
    }

    return nullptr;
}

const Entity* Scene::FindEntity(
    const std::string& name
) const
{
    for (const Entity& entity : entities)
    {
        if (entity.name == name)
        {
            return &entity;
        }
    }

    return nullptr;
}

std::deque<Entity>& Scene::GetEntities()
{
    return entities;
}

const std::deque<Entity>& Scene::GetEntities() const
{
    return entities;
}