#include "Scene.h"

Entity& Scene::CreateEntity(
    const std::string& name,
    const std::string& prefab
)
{
    Entity entity;

    entity.id =
        nextEntityId++;

    entity.name =
        name;

    entity.prefab =
        prefab;

    entities.push_back(
        entity
    );

    return entities.back();
}

Entity& Scene::CreateEntityWithId(
    EntityId id,
    const std::string& name,
    const std::string& prefab
)
{
    Entity entity;

    entity.id =
        id;

    entity.name =
        name;

    entity.prefab =
        prefab;

    entities.push_back(
        entity
    );

    if (
        id >= nextEntityId
    )
    {
        nextEntityId =
            id + 1;
    }

    return entities.back();
}

Entity* Scene::FindEntity(
    const std::string& name
)
{
    for (
        Entity& entity :
        entities
    )
    {
        if (
            entity.name ==
            name
        )
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
    for (
        const Entity& entity :
        entities
    )
    {
        if (
            entity.name ==
            name
        )
        {
            return &entity;
        }
    }

    return nullptr;
}

Entity* Scene::FindEntityById(
    EntityId id
)
{
    for (
        Entity& entity :
        entities
    )
    {
        if (
            entity.id ==
            id
        )
        {
            return &entity;
        }
    }

    return nullptr;
}

const Entity* Scene::FindEntityById(
    EntityId id
) const
{
    for (
        const Entity& entity :
        entities
    )
    {
        if (
            entity.id ==
            id
        )
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