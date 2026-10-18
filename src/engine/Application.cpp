#include "Application.h"

#include "engine/physics/CollisionSystem.h"
#include "engine/scene/SceneSerializer.h"

#include "game/EnemySystem.h"
#include "game/GameWorld.h"

#include <glm/glm.hpp>

#include <cmath>

namespace
{
    constexpr const char* WORLD_PATH =
        "assets/worlds/test_world.json";
}

// =========================================================
// Initialize
// =========================================================

bool Application::Initialize()
{
    if (
        !SDL_Init(
            SDL_INIT_VIDEO
        )
    )
    {
        SDL_Log(
            "SDL init failed: %s",
            SDL_GetError()
        );

        return false;
    }

    window =
        SDL_CreateWindow(
            "RAI: Ashes of Eternity | Engine 0.0.15",
            1280,
            720,
            SDL_WINDOW_RESIZABLE
        );

    if (
        window == nullptr
    )
    {
        SDL_Log(
            "Window creation failed: %s",
            SDL_GetError()
        );

        return false;
    }

    if (
        !renderer.Initialize(
            window
        )
    )
    {
        SDL_Log(
            "Renderer initialization failed."
        );

        return false;
    }

    // =====================================================
    // Load Scene
    // =====================================================

    if (
        !SceneSerializer::Load(
            scene,
            WORLD_PATH
        )
    )
    {
        SDL_Log(
            "Could not load world. Creating default world."
        );

        GameWorld::CreateDefaultWorld(
            scene
        );

        SceneSerializer::Save(
            scene,
            WORLD_PATH
        );
    }

    RebuildGameState();

    if (
        player == nullptr
    )
    {
        SDL_Log(
            "Player Entity not found."
        );

        return false;
    }

    camera.target =
        player->transform.position;

    playerVelocity =
        glm::vec3(
            0.0f
        );

    previousTicks =
        SDL_GetTicks();

    running =
        true;

    SDL_Log(
        "RAI Scene ready. Entities: %zu",
        scene.GetEntities().size()
    );

    SDL_Log(
        "Enemies: %zu",
        gameState.enemyIds.size()
    );

    SDL_Log(
        "F5 = Save Scene | F9 = Reload Scene"
    );

    return true;
}

// =========================================================
// Rebuild Game State
// =========================================================

void Application::RebuildGameState()
{
    gameState.Clear();

    player =
        nullptr;

    for (
        Entity& entity :
        scene.GetEntities()
    )
    {
        // -------------------------------------------------
        // Player
        // -------------------------------------------------

        if (
            entity.prefab ==
                "player_placeholder" ||
            entity.name ==
                "Player"
        )
        {
            if (
                gameState.playerId == 0
            )
            {
                gameState.playerId =
                    entity.id;
            }
        }

        // -------------------------------------------------
        // Enemies
        // -------------------------------------------------

        if (
            entity.prefab ==
            "enemy_placeholder"
        )
        {
            gameState.enemyIds.push_back(
                entity.id
            );
        }
    }

    if (
        gameState.playerId != 0
    )
    {
        player =
            scene.FindEntityById(
                gameState.playerId
            );
    }

    SDL_Log(
        "GameState rebuilt. Player=%llu | Enemies=%zu",
        static_cast<unsigned long long>(
            gameState.playerId
        ),
        gameState.enemyIds.size()
    );
}

// =========================================================
// Events
// =========================================================

void Application::ProcessEvents()
{
    SDL_Event event{};

    while (
        SDL_PollEvent(
            &event
        )
    )
    {
        if (
            event.type ==
            SDL_EVENT_QUIT
        )
        {
            running =
                false;
        }

        if (
            event.type ==
            SDL_EVENT_KEY_DOWN
        )
        {
            if (
                event.key.key ==
                SDLK_ESCAPE
            )
            {
                running =
                    false;
            }

            // =================================================
            // Save
            // =================================================

            if (
                event.key.key ==
                SDLK_F5
            )
            {
                SceneSerializer::Save(
                    scene,
                    WORLD_PATH
                );
            }

            // =================================================
            // Reload
            // =================================================

            if (
                event.key.key ==
                SDLK_F9
            )
            {
                if (
                    SceneSerializer::Load(
                        scene,
                        WORLD_PATH
                    )
                )
                {
                    RebuildGameState();

                    playerVelocity =
                        glm::vec3(
                            0.0f
                        );

                    if (
                        player != nullptr
                    )
                    {
                        camera.target =
                            player->transform.position;

                        SDL_Log(
                            "World reloaded successfully."
                        );
                    }
                    else
                    {
                        SDL_Log(
                            "World reloaded, but Player is missing."
                        );
                    }
                }
            }
        }
    }
}

