#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <glm/glm.hpp>

#include <string>

class Camera;
class Scene;
struct Entity;

class Renderer
{
public:
    bool Initialize(
        SDL_Window* window
    );

    bool Render(
        const Scene& scene,
        const Camera& camera
    );

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

    bool CreateVertexBuffer();

    bool UploadCubeVertices();

    bool CreateSampler();

    bool CreateAsciiAtlas();

    bool UploadAsciiAtlas();

    bool CreatePipelines();

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

    SDL_GPUBuffer* vertexBuffer =
        nullptr;

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
};
