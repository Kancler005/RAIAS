#include "Application.h"

#include "engine/physics/CollisionSystem.h"
#include "engine/scene/SceneSerializer.h"

#include "game/EnemySystem.h"
#include "game/GameWorld.h"

#include <glm/glm.hpp>

#include <cmath>
#include <cstddef>

namespace
{
    constexpr const char* WORLD_PATH =
        "assets/worlds/test_world.json";

    constexpr const char* GAME_TITLE =
        "RAI: Ashes of Eternity | Engine 0.0.18";

    constexpr const char* EDITOR_TITLE =
        "RAI: Ashes of Eternity | Engine 0.0.18 | EDITOR MODE";
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
            GAME_TITLE,
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
        "F1 = Editor Mode | F5 = Save | F9 = Reload"
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
        entity.editorSelected =
            false;

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
// Editor Selection
// =========================================================

void Application::ClearEditorSelection()
{
    for (
        Entity& entity :
        scene.GetEntities()
    )
    {
        entity.editorSelected =
            false;
    }

    selectedEditorEntityId =
        0;
}

Entity* Application::GetSelectedEditorEntity()
{
    if (
        selectedEditorEntityId == 0
    )
    {
        return nullptr;
    }

    return scene.FindEntityById(
        selectedEditorEntityId
    );
}

void Application::SelectEditorEntity(
    bool reverse
)
{
    auto& entities =
        scene.GetEntities();

    if (
        entities.empty()
    )
    {
        ClearEditorSelection();

        return;
    }

    std::size_t currentIndex =
        0;

    bool currentFound =
        false;

    for (
        std::size_t i = 0;
        i < entities.size();
        ++i
    )
    {
        if (
            entities[i].id ==
            selectedEditorEntityId
        )
        {
            currentIndex =
                i;

            currentFound =
                true;

            break;
        }
    }

    std::size_t nextIndex =
        0;

    if (
        !currentFound
    )
    {
        nextIndex =
            reverse
                ? entities.size() - 1
                : 0;
    }
    else if (
        reverse
    )
    {
        nextIndex =
            currentIndex == 0
                ? entities.size() - 1
                : currentIndex - 1;
    }
    else
    {
        nextIndex =
            (
                currentIndex + 1
            ) %
            entities.size();
    }

    ClearEditorSelection();

    Entity& selected =
        entities[
            nextIndex
        ];

    selected.editorSelected =
        true;

    selectedEditorEntityId =
        selected.id;

    SDL_Log(
        "[EDITOR] Selected: %s | id=%llu | prefab=%s",
        selected.name.c_str(),
        static_cast<unsigned long long>(
            selected.id
        ),
        selected.prefab.c_str()
    );
}

// =========================================================
// Toggle Editor
// =========================================================

void Application::ToggleEditorMode()
{
    editorMode =
        !editorMode;

    playerVelocity =
        glm::vec3(
            0.0f
        );

    if (
        editorMode
    )
    {
        SDL_SetWindowTitle(
            window,
            EDITOR_TITLE
        );

        ClearEditorSelection();

        SelectEditorEntity(
            false
        );

        SDL_Log(
            "=========================================="
        );

        SDL_Log(
            "RAI EDITOR MODE ENABLED"
        );

        SDL_Log(
            "TAB / SHIFT+TAB = select entity"
        );

        SDL_Log(
            "ARROWS = move on ground"
        );

        SDL_Log(
            "PAGE UP / PAGE DOWN = move vertically"
        );

        SDL_Log(
            "Q / E = rotate Y"
        );

        SDL_Log(
            "+ / - = scale"
        );

        SDL_Log(
            "F5 = save world"
        );

        SDL_Log(
            "F9 = reload world"
        );

        SDL_Log(
            "F1 = return to gameplay"
        );

        SDL_Log(
            "=========================================="
        );

        return;
    }

    ClearEditorSelection();

    SDL_SetWindowTitle(
        window,
        GAME_TITLE
    );

    SDL_Log(
        "RAI EDITOR MODE DISABLED"
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
            event.type !=
            SDL_EVENT_KEY_DOWN
        )
        {
            continue;
        }

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

            continue;
        }

        // =================================================
        // F1 - Editor Mode
        // =================================================

        if (
            event.key.key ==
            SDLK_F1
        )
        {
            ToggleEditorMode();

            continue;
        }

        // =================================================
        // TAB - Selection
        // =================================================

