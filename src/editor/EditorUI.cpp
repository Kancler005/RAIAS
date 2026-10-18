#include "EditorUI.h"

#include "engine/Camera.h"
#include "engine/scene/Scene.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include <ImGuizmo.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cstdio>

// =========================================================
// Constants
// =========================================================

namespace
{
    constexpr float HIERARCHY_WIDTH =
        285.0f;

    constexpr float INSPECTOR_WIDTH =
        360.0f;

    constexpr float VIEWPORT_TOOLBAR_MARGIN =
        12.0f;
}

// =========================================================
// Initialize
// =========================================================

bool EditorUI::Initialize(
    SDL_Window* newWindow,
    SDL_GPUDevice* newDevice
)
{
    window =
        newWindow;

    device =
        newDevice;

    if (
        window == nullptr ||
        device == nullptr
    )
    {
        SDL_Log(
            "EditorUI initialization failed: invalid window/device."
        );

        return false;
    }

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGuiIO& io =
        ImGui::GetIO();

    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard;

    // =====================================================
    // Style
    // =====================================================

    ImGui::StyleColorsDark();

    ImGuiStyle& style =
        ImGui::GetStyle();

    style.WindowRounding =
        4.0f;

    style.FrameRounding =
        3.0f;

    style.GrabRounding =
        3.0f;

    style.WindowBorderSize =
        1.0f;

    style.FrameBorderSize =
        0.0f;

    const float displayScale =
        SDL_GetDisplayContentScale(
            SDL_GetPrimaryDisplay()
        );

    if (
        displayScale > 0.0f
    )
    {
        style.ScaleAllSizes(
            displayScale
        );

        style.FontScaleDpi =
            displayScale;
    }

    // =====================================================
    // SDL3 Platform Backend
    // =====================================================

    if (
        !ImGui_ImplSDL3_InitForSDLGPU(
            window
        )
    )
    {
        SDL_Log(
            "ImGui SDL3 backend initialization failed."
        );

        ImGui::DestroyContext();

        return false;
    }

    // =====================================================
    // SDL_GPU Renderer Backend
    // =====================================================

    ImGui_ImplSDLGPU3_InitInfo
        initInfo{};

    initInfo.Device =
        device;

    initInfo.ColorTargetFormat =
        SDL_GetGPUSwapchainTextureFormat(
            device,
            window
        );

    initInfo.MSAASamples =
        SDL_GPU_SAMPLECOUNT_1;

    initInfo.SwapchainComposition =
        SDL_GPU_SWAPCHAINCOMPOSITION_SDR;

    initInfo.PresentMode =
        SDL_GPU_PRESENTMODE_VSYNC;

    if (
        !ImGui_ImplSDLGPU3_Init(
            &initInfo
        )
    )
    {
        SDL_Log(
            "ImGui SDL_GPU backend initialization failed."
        );

        ImGui_ImplSDL3_Shutdown();

        ImGui::DestroyContext();

        return false;
    }

    initialized =
        true;

    SDL_Log(
        "RAI Editor UI initialized."
    );

    return true;
}

// =========================================================
// Shutdown
// =========================================================

void EditorUI::Shutdown()
{
    if (
        !initialized
    )
    {
        return;
    }

    if (
        device != nullptr
    )
    {
        SDL_WaitForGPUIdle(
            device
        );
    }

    ImGui_ImplSDLGPU3_Shutdown();

    ImGui_ImplSDL3_Shutdown();

    ImGui::DestroyContext();

    drawData =
        nullptr;

    window =
        nullptr;

    device =
        nullptr;

    initialized =
        false;

    frameStarted =
        false;
}

// =========================================================
// Events
// =========================================================

void EditorUI::ProcessEvent(
    const SDL_Event& event
)
{
    if (
        !initialized
    )
    {
        return;
    }

    ImGui_ImplSDL3_ProcessEvent(
        &event
    );
}

// =========================================================
// Frame
// =========================================================

void EditorUI::BeginFrame()
{
    if (
        !initialized
    )
    {
        return;
    }

    ImGui_ImplSDLGPU3_NewFrame();

    ImGui_ImplSDL3_NewFrame();

    ImGui::NewFrame();

    // ImGuizmo expects this once for every ImGui frame.
    ImGuizmo::BeginFrame();

    drawData =
        nullptr;

    frameStarted =
        true;
}

void EditorUI::EndFrame()
{
    if (
        !initialized ||
        !frameStarted
    )
    {
        return;
    }

    ImGui::Render();

    drawData =
        ImGui::GetDrawData();

    frameStarted =
        false;
}

ImDrawData* EditorUI::GetDrawData() const
{
    return drawData;
}

