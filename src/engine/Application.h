#pragma once

#include "Camera.h"
#include "Renderer.h"
#include "scene/Scene.h"

#include "game/GameState.h"

#include <SDL3/SDL.h>

#include <glm/glm.hpp>

class Application
{
public:
    bool Initialize();

    void Run();

    void Shutdown();

private:
    void ProcessEvents();

    void Update(
        float deltaTime
    );

    void UpdateGameplay(
        float deltaTime
    );

    void UpdateEditor(
        float deltaTime
    );

    void RebuildGameState();

    void ToggleEditorMode();

    void ClearEditorSelection();

    void SelectEditorEntity(
        bool reverse
    );

    Entity* GetSelectedEditorEntity();

private:
    SDL_Window* window =
        nullptr;

    Renderer renderer;

    Scene scene;

    Camera camera;

    GameState gameState;

    Entity* player =
        nullptr;

    bool running =
        false;

    Uint64 previousTicks =
        0;

    // =====================================================
    // Player movement
    // =====================================================

    glm::vec3 playerVelocity{
        0.0f,
        0.0f,
        0.0f
    };

    float playerSpeed =
        2.6f;

    float acceleration =
        8.0f;

    float deceleration =
        10.0f;

    // =====================================================
    // Editor
    // =====================================================

    bool editorMode =
        false;

    EntityId selectedEditorEntityId =
        0;

    float editorMoveSpeed =
        2.0f;

    float editorVerticalSpeed =
        1.5f;

    float editorRotationSpeed =
        90.0f;

    float editorScaleSpeed =
        1.0f;
};