        if (
            editorMode &&
            event.key.key ==
                SDLK_TAB
        )
        {
            const SDL_Keymod modifiers =
                SDL_GetModState();

            const bool reverse =
                (
                    modifiers &
                    SDL_KMOD_SHIFT
                ) != 0;

            SelectEditorEntity(
                reverse
            );

            continue;
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

            SDL_Log(
                editorMode
                    ? "[EDITOR] World saved."
                    : "World saved."
            );

            continue;
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
                selectedEditorEntityId =
                    0;

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
                }

                if (
                    editorMode
                )
                {
                    SelectEditorEntity(
                        false
                    );
                }

                SDL_Log(
                    "World reloaded successfully."
                );
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
        editorMode
    )
    {
        UpdateEditor(
            deltaTime
        );

        return;
    }

    UpdateGameplay(
        deltaTime
    );
}

// =========================================================
// Gameplay Update
// =========================================================

void Application::UpdateGameplay(
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
    // Temporary World Bounds
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
    // Enemy AI
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
// Editor Update
// =========================================================

void Application::UpdateEditor(
    float deltaTime
)
{
    Entity* selected =
        GetSelectedEditorEntity();

    if (
        selected == nullptr
    )
    {
        SelectEditorEntity(
            false
        );

        selected =
            GetSelectedEditorEntity();

        if (
            selected == nullptr
        )
        {
            return;
        }
    }

    const bool* keyboard =
        SDL_GetKeyboardState(
            nullptr
        );

    // =====================================================
    // Camera-relative editor movement
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

    glm::vec3 movement{
        0.0f
    };

    if (
        keyboard[
            SDL_SCANCODE_UP
        ]
    )
    {
        movement +=
            movementForward;
    }

    if (
        keyboard[
            SDL_SCANCODE_DOWN
        ]
    )
    {
        movement -=
            movementForward;
    }

    if (
        keyboard[
            SDL_SCANCODE_RIGHT
        ]
    )
    {
        movement +=
            movementRight;
    }

    if (
        keyboard[
            SDL_SCANCODE_LEFT
        ]
    )
    {
        movement -=
            movementRight;
    }

    if (
        glm::dot(
            movement,
            movement
        ) > 0.0001f
    )
    {
        movement =
            glm::normalize(
                movement
            );

        selected->transform.position +=
            movement *
            editorMoveSpeed *
            deltaTime;
    }

    // =====================================================
    // Vertical Movement
    // =====================================================

    if (
        keyboard[
            SDL_SCANCODE_PAGEUP
        ]
    )
    {
        selected->transform.position.y +=
            editorVerticalSpeed *
            deltaTime;
    }

    if (
        keyboard[
            SDL_SCANCODE_PAGEDOWN
        ]
    )
    {
        selected->transform.position.y -=
            editorVerticalSpeed *
            deltaTime;
    }

    // =====================================================
    // Rotation
    // =====================================================

    if (
        keyboard[
            SDL_SCANCODE_Q
        ]
    )
    {
        selected->transform.rotation.y -=
            editorRotationSpeed *
            deltaTime;
    }

    if (
        keyboard[
            SDL_SCANCODE_E
        ]
    )
    {
        selected->transform.rotation.y +=
            editorRotationSpeed *
            deltaTime;
    }

    // =====================================================
    // Uniform Scale
    //
    // + can be SHIFT + '=' on normal keyboards.
    // SDL_SCANCODE_EQUALS therefore covers both.
    // =====================================================

    bool scaleUp =
        keyboard[
            SDL_SCANCODE_EQUALS
        ];

    bool scaleDown =
        keyboard[
            SDL_SCANCODE_MINUS
        ];

#ifdef SDL_SCANCODE_KP_PLUS
    scaleUp =
        scaleUp ||
        keyboard[
            SDL_SCANCODE_KP_PLUS
        ];
#endif

#ifdef SDL_SCANCODE_KP_MINUS
    scaleDown =
        scaleDown ||
        keyboard[
            SDL_SCANCODE_KP_MINUS
        ];
#endif

    if (
        scaleUp
    )
    {
        const float factor =
            1.0f +
            editorScaleSpeed *
            deltaTime;

        selected->transform.scale *=
            factor;
    }

    if (
        scaleDown
    )
    {
        const float factor =
            glm::max(
                0.01f,
                1.0f -
                editorScaleSpeed *
                deltaTime
            );

        selected->transform.scale *=
            factor;
    }

    selected->transform.scale =
        glm::max(
            selected->transform.scale,
            glm::vec3(
                0.05f
            )
        );

    // Gameplay simulation is intentionally paused in editor mode.
    // Camera stays where the player left it, so moving an object
    // does not drag the whole view around.
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