// =========================================================
// Input Capture
// =========================================================

bool EditorUI::WantsKeyboardCapture() const
{
    if (
        !initialized
    )
    {
        return false;
    }

    return ImGui::GetIO()
        .WantCaptureKeyboard;
}

bool EditorUI::WantsMouseCapture() const
{
    if (
        !initialized
    )
    {
        return false;
    }

    return ImGui::GetIO()
        .WantCaptureMouse;
}

// =========================================================
// Main Editor UI
// =========================================================

void EditorUI::Draw(
    Scene& scene,
    EntityId& selectedEntityId,
    const Camera& camera
)
{
    if (
        !initialized ||
        !frameStarted
    )
    {
        return;
    }

    ApplySelection(
        scene,
        selectedEntityId
    );

    HandleGizmoShortcuts();

    DrawHierarchy(
        scene,
        selectedEntityId
    );

    DrawInspector(
        scene,
        selectedEntityId
    );

    DrawViewport(
        scene,
        selectedEntityId,
        camera
    );
}

// =========================================================
// Hierarchy
// =========================================================

void EditorUI::DrawHierarchy(
    Scene& scene,
    EntityId& selectedEntityId
)
{
    ImGuiViewport* viewport =
        ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(
        viewport->WorkPos,
        ImGuiCond_Always
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            HIERARCHY_WIDTH,
            viewport->WorkSize.y
        ),
        ImGuiCond_Always
    );

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;

    ImGui::Begin(
        "Hierarchy",
        nullptr,
        flags
    );

    ImGui::TextUnformatted(
        "RAI WORLD"
    );

    ImGui::Separator();

    for (
        Entity& entity :
        scene.GetEntities()
    )
    {
        const bool selected =
            entity.id ==
            selectedEntityId;

        char label[256];

        std::snprintf(
            label,
            sizeof(label),
            "%s  [#%llu]",
            entity.name.c_str(),
            static_cast<unsigned long long>(
                entity.id
            )
        );

        ImGui::PushID(
            static_cast<int>(
                entity.id &
                0x7FFFFFFF
            )
        );

        if (
            ImGui::Selectable(
                label,
                selected
            )
        )
        {
            selectedEntityId =
                entity.id;

            ApplySelection(
                scene,
                selectedEntityId
            );
        }

        if (
            ImGui::IsItemHovered()
        )
        {
            ImGui::SetTooltip(
                "Prefab: %s",
                entity.prefab.c_str()
            );
        }

        ImGui::PopID();
    }

    ImGui::Separator();

    ImGui::TextDisabled(
        "N: Create | Cmd/Ctrl+D: Duplicate"
    );

    ImGui::TextDisabled(
        "Delete/Backspace: Delete | F5: Save"
    );

    ImGui::End();
}

// =========================================================
// Inspector
// =========================================================

void EditorUI::DrawInspector(
    Scene& scene,
    EntityId selectedEntityId
)
{
    ImGuiViewport* viewport =
        ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(
        ImVec2(
            viewport->WorkPos.x +
                viewport->WorkSize.x -
                INSPECTOR_WIDTH,
            viewport->WorkPos.y
        ),
        ImGuiCond_Always
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            INSPECTOR_WIDTH,
            viewport->WorkSize.y
        ),
        ImGuiCond_Always
    );

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;

    ImGui::Begin(
        "Inspector",
        nullptr,
        flags
    );

    Entity* entity =
        scene.FindEntityById(
            selectedEntityId
        );

    if (
        entity == nullptr
    )
    {
        ImGui::TextDisabled(
            "No entity selected."
        );

        ImGui::End();

        return;
    }

    ImGui::Text(
        "Entity #%llu",
        static_cast<unsigned long long>(
            entity->id
        )
    );

    ImGui::Separator();

    // =====================================================
    // Name
    // =====================================================

    char nameBuffer[192]{};

    std::snprintf(
        nameBuffer,
        sizeof(nameBuffer),
        "%s",
        entity->name.c_str()
    );

    if (
        ImGui::InputText(
            "Name",
            nameBuffer,
            sizeof(nameBuffer)
        )
    )
    {
        entity->name =
            nameBuffer;
    }

    ImGui::Text(
        "Prefab: %s",
        entity->prefab.c_str()
    );

    ImGui::SeparatorText(
        "Transform"
    );

    ImGui::DragFloat3(
        "Position",
        &entity->transform.position.x,
        0.05f
    );

    ImGui::DragFloat3(
        "Rotation",
        &entity->transform.rotation.x,
        0.5f
    );

    if (
        ImGui::DragFloat3(
            "Scale",
            &entity->transform.scale.x,
            0.02f,
            0.05f,
            100.0f
        )
    )
    {
        entity->transform.scale =
            glm::max(
                entity->transform.scale,
                glm::vec3(
                    0.05f
                )
            );
    }

    ImGui::SeparatorText(
        "Rendering"
    );

    ImGui::ColorEdit4(
        "Color",
        &entity->color.x
    );

    ImGui::Checkbox(
        "Visible",
        &entity->visible
    );

    ImGui::SeparatorText(
        "Physics"
    );

    ImGui::Checkbox(
        "Collidable",
        &entity->collidable
    );

    ImGui::Spacing();

    ImGui::Separator();

    ImGui::TextDisabled(
        "W Move | E Rotate | R Scale"
    );

    ImGui::TextDisabled(
        "Changes are live. F5 saves Scene JSON."
    );

    ImGui::End();
}

