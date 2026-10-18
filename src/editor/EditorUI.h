#pragma once

#include "engine/scene/Entity.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

class Camera;
class Scene;
struct ImDrawData;

enum class EditorGizmoOperation
{
    Move,
    Rotate,
    Scale
};

class EditorUI
{
public:
    bool Initialize(
        SDL_Window* window,
        SDL_GPUDevice* device
    );

    void Shutdown();

    void ProcessEvent(
        const SDL_Event& event
    );

    void BeginFrame();

    void Draw(
        Scene& scene,
        EntityId& selectedEntityId,
        const Camera& camera
    );

    void EndFrame();

    [[nodiscard]]
    ImDrawData* GetDrawData() const;

    [[nodiscard]]
    bool WantsKeyboardCapture() const;

    [[nodiscard]]
    bool WantsMouseCapture() const;

private:
    void DrawHierarchy(
        Scene& scene,
        EntityId& selectedEntityId
    );

    void DrawInspector(
        Scene& scene,
        EntityId selectedEntityId
    );

    void DrawViewport(
        Scene& scene,
        EntityId selectedEntityId,
        const Camera& camera
    );

    void DrawViewportToolbar(
        const Entity* selectedEntity
    );

    void HandleGizmoShortcuts();

    static void ApplySelection(
        Scene& scene,
        EntityId selectedEntityId
    );

private:
    SDL_Window* window =
        nullptr;

    SDL_GPUDevice* device =
        nullptr;

    ImDrawData* drawData =
        nullptr;

    bool initialized =
        false;

    bool frameStarted =
        false;

    EditorGizmoOperation gizmoOperation =
        EditorGizmoOperation::Move;

    bool localGizmo =
        false;
};
