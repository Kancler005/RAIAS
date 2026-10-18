#include "GameWorld.h"

#include "engine/scene/Scene.h"

void GameWorld::CreateDefaultWorld(
    Scene& scene
)
{
    Entity& ground =
        scene.CreateEntity(
            "Ground",
            "ground_block"
        );

    ground.transform.position =
    {
        0.0f,
        -0.65f,
        0.0f
    };

    ground.transform.scale =
    {
        9.0f,
        0.3f,
        9.0f
    };

    ground.color =
    {
        0.10f,
        0.11f,
        0.13f,
        1.0f
    };

    ground.collidable =
        false;

    // =====================================================
    // Player
    // =====================================================

    Entity& player =
        scene.CreateEntity(
            "Player",
            "player_placeholder"
        );

    player.transform.position =
    {
        0.0f,
        0.0f,
        0.0f
    };

    player.transform.scale =
    {
        0.7f,
        1.0f,
        0.7f
    };

    player.color =
    {
        0.75f,
        0.16f,
        0.10f,
        1.0f
    };

    player.collidable =
        false;

    // =====================================================
    // Pillar 01
    // =====================================================

    Entity& pillar01 =
        scene.CreateEntity(
            "Pillar_01",
            "stone_pillar"
        );

    pillar01.transform.position =
    {
        -2.2f,
         0.5f,
         1.6f
    };

    pillar01.transform.scale =
    {
        0.65f,
        2.0f,
        0.65f
    };

    pillar01.color =
    {
        0.30f,
        0.31f,
        0.34f,
        1.0f
    };

    pillar01.collidable =
        true;

    // =====================================================
    // Pillar 02
    // =====================================================

    Entity& pillar02 =
        scene.CreateEntity(
            "Pillar_02",
            "stone_pillar"
        );

    pillar02.transform.position =
    {
         2.1f,
         0.25f,
        -1.7f
    };

    pillar02.transform.scale =
    {
        0.9f,
        1.5f,
        0.9f
    };

    pillar02.color =
    {
        0.25f,
        0.27f,
        0.31f,
        1.0f
    };

    pillar02.collidable =
        true;

    // =====================================================
    // Wall
    // =====================================================

    Entity& wall =
        scene.CreateEntity(
            "Wall_01",
            "stone_wall"
        );

    wall.transform.position =
    {
        0.0f,
        0.0f,
        2.8f
    };

    wall.transform.scale =
    {
        4.0f,
        1.0f,
        0.45f
    };

    wall.color =
    {
        0.22f,
        0.23f,
        0.26f,
        1.0f
    };

    wall.collidable =
        true;

    // =====================================================
    // Ruin 01
    // =====================================================

    Entity& ruin01 =
        scene.CreateEntity(
            "Ruin_01",
            "ruin_block"
        );

    ruin01.transform.position =
    {
        -2.8f,
        -0.20f,
        -2.4f
    };

    ruin01.transform.rotation =
    {
        0.0f,
        20.0f,
        0.0f
    };

    ruin01.transform.scale =
    {
        1.4f,
        0.6f,
        1.0f
    };

    ruin01.color =
    {
        0.18f,
        0.20f,
        0.24f,
        1.0f
    };

    ruin01.collidable =
        true;

    // =====================================================
    // Ruin 02
    // =====================================================

    Entity& ruin02 =
        scene.CreateEntity(
            "Ruin_02",
            "ruin_block"
        );

    ruin02.transform.position =
    {
        2.8f,
        -0.10f,
        2.0f
    };

    ruin02.transform.rotation =
    {
        0.0f,
        -25.0f,
        0.0f
    };

    ruin02.transform.scale =
    {
        1.2f,
        0.8f,
        1.2f
    };

    ruin02.color =
    {
        0.28f,
        0.22f,
        0.20f,
        1.0f
    };

    ruin02.collidable =
        true;
}