// =========================================================
// Gizmo Shortcuts
// =========================================================

void EditorUI::HandleGizmoShortcuts()
{
    ImGuiIO& io =
        ImGui::GetIO();

    // Do not steal letters while typing a Name or editing a widget.
    if (
        io.WantTextInput ||
        ImGui::IsAnyItemActive()
    )
    {
        return;
    }

    if (
        ImGui::IsKeyPressed(
            ImGuiKey_W,
            false
        )
    )
    {
        gizmoOperation =
            EditorGizmoOperation::Move;
    }

    if (
        ImGui::IsKeyPressed(
            ImGuiKey_E,
            false
        )
    )
    {
        gizmoOperation =
            EditorGizmoOperation::Rotate;
    }

    if (
        ImGui::IsKeyPressed(
            ImGuiKey_R,
            false
        )
    )
    {
        gizmoOperation =
            EditorGizmoOperation::Scale;
    }
}

// =========================================================
// Viewport Toolbar
// =========================================================

void EditorUI::DrawViewportToolbar(
    const Entity* selectedEntity
)
{
    ImGuiViewport* viewport =
        ImGui::GetMainViewport();

    const float left =
        viewport->WorkPos.x +
        HIERARCHY_WIDTH +
        VIEWPORT_TOOLBAR_MARGIN;

    const float top =
        viewport->WorkPos.y +
        VIEWPORT_TOOLBAR_MARGIN;

    ImGui::SetNextWindowPos(
        ImVec2(
            left,
            top
        ),
        ImGuiCond_Always
    );

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin(
        "##ViewportToolbar",
        nullptr,
        flags
    );

    const bool moveSelected =
        gizmoOperation ==
        EditorGizmoOperation::Move;

    const bool rotateSelected =
        gizmoOperation ==
        EditorGizmoOperation::Rotate;

    const bool scaleSelected =
        gizmoOperation ==
        EditorGizmoOperation::Scale;

    if (
        moveSelected
    )
    {
        ImGui::PushStyleColor(
            ImGuiCol_Button,
            ImVec4(
                0.55f,
                0.34f,
                0.10f,
                1.0f
            )
        );
    }

    if (
        ImGui::Button(
            "W  Move"
        )
    )
    {
        gizmoOperation =
            EditorGizmoOperation::Move;
    }

    if (
        moveSelected
    )
    {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    if (
        rotateSelected
    )
    {
        ImGui::PushStyleColor(
            ImGuiCol_Button,
            ImVec4(
                0.55f,
                0.34f,
                0.10f,
                1.0f
            )
        );
    }

    if (
        ImGui::Button(
            "E  Rotate"
        )
    )
    {
        gizmoOperation =
            EditorGizmoOperation::Rotate;
    }

    if (
        rotateSelected
    )
    {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    if (
        scaleSelected
    )
    {
        ImGui::PushStyleColor(
            ImGuiCol_Button,
            ImVec4(
                0.55f,
                0.34f,
                0.10f,
                1.0f
            )
        );
    }

    if (
        ImGui::Button(
            "R  Scale"
        )
    )
    {
        gizmoOperation =
            EditorGizmoOperation::Scale;
    }

    if (
        scaleSelected
    )
    {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    if (
        ImGui::Button(
            localGizmo
                ? "Local"
                : "World"
        )
    )
    {
        localGizmo =
            !localGizmo;
    }

    if (
        selectedEntity != nullptr
    )
    {
        ImGui::SameLine();

        ImGui::TextDisabled(
            "| %s",
            selectedEntity->name.c_str()
        );
    }

    ImGui::End();
}

// =========================================================
// Viewport + Transform Gizmo
// =========================================================

void EditorUI::DrawViewport(
    Scene& scene,
    EntityId selectedEntityId,
    const Camera& camera
)
{
    ImGuiViewport* viewport =
        ImGui::GetMainViewport();

    Entity* entity =
        scene.FindEntityById(
            selectedEntityId
        );

    DrawViewportToolbar(
        entity
    );

    // =====================================================
    // Visible central editor viewport border.
    //
    // The game still renders the world to the whole swapchain.
    // Side panels cover the outer areas. This milestone makes
    // the central editing region explicit without duplicating
    // the world renderer.
    // =====================================================

    const float viewportLeft =
        viewport->WorkPos.x +
        HIERARCHY_WIDTH;

    const float viewportRight =
        viewport->WorkPos.x +
        viewport->WorkSize.x -
        INSPECTOR_WIDTH;

    const float viewportTop =
        viewport->WorkPos.y;

    const float viewportBottom =
        viewport->WorkPos.y +
        viewport->WorkSize.y;

    ImDrawList* foreground =
        ImGui::GetForegroundDrawList();

    foreground->AddRect(
        ImVec2(
            viewportLeft,
            viewportTop
        ),
        ImVec2(
            viewportRight,
            viewportBottom
        ),
        IM_COL32(
            95,
            95,
            105,
            255
        ),
        0.0f,
        0,
        1.0f
    );

    if (
        entity == nullptr
    )
    {
        return;
    }

    // =====================================================
    // Camera matrices
    //
    // Renderer uses full-window aspect, so the gizmo must use
    // the same projection to line up with the actual world.
    // =====================================================

    const float fullAspect =
        viewport->WorkSize.x /
        glm::max(
            viewport->WorkSize.y,
            1.0f
        );

    const glm::vec3 cameraPosition =
        camera.target +
        camera.offset;

    const glm::mat4 view =
        glm::lookAtLH(
            cameraPosition,
            camera.target,
            glm::vec3(
                0.0f,
                1.0f,
                0.0f
            )
        );

    const float halfWidth =
        camera.halfHeight *
        fullAspect;

    const glm::mat4 projection =
        glm::orthoLH_ZO(
            -halfWidth,
             halfWidth,
            -camera.halfHeight,
             camera.halfHeight,
             0.1f,
             100.0f
        );

    glm::mat4 model =
        entity->transform.GetMatrix();

    // =====================================================
    // ImGuizmo
    // =====================================================

    ImGuizmo::SetDrawlist(
        foreground
    );

    ImGuizmo::SetOrthographic(
        true
    );

    // Use the full rendered area for coordinate mapping.
    // The left/right editor panels simply cover the outer world.
    ImGuizmo::SetRect(
        viewport->WorkPos.x,
        viewport->WorkPos.y,
        viewport->WorkSize.x,
        viewport->WorkSize.y
    );

    ImGuizmo::OPERATION operation =
        ImGuizmo::TRANSLATE;

    switch (
        gizmoOperation
    )
    {
        case EditorGizmoOperation::Move:
        {
            operation =
                ImGuizmo::TRANSLATE;

            break;
        }

        case EditorGizmoOperation::Rotate:
        {
            operation =
                ImGuizmo::ROTATE;

            break;
        }

        case EditorGizmoOperation::Scale:
        {
            operation =
                ImGuizmo::SCALE;

            break;
        }
    }

    const ImGuizmo::MODE mode =
        localGizmo
            ? ImGuizmo::LOCAL
            : ImGuizmo::WORLD;

    const bool changed =
        ImGuizmo::Manipulate(
            glm::value_ptr(
                view
            ),
            glm::value_ptr(
                projection
            ),
            operation,
            mode,
            glm::value_ptr(
                model
            )
        );

    if (
        !changed
    )
    {
        return;
    }

    float translation[3] =
    {
        0.0f,
        0.0f,
        0.0f
    };

    float rotation[3] =
    {
        0.0f,
        0.0f,
        0.0f
    };

    float scale[3] =
    {
        1.0f,
        1.0f,
        1.0f
    };

    ImGuizmo::DecomposeMatrixToComponents(
        glm::value_ptr(
            model
        ),
        translation,
        rotation,
        scale
    );

    entity->transform.position =
    {
        translation[0],
        translation[1],
        translation[2]
    };

    entity->transform.rotation =
    {
        rotation[0],
        rotation[1],
        rotation[2]
    };

    entity->transform.scale =
        glm::max(
            glm::vec3(
                scale[0],
                scale[1],
                scale[2]
            ),
            glm::vec3(
                0.05f
            )
        );
}

// =========================================================
// Selection Visual State
// =========================================================

void EditorUI::ApplySelection(
    Scene& scene,
    EntityId selectedEntityId
)
{
    for (
        Entity& entity :
        scene.GetEntities()
    )
    {
        entity.editorSelected =
            entity.id ==
            selectedEntityId;
    }
}
