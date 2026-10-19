#include "Application.h"

#include "engine/physics/CollisionSystem.h"
#include "engine/scene/SceneSerializer.h"
#include "engine/scene/WorldExpansion.h"

#include "game/EnemySystem.h"
#include "game/GameWorld.h"

#include <glm/glm.hpp>

#include <cmath>
#include <cstddef>
#include <string>

namespace
{
    constexpr const char* WORLD_PATH =
        "assets/worlds/test_world.json";

    constexpr const char* GAME_TITLE =
        "RAI: Ashes of Eternity | Engine 0.0.25";

    constexpr const char* EDITOR_TITLE =
        "RAI: Ashes of Eternity | Engine 0.0.25 | EDITOR MODE";
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

    if (
        !editorUI.Initialize(
            window,
            renderer.GetDevice()
        )
    )
    {
        SDL_Log(
            "Editor UI initialization failed."
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

    // =====================================================
    // One-time world expansion
    // =====================================================

    if (
        WorldExpansion::Apply(
            scene
        )
    )
    {
        if (
            !SceneSerializer::Save(
                scene,
                WORLD_PATH
            )
        )
        {
            SDL_Log(
                "WARNING: expanded world could not be saved."
            );
        }
        else
        {
            SDL_Log(
                "Expanded world saved to %s",
                WORLD_PATH
            );
        }
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
// Select Entity By ID
// =========================================================

void Application::SelectEditorEntityById(
    EntityId id
)
{
    ClearEditorSelection();

    Entity* entity =
        scene.FindEntityById(
            id
        );

    if (
        entity == nullptr
    )
    {
        return;
    }

    entity->editorSelected =
        true;

    selectedEditorEntityId =
        entity->id;

    SDL_Log(
        "[EDITOR] Selected: %s | id=%llu | prefab=%s",
        entity->name.c_str(),
        static_cast<unsigned long long>(
            entity->id
        ),
        entity->prefab.c_str()
    );
}

// =========================================================
// Create Object
// =========================================================

void Application::CreateEditorObject()
{
    Entity& object =
        scene.CreateEntity(
            "New_Object",
            "editor_cube"
        );

    object.name =
        "Object_" +
        std::to_string(
            object.id
        );

    Entity* selected =
        GetSelectedEditorEntity();

    if (
        selected != nullptr
    )
    {
        object.transform.position =
            selected->transform.position +
            glm::vec3(
                0.8f,
                0.0f,
                0.0f
            );
    }
    else
    {
        object.transform.position =
            camera.target +
            glm::vec3(
                0.8f,
                0.0f,
                0.0f
            );
    }

    object.transform.rotation =
        glm::vec3(
            0.0f
        );

    object.transform.scale =
        glm::vec3(
            1.0f
        );

    object.color =
    {
        0.42f,
        0.46f,
        0.52f,
        1.0f
    };

    object.visible =
        true;

    object.collidable =
        true;

    SelectEditorEntityById(
        object.id
    );

    SDL_Log(
        "[EDITOR] Created object: %s | id=%llu",
        object.name.c_str(),
        static_cast<unsigned long long>(
            object.id
        )
    );
}

// =========================================================
// Duplicate Object
// =========================================================

void Application::DuplicateSelectedEditorObject()
{
    Entity* source =
        GetSelectedEditorEntity();

    if (
        source == nullptr
    )
    {
        return;
    }

    // Do not duplicate the real player.
    if (
        source->id ==
            gameState.playerId ||
        source->name ==
            "Player"
    )
    {
        SDL_Log(
            "[EDITOR] Player duplication is blocked."
        );

        return;
    }

    // Copy the source data BEFORE creating a new deque item.
    // This avoids depending on reference stability during changes.
    const std::string sourceName =
        source->name;

    const std::string sourcePrefab =
        source->prefab;

    const Transform sourceTransform =
        source->transform;

    const glm::vec4 sourceColor =
        source->color;

    const bool sourceVisible =
        source->visible;

    const bool sourceCollidable =
        source->collidable;

    Entity& copy =
        scene.CreateEntity(
            sourceName + "_Copy",
            sourcePrefab
        );

    copy.name =
        sourceName +
        "_Copy_" +
        std::to_string(
            copy.id
        );

    copy.transform =
        sourceTransform;

    copy.transform.position.x +=
        0.8f;

    copy.color =
        sourceColor;

    copy.visible =
        sourceVisible;

    copy.collidable =
        sourceCollidable;

    SelectEditorEntityById(
        copy.id
    );

    SDL_Log(
        "[EDITOR] Duplicated: %s | id=%llu",
        copy.name.c_str(),
        static_cast<unsigned long long>(
            copy.id
        )
    );
}

// =========================================================
// Delete Object
// =========================================================

void Application::DeleteSelectedEditorObject()
{
    Entity* selected =
        GetSelectedEditorEntity();

    if (
        selected == nullptr
    )
    {
        return;
    }

    if (
        selected->id ==
            gameState.playerId ||
        selected->name ==
            "Player"
    )
    {
        SDL_Log(
            "[EDITOR] Player deletion is blocked."
        );

        return;
    }

    const EntityId deletedId =
        selected->id;

    const std::string deletedName =
        selected->name;

    ClearEditorSelection();

    if (
        !scene.DestroyEntity(
            deletedId
        )
    )
    {
        SDL_Log(
            "[EDITOR] Delete failed: id=%llu",
            static_cast<unsigned long long>(
                deletedId
            )
        );

        return;
    }

    // deque erase can invalidate stored pointers,
    // so reacquire Player immediately.
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
        "[EDITOR] Deleted: %s | id=%llu",
        deletedName.c_str(),
        static_cast<unsigned long long>(
            deletedId
        )
    );

    SelectEditorEntity(
        false
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
            "W / E / R = Move / Rotate / Scale gizmo"
        );

        SDL_Log(
            "+ / - = scale"
        );

        SDL_Log(
            "N = create cube"
        );

        SDL_Log(
            "CTRL/CMD + D = duplicate selected"
        );

        SDL_Log(
            "DELETE/BACKSPACE = delete selected"
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

    RebuildGameState();

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
        editorUI.ProcessEvent(
            event
        );

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
        // ImGui keyboard capture
        //
        // F1/F5/F9 above/below remain engine commands, but
        // editing text/numbers in Inspector must not trigger
        // editor object hotkeys.
        // =================================================

        if (
            editorMode &&
            editorUI.WantsKeyboardCapture() &&
            event.key.key != SDLK_F5 &&
            event.key.key != SDLK_F9
        )
        {
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
        // N - Create Object
        // =================================================

        if (
            editorMode &&
            event.key.scancode ==
                SDL_SCANCODE_N
        )
        {
            CreateEditorObject();

            continue;
        }

        // =================================================
        // CTRL/CMD + D - Duplicate
        // =================================================

        if (
            editorMode &&
            event.key.scancode ==
                SDL_SCANCODE_D
        )
        {
            const SDL_Keymod modifiers =
                SDL_GetModState();

            const bool duplicateModifier =
                (
                    modifiers &
                    SDL_KMOD_CTRL
                ) != 0 ||
                (
                    modifiers &
                    SDL_KMOD_GUI
                ) != 0;

            if (
                duplicateModifier
            )
            {
                DuplicateSelectedEditorObject();

                continue;
            }
        }

        // =================================================
        // DELETE / BACKSPACE - Delete
        // =================================================

        if (
            editorMode &&
            (
                event.key.scancode ==
                    SDL_SCANCODE_DELETE ||
                event.key.scancode ==
                    SDL_SCANCODE_BACKSPACE
            )
        )
        {
            DeleteSelectedEditorObject();

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
            -17.0f,
            17.0f
        );

    player->transform.position.z =
        glm::clamp(
            player->transform.position.z,
            -17.0f,
            17.0f
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
    if (
        editorUI.WantsKeyboardCapture()
    )
    {
        return;
    }

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
    //
    // W / E / R are now reserved for transform gizmos.
    // Rotation is performed through the gizmo or Inspector.
    // =====================================================

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

        ImDrawData* uiDrawData =
            nullptr;

        if (
            editorMode
        )
        {
            editorUI.BeginFrame();

            editorUI.Draw(
                scene,
                selectedEditorEntityId,
                camera
            );

            editorUI.EndFrame();

            uiDrawData =
                editorUI.GetDrawData();
        }

        if (
            !renderer.Render(
                scene,
                camera,
                uiDrawData
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
    editorUI.Shutdown();

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
