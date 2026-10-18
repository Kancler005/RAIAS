#include "SceneSerializer.h"

#include "Scene.h"

#include <SDL3/SDL.h>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <utility>

using json =
    nlohmann::json;

bool SceneSerializer::Save(
    const Scene& scene,
    const std::string& path
)
{
    json root;

    root["format"] =
        "RAI Scene";

    root["version"] =
        2;

    root["entities"] =
        json::array();

    for (
        const Entity& entity :
        scene.GetEntities()
    )
    {
        json entityJson;

        entityJson["id"] =
            entity.id;

        entityJson["name"] =
            entity.name;

        entityJson["prefab"] =
            entity.prefab;

        entityJson["visible"] =
            entity.visible;

        entityJson["collidable"] =
            entity.collidable;

        entityJson["color"] =
        {
            entity.color.r,
            entity.color.g,
            entity.color.b,
            entity.color.a
        };

        entityJson["transform"]["position"] =
        {
            entity.transform.position.x,
            entity.transform.position.y,
            entity.transform.position.z
        };

        entityJson["transform"]["rotation"] =
        {
            entity.transform.rotation.x,
            entity.transform.rotation.y,
            entity.transform.rotation.z
        };

        entityJson["transform"]["scale"] =
        {
            entity.transform.scale.x,
            entity.transform.scale.y,
            entity.transform.scale.z
        };

        root["entities"].push_back(
            entityJson
        );
    }

    const std::filesystem::path filePath(
        path
    );

    if (
        !filePath
            .parent_path()
            .empty()
    )
    {
        std::error_code error;

        std::filesystem::create_directories(
            filePath.parent_path(),
            error
        );

        if (
            error
        )
        {
            SDL_Log(
                "Could not create directory: %s",
                error.message().c_str()
            );

            return false;
        }
    }

    std::ofstream file(
        path
    );

    if (
        !file.is_open()
    )
    {
        SDL_Log(
            "Could not save Scene: %s",
            path.c_str()
        );

        return false;
    }

    file <<
        root.dump(4);

    SDL_Log(
        "Scene saved: %s",
        path.c_str()
    );

    return true;
}

bool SceneSerializer::Load(
    Scene& scene,
    const std::string& path
)
{
    std::ifstream file(
        path
    );

    if (
        !file.is_open()
    )
    {
        SDL_Log(
            "Scene file not found: %s",
            path.c_str()
        );

        return false;
    }

    json root;

    try
    {
        file >>
            root;
    }
    catch (
        const json::exception& exception
    )
    {
        SDL_Log(
            "JSON parsing failed: %s",
            exception.what()
        );

        return false;
    }

    if (
        !root.contains(
            "entities"
        ) ||
        !root["entities"].is_array()
    )
    {
        SDL_Log(
            "Invalid Scene file."
        );

        return false;
    }

    Scene loadedScene;

    try
    {
        for (
            const json& entityJson :
            root["entities"]
        )
        {
            const std::string name =
                entityJson.value(
                    "name",
                    "Entity"
                );

            const std::string prefab =
                entityJson.value(
                    "prefab",
                    "cube"
                );

            Entity* entityPointer =
                nullptr;

            // Старые Scene-файлы без ID тоже поддерживаем.

            if (
                entityJson.contains(
                    "id"
                )
            )
            {
                const EntityId id =
                    entityJson["id"]
                        .get<EntityId>();

                entityPointer =
                    &loadedScene.CreateEntityWithId(
                        id,
                        name,
                        prefab
                    );
            }
            else
            {
                entityPointer =
                    &loadedScene.CreateEntity(
                        name,
                        prefab
                    );
            }

            Entity& entity =
                *entityPointer;

            entity.visible =
                entityJson.value(
                    "visible",
                    true
                );

            entity.collidable =
                entityJson.value(
                    "collidable",
                    false
                );

            if (
                entityJson.contains(
                    "color"
                )
            )
            {
                const json& color =
                    entityJson["color"];

                if (
                    color.is_array() &&
                    color.size() == 4
                )
                {
                    entity.color =
                    {
                        color[0].get<float>(),
                        color[1].get<float>(),
                        color[2].get<float>(),
                        color[3].get<float>()
                    };
                }
            }

            if (
                entityJson.contains(
                    "transform"
                )
            )
            {
                const json& transform =
                    entityJson["transform"];

                if (
                    transform.contains(
                        "position"
                    )
                )
                {
                    const json& position =
                        transform["position"];

                    if (
                        position.is_array() &&
                        position.size() == 3
                    )
                    {
                        entity.transform.position =
                        {
                            position[0].get<float>(),
                            position[1].get<float>(),
                            position[2].get<float>()
                        };
                    }
                }

                if (
                    transform.contains(
                        "rotation"
                    )
                )
                {
                    const json& rotation =
                        transform["rotation"];

                    if (
                        rotation.is_array() &&
                        rotation.size() == 3
                    )
                    {
                        entity.transform.rotation =
                        {
                            rotation[0].get<float>(),
                            rotation[1].get<float>(),
                            rotation[2].get<float>()
                        };
                    }
                }

                if (
                    transform.contains(
                        "scale"
                    )
                )
                {
                    const json& scale =
                        transform["scale"];

                    if (
                        scale.is_array() &&
                        scale.size() == 3
                    )
                    {
                        entity.transform.scale =
                        {
                            scale[0].get<float>(),
                            scale[1].get<float>(),
                            scale[2].get<float>()
                        };
                    }
                }
            }
        }
    }
    catch (
        const json::exception& exception
    )
    {
        SDL_Log(
            "Scene data error: %s",
            exception.what()
        );

        return false;
    }

    scene =
        std::move(
            loadedScene
        );

    SDL_Log(
        "Scene loaded: %s | Entities: %zu",
        path.c_str(),
        scene.GetEntities().size()
    );

    return true;
}