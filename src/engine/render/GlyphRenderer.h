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

enum class GlyphMaterialKind : Uint8
{
    AshSoil,
    DeadGrass,
    Stone
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

    // Generates the first RAI world-material field.
    //
    // Unlike 0.0.22's uniform random grid, this uses low-frequency
    // deterministic noise to form coherent patches of ash, dead grass,
    // stone and empty negative space.
    bool CreateDarkFantasyGround(
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

    void AppendMaterialGlyph(
        std::vector<GlyphVertex>& vertices,
        GlyphMaterialKind material,
        const glm::vec3& center,
        Uint32 hash,
        float materialStrength
    );

    static Uint32 HashGrid(
        int x,
        int z,
        Uint32 seed = 0u
    );

    static float Hash01(
        Uint32 value
    );

    static float SmoothStep(
        float value
    );

    static float ValueNoise(
        float x,
        float z,
        float frequency,
        Uint32 seed
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

    Uint32 ashGlyphCount =
        0;

    Uint32 grassGlyphCount =
        0;

    Uint32 stoneGlyphCount =
        0;
};
