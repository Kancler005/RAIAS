#include "Application.h"

#include "engine/physics/CollisionSystem.h"
#include "engine/scene/SceneSerializer.h"
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
            "RAI: Ashes of Eternity | Engine 0.0.13.1",
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
    // Load World
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

    // =====================================================
    // Player
    // =====================================================

    player =
        scene.FindEntity(
            "Player"
        );

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
        "F5 = Save Scene | F9 = Reload Scene"
    );

    return true;
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
            // =================================================
            // ESC
            // =================================================

            if (
                event.key.key ==
                SDLK_ESCAPE
            )
            {
                running =
                    false;
            }

            // =================================================
            // F5 - Save
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
            // F9 - Reload
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
                    player =
                        scene.FindEntity(
                            "Player"
                        );

                    playerVelocity =
                        glm::vec3(
                            0.0f
                        );

                    if (
                        player != nullptr
                    )
                    {
                        camera.target =
                            player
                                ->transform
                                .position;

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
    // Camera-relative directions
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
    // Input
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

    // =====================================================
    // Desired velocity
    // =====================================================

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
    // Smooth acceleration / deceleration
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

    // Stop tiny floating-point movement.

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

    // =====================================================
    // Move with collision
    // =====================================================

    const glm::vec3 movementDelta =
        playerVelocity *
        deltaTime;

    CollisionSystem::Move(
        scene,
        *player,
        movementDelta
    );

    // =====================================================
    // Temporary world boundaries
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
    // Smooth Camera
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