// =========================================================
// Update
// =========================================================

void Application::Update(
    float deltaTime
)
{
    if (
        player == nullptr
    )
    {
        return;
    }

    const bool* keyboard =
        SDL_GetKeyboardState(
            nullptr
        );

    // =====================================================
    // Camera-relative movement
    // =====================================================

    glm::vec3 movementForward{
        -camera.offset.x,
         0.0f,
        -camera.offset.z
    };

    movementForward =
        glm::normalize(
            movementForward
        );

    glm::vec3 movementRight =
        glm::cross(
            glm::vec3(
                0.0f,
                1.0f,
                0.0f
            ),
            movementForward
        );

    movementRight =
        glm::normalize(
            movementRight
        );

    // =====================================================
    // Player Input
    // =====================================================

    glm::vec3 inputDirection{
        0.0f,
        0.0f,
        0.0f
    };

    if (
        keyboard[
            SDL_SCANCODE_W
        ]
    )
    {
        inputDirection +=
            movementForward;
    }

    if (
        keyboard[
            SDL_SCANCODE_S
        ]
    )
    {
        inputDirection -=
            movementForward;
    }

    if (
        keyboard[
            SDL_SCANCODE_D
        ]
    )
    {
        inputDirection +=
            movementRight;
    }

    if (
        keyboard[
            SDL_SCANCODE_A
        ]
    )
    {
        inputDirection -=
            movementRight;
    }

    glm::vec3 desiredVelocity{
        0.0f,
        0.0f,
        0.0f
    };

    const bool hasInput =
        glm::dot(
            inputDirection,
            inputDirection
        ) > 0.0f;

    if (
        hasInput
    )
    {
        inputDirection =
            glm::normalize(
                inputDirection
            );

        desiredVelocity =
            inputDirection *
            playerSpeed;
    }

    // =====================================================
    // Smooth Player Movement
    // =====================================================

    const float smoothingSpeed =
        hasInput
            ? acceleration
            : deceleration;

    const float velocityBlend =
        1.0f -
        std::exp(
            -smoothingSpeed *
            deltaTime
        );

    playerVelocity =
        glm::mix(
            playerVelocity,
            desiredVelocity,
            velocityBlend
        );

    if (
        !hasInput &&
        glm::dot(
            playerVelocity,
            playerVelocity
        ) < 0.0001f
    )
    {
        playerVelocity =
            glm::vec3(
                0.0f
            );
    }

    const glm::vec3 movementDelta =
        playerVelocity *
        deltaTime;

    CollisionSystem::Move(
        scene,
        *player,
        movementDelta
    );

    // =====================================================
    // World Bounds
    // =====================================================

    player->transform.position.x =
        glm::clamp(
            player->transform.position.x,
            -4.0f,
            4.0f
        );

    player->transform.position.z =
        glm::clamp(
            player->transform.position.z,
            -4.0f,
            4.0f
        );

    // =====================================================
    // ENEMY AI
    // =====================================================

    EnemySystem::Update(
        scene,
        gameState,
        deltaTime
    );

    // =====================================================
    // Camera
    // =====================================================

    camera.UpdateFollow(
        player->transform.position,
        deltaTime
    );
}

// =========================================================
// Run
// =========================================================

void Application::Run()
{
    while (
        running
    )
    {
        const Uint64 currentTicks =
            SDL_GetTicks();

        float deltaTime =
            static_cast<float>(
                currentTicks -
                previousTicks
            ) /
            1000.0f;

        previousTicks =
            currentTicks;

        if (
            deltaTime > 0.05f
        )
        {
            deltaTime =
                0.05f;
        }

        ProcessEvents();

        Update(
            deltaTime
        );

        if (
            !renderer.Render(
                scene,
                camera
            )
        )
        {
            running =
                false;
        }
    }
}

// =========================================================
// Shutdown
// =========================================================

void Application::Shutdown()
{
    renderer.Shutdown();

    if (
        window != nullptr
    )
    {
        SDL_DestroyWindow(
            window
        );

        window =
            nullptr;
    }

    SDL_Quit();
}