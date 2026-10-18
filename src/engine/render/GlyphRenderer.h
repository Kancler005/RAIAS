#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <glm/glm.hpp>

#include <string>
#include <vector>

struct GlyphVertex
{
    float centerX;
    float centerY;
    float centerZ;

    float cornerX;
    float cornerY;

    float glyphIndex;

    float size;

    float r;
    float g;
    float b;
    float a;
};

class GlyphRenderer
{
public:
    bool Initialize(
        SDL_GPUDevice* device,
        SDL_GPUTextureFormat colorTargetFormat,
        SDL_GPUTexture* glyphAtlas,
        SDL_GPUSampler* glyphSampler
    );

    bool CreateGroundGrid(
        float minX,
        float maxX,
        float minZ,
        float maxZ,
        float spacing,
        float worldY
    );

    bool Render(
        SDL_GPUCommandBuffer* commandBuffer,
        SDL_GPUTexture* colorTarget,
        SDL_GPUTexture* depthTarget,
        const glm::mat4& viewProjection,
        const glm::vec3& cameraRight,
        const glm::vec3& cameraUp
    );

    void Shutdown();

    [[nodiscard]]
    Uint32 GetGlyphCount() const
    {
        return glyphCount;
    }

private:
    SDL_GPUShader* LoadShader(
        const std::string& path,
        SDL_GPUShaderStage stage,
        Uint32 numSamplers,
        Uint32 numUniformBuffers
    );

    bool CreatePipeline(
        SDL_GPUTextureFormat colorTargetFormat
    );

    bool UploadVertices(
        const std::vector<GlyphVertex>& vertices
    );

    static Uint32 HashGrid(
        int x,
        int z
    );

    static float Hash01(
        Uint32 value
    );

private:
    SDL_GPUDevice* device =
        nullptr;

    SDL_GPUGraphicsPipeline* pipeline =
        nullptr;

    SDL_GPUBuffer* vertexBuffer =
        nullptr;

    SDL_GPUTexture* glyphAtlas =
        nullptr;

    SDL_GPUSampler* glyphSampler =
        nullptr;

    Uint32 vertexCount =
        0;

    Uint32 glyphCount =
        0;
};
