#include "WorldExpansion.h"

#include "Scene.h"

#include <SDL3/SDL.h>

#include <glm/glm.hpp>

#include <array>
#include <string>

namespace
{
    constexpr const char* WORLD_PACK_MARKER =
        "__RAI_WORLD_PACK_0_0_24__";

    Entity& AddEnvironmentEntity(
        Scene& scene,
        const std::string& name,
        const std::string& prefab,
        const glm::vec3& position,
        const glm::vec3& rotation,
        const glm::vec3& scale,
        const glm::vec4& color,
        bool collidable
    )
    {
        Entity& entity =
            scene.CreateEntity(
                name,
                prefab
            );

        entity.transform.position =
            position;

        entity.transform.rotation =
            rotation;

        entity.transform.scale =
            scale;

        entity.color =
            color;

        entity.visible =
            true;

        entity.collidable =
            collidable;

        return entity;
    }
}

bool WorldExpansion::Apply(
    Scene& scene
)
{
    if (
        scene.FindEntity(
            WORLD_PACK_MARKER
        ) != nullptr
    )
    {
        return false;
    }

    // =====================================================
    // Territory
    // =====================================================

    Entity* ground =
        scene.FindEntity(
            "Ground"
        );

    if (
        ground == nullptr
    )
    {
        for (
            Entity& entity :
            scene.GetEntities()
        )
        {
            if (
                entity.prefab ==
                    "ground_block"
            )
            {
                ground =
                    &entity;

                break;
            }
        }
    }

    if (
        ground != nullptr
    )
    {
        ground->transform.scale.x =
            38.0f;

        ground->transform.scale.z =
            38.0f;
    }

    // =====================================================
    // Procedural Rock Field
    // =====================================================

    struct RockPlacement
    {
        const char* prefab;
        glm::vec3 position;
        float rotationY;
        glm::vec3 scale;
        glm::vec4 color;
    };

    const std::array<RockPlacement, 22>
        rocks =
    {{
        {"rock_small", {-5.4f, 0.0f, -3.4f},  18.0f, {1.0f, 0.9f, 1.1f}, {0.34f, 0.35f, 0.34f, 1.0f}},
        {"rock_wide",  {-7.1f, 0.0f, -4.3f}, -12.0f, {1.2f, 0.9f, 1.0f}, {0.30f, 0.31f, 0.31f, 1.0f}},
        {"rock_tall",  {-8.0f, 0.0f, -2.2f},  31.0f, {0.9f, 1.2f, 0.9f}, {0.38f, 0.37f, 0.34f, 1.0f}},
        {"rock_small", {-4.0f, 0.0f, -5.7f},  55.0f, {0.8f, 0.8f, 0.9f}, {0.29f, 0.30f, 0.30f, 1.0f}},

        {"rock_wide",  { 4.8f, 0.0f,  5.5f},   9.0f, {1.3f, 1.0f, 1.2f}, {0.33f, 0.32f, 0.30f, 1.0f}},
        {"rock_small", { 6.2f, 0.0f,  6.8f},  42.0f, {0.9f, 0.8f, 1.0f}, {0.36f, 0.35f, 0.32f, 1.0f}},
        {"rock_tall",  { 7.5f, 0.0f,  4.6f}, -20.0f, {0.8f, 1.1f, 0.9f}, {0.31f, 0.32f, 0.33f, 1.0f}},
        {"rock_small", { 3.5f, 0.0f,  7.4f},  70.0f, {0.7f, 0.7f, 0.8f}, {0.28f, 0.29f, 0.30f, 1.0f}},

        {"rock_small", {10.7f, 0.0f, -5.4f},  16.0f, {0.9f, 0.8f, 0.9f}, {0.35f, 0.34f, 0.31f, 1.0f}},
        {"rock_wide",  {12.3f, 0.0f, -6.6f}, -34.0f, {1.0f, 0.8f, 1.1f}, {0.30f, 0.31f, 0.30f, 1.0f}},
        {"rock_small", {11.5f, 0.0f, -9.2f},  48.0f, {0.7f, 0.7f, 0.8f}, {0.37f, 0.36f, 0.33f, 1.0f}},

        {"rock_tall",  {-11.2f, 0.0f,  7.0f},  12.0f, {0.9f, 1.0f, 0.9f}, {0.32f, 0.33f, 0.33f, 1.0f}},
        {"rock_small", {-12.6f, 0.0f,  8.4f}, -52.0f, {0.8f, 0.8f, 0.9f}, {0.35f, 0.34f, 0.32f, 1.0f}},
        {"rock_wide",  {-9.8f, 0.0f, 10.3f},  27.0f, {1.1f, 0.8f, 1.0f}, {0.29f, 0.30f, 0.30f, 1.0f}},
        {"rock_small", {-13.3f, 0.0f, 11.2f},  80.0f, {0.7f, 0.7f, 0.7f}, {0.38f, 0.37f, 0.34f, 1.0f}},

        {"rock_small", { 0.5f, 0.0f, 11.0f},  33.0f, {0.8f, 0.8f, 0.8f}, {0.33f, 0.33f, 0.31f, 1.0f}},
        {"rock_wide",  {-2.0f, 0.0f, 13.0f}, -18.0f, {1.0f, 0.9f, 1.1f}, {0.29f, 0.30f, 0.31f, 1.0f}},
        {"rock_tall",  { 2.3f, 0.0f, 14.2f},  58.0f, {0.8f, 1.0f, 0.8f}, {0.36f, 0.35f, 0.32f, 1.0f}},

        {"rock_wide",  {-1.3f, 0.0f,-13.0f},  15.0f, {1.1f, 0.8f, 1.0f}, {0.32f, 0.32f, 0.31f, 1.0f}},
        {"rock_small", { 1.2f, 0.0f,-14.6f}, -39.0f, {0.8f, 0.7f, 0.8f}, {0.35f, 0.34f, 0.32f, 1.0f}},
        {"rock_tall",  {-3.7f, 0.0f,-15.0f},  71.0f, {0.7f, 1.0f, 0.8f}, {0.29f, 0.30f, 0.31f, 1.0f}},
        {"rock_small", { 4.0f, 0.0f,-12.2f},  24.0f, {0.9f, 0.8f, 0.9f}, {0.37f, 0.36f, 0.33f, 1.0f}}
    }};

    int rockIndex =
        1;

    for (
        const RockPlacement& rock :
        rocks
    )
    {
        AddEnvironmentEntity(
            scene,
            "Rock_" +
                std::to_string(
                    rockIndex
                ),
            rock.prefab,
            rock.position,
            {
                0.0f,
                rock.rotationY,
                0.0f
            },
            rock.scale,
            rock.color,
            true
        );

        ++rockIndex;
    }

    // =====================================================
    // Building 1: Ruined Shrine
    // =====================================================

    AddEnvironmentEntity(
        scene,
        "Ruined_Shrine_01",
        "ruined_shrine",
        {
            9.0f,
            -0.46f,
            -9.4f
        },
        {
            0.0f,
            -22.0f,
            0.0f
        },
        {
            1.12f,
            1.12f,
            1.12f
        },
        {
            0.31f,
            0.30f,
            0.28f,
            1.0f
        },
        false
    );

    // =====================================================
    // Building 2: Broken Watchtower
    // =====================================================

    AddEnvironmentEntity(
        scene,
        "Broken_Watchtower_01",
        "broken_watchtower",
        {
            -10.4f,
            -0.46f,
            9.8f
        },
        {
            0.0f,
            18.0f,
            0.0f
        },
        {
            1.0f,
            1.0f,
            1.0f
        },
        {
            0.27f,
            0.29f,
            0.30f,
            1.0f
        },
        false
    );

    // =====================================================
    // World pack marker.
    //
    // Invisible and hidden from editor hierarchy so the pack
    // is only generated once and user edits are respected.
    // =====================================================

    Entity& marker =
        scene.CreateEntity(
            WORLD_PACK_MARKER,
            "editor_marker"
        );

    marker.visible =
        false;

    marker.collidable =
        false;

    SDL_Log(
        "RAI World Expansion 0.0.24 applied: larger territory, 22 rocks, 2 structures."
    );

    return true;
}
