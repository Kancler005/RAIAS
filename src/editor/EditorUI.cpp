#include "EditorUI.h"

#include "engine/scene/Scene.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include <glm/glm.hpp>

#include <algorithm>
#include <cstdio>

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
    EntityId& selectedEntityId
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

    DrawHierarchy(
        scene,
        selectedEntityId
    );

    DrawInspector(
        scene,
        selectedEntityId
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

    const float panelWidth =
        285.0f;

    ImGui::SetNextWindowPos(
        viewport->WorkPos,
        ImGuiCond_Always
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            panelWidth,
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

    const float panelWidth =
        360.0f;

    ImGui::SetNextWindowPos(
        ImVec2(
            viewport->WorkPos.x +
                viewport->WorkSize.x -
                panelWidth,
            viewport->WorkPos.y
        ),
        ImGuiCond_Always
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            panelWidth,
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
        "Changes are written to the Scene immediately."
    );

    ImGui::TextDisabled(
        "Press F5 to save them to JSON."
    );

    ImGui::End();
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
