#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <glm/glm.hpp>

#include "render/Mesh.h"
#include "render/GlyphRenderer.h"

#include <string>
#include <unordered_map>
#include <cstdint>

class Camera;
class Scene;
struct Entity;
struct ImDrawData;

class Renderer
{
public:
    bool Initialize(
        SDL_Window* window
    );

    bool Render(
        const Scene& scene,
        const Camera& camera,
        ImDrawData* uiDrawData = nullptr
    );

    [[nodiscard]]
    SDL_GPUDevice* GetDevice() const
    {
        return device;
    }

    void Shutdown();

private:
    SDL_GPUShader* LoadShader(
        const std::string& path,
        SDL_GPUShaderStage stage,
        Uint32 numSamplers,
        Uint32 numUniformBuffers
    );

    SDL_GPUTexture* CreateDepthTexture(
        Uint32 width,
        Uint32 height
    );

    SDL_GPUTexture* CreateOffscreenColorTexture(
        Uint32 width,
        Uint32 height
    );


    bool CreateSampler();

    bool CreateAsciiAtlas();

    bool UploadAsciiAtlas();

    bool CreatePipelines();

    void DrawModelWithColor(
        SDL_GPUCommandBuffer* commandBuffer,
        SDL_GPURenderPass* renderPass,
        const glm::mat4& viewProjection,
        const glm::mat4& model,
        const glm::vec4& color,
        Mesh& mesh
    );

    void DrawAnimatedKnight(
        SDL_GPUCommandBuffer* commandBuffer,
        SDL_GPURenderPass* renderPass,
        const glm::mat4& viewProjection,
        const Entity& entity
    );

    void DrawEntity(
        SDL_GPUCommandBuffer* commandBuffer,
        SDL_GPURenderPass* renderPass,
        const glm::mat4& viewProjection,
        const Entity& entity
    );

private:
    SDL_Window* window =
        nullptr;

    SDL_GPUDevice* device =
        nullptr;

    SDL_GPUGraphicsPipeline* scenePipeline =
        nullptr;

    SDL_GPUGraphicsPipeline* postProcessPipeline =
        nullptr;

    Mesh cubeMesh;

    // Procedural environment models.
    Mesh rockSmallMesh;
    Mesh rockWideMesh;
    Mesh rockTallMesh;
    Mesh ruinedShrineMesh;
    Mesh brokenWatchtowerMesh;

    Mesh deadTreeAMesh;
    Mesh deadTreeBMesh;
    Mesh campfireMesh;

    // First imported GLB/glTF mesh.
    // 0.0.18+ will move this into AssetManager.
    Mesh importedMesh;

    // World-space ASCII layer.
    // Unlike the old full-screen postprocess, these glyphs live
    // at real 3D positions and use the scene depth buffer.
    GlyphRenderer glyphRenderer;

    SDL_GPUTexture* depthTexture =
        nullptr;

    SDL_GPUTexture* offscreenColorTexture =
        nullptr;

    SDL_GPUTexture* historyColorTexture =
        nullptr;

    SDL_GPUTexture* asciiAtlasTexture =
        nullptr;

    SDL_GPUSampler* offscreenSampler =
        nullptr;

    Uint32 renderWidth =
        0;

    Uint32 renderHeight =
        0;

    bool historyValid =
        false;

    std::unordered_map<std::uint64_t, glm::vec3>
        previousEntityPositions;

    std::unordered_map<std::uint64_t, float>
        entityMotionBlend;

    float animationTime =
        0.0f;

    float animationDeltaTime =
        1.0f / 60.0f;
};
