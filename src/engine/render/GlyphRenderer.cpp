#include "GlyphRenderer.h"

#include <glm/gtc/type_ptr.hpp>

#include <cstddef>
#include <cstring>
#include <fstream>
#include <vector>

namespace
{
    constexpr Uint32 ASCII_GLYPH_COUNT =
        8;

    constexpr Uint32 ASCII_ATLAS_WIDTH =
        64;

    constexpr Uint32 ASCII_ATLAS_HEIGHT =
        8;

    struct alignas(16) GlyphCameraUniform
    {
        glm::mat4 viewProjection;

        glm::vec4 cameraRight;

        glm::vec4 cameraUp;

        glm::vec4 glyphSettings;
    };

    struct alignas(16) GlyphAtlasUniform
    {
        glm::vec4 atlasInfo;
    };

    std::vector<Uint8> LoadFile(
        const std::string& path
    )
    {
        std::ifstream file(
            path,
            std::ios::binary |
            std::ios::ate
        );

        if (
            !file.is_open()
        )
        {
            SDL_Log(
                "GlyphRenderer could not open shader: %s",
                path.c_str()
            );

            return {};
        }

        const std::streamsize size =
            file.tellg();

        if (
            size <= 0
        )
        {
            return {};
        }

        file.seekg(
            0,
            std::ios::beg
        );

        std::vector<Uint8> data(
            static_cast<std::size_t>(
                size
            )
        );

        if (
            !file.read(
                reinterpret_cast<char*>(
                    data.data()
                ),
                size
            )
        )
        {
            return {};
        }

        return data;
    }

    void AppendGlyphQuad(
        std::vector<GlyphVertex>& vertices,
        const glm::vec3& center,
        float glyphIndex,
        float size,
        const glm::vec4& color
    )
    {
        const GlyphVertex topLeft
        {
            center.x,
            center.y,
            center.z,

            -0.5f,
            -0.5f,

            glyphIndex,
            size,

            color.r,
            color.g,
            color.b,
            color.a
        };

        const GlyphVertex topRight
        {
            center.x,
            center.y,
            center.z,

             0.5f,
            -0.5f,

            glyphIndex,
            size,

            color.r,
            color.g,
            color.b,
            color.a
        };

        const GlyphVertex bottomRight
        {
            center.x,
            center.y,
            center.z,

             0.5f,
             0.5f,

            glyphIndex,
            size,

            color.r,
            color.g,
            color.b,
            color.a
        };

        const GlyphVertex bottomLeft
        {
            center.x,
            center.y,
            center.z,

            -0.5f,
             0.5f,

            glyphIndex,
            size,

            color.r,
            color.g,
            color.b,
            color.a
        };

        // Triangle 1
        vertices.push_back(
            topLeft
        );

        vertices.push_back(
            topRight
        );

        vertices.push_back(
            bottomRight
        );

        // Triangle 2
        vertices.push_back(
            topLeft
        );

        vertices.push_back(
            bottomRight
        );

        vertices.push_back(
            bottomLeft
        );
    }
}

// ============================================================
// Initialize
// ============================================================

bool GlyphRenderer::Initialize(
    SDL_GPUDevice* newDevice,
    SDL_GPUTextureFormat colorTargetFormat,
    SDL_GPUTexture* newGlyphAtlas,
    SDL_GPUSampler* newGlyphSampler
)
{
    device =
        newDevice;

    glyphAtlas =
        newGlyphAtlas;

    glyphSampler =
        newGlyphSampler;

    if (
        device == nullptr ||
        glyphAtlas == nullptr ||
        glyphSampler == nullptr
    )
    {
        SDL_Log(
            "GlyphRenderer initialization failed: invalid resources."
        );

        return false;
    }

    if (
        !CreatePipeline(
            colorTargetFormat
        )
    )
    {
        return false;
    }

    SDL_Log(
        "RAI World GlyphRenderer initialized."
    );

    return true;
}

// ============================================================
// Shader
// ============================================================

SDL_GPUShader* GlyphRenderer::LoadShader(
    const std::string& path,
    SDL_GPUShaderStage stage,
    Uint32 numSamplers,
    Uint32 numUniformBuffers
)
{
    const std::vector<Uint8> code =
        LoadFile(
            path
        );

    if (
        code.empty()
    )
    {
        return nullptr;
    }

    SDL_GPUShaderCreateInfo info{};

    info.code =
        code.data();

    info.code_size =
        code.size();

    info.entrypoint =
        "main0";

    info.format =
        SDL_GPU_SHADERFORMAT_MSL;

    info.stage =
        stage;

    info.num_samplers =
        numSamplers;

    info.num_storage_textures =
        0;

    info.num_storage_buffers =
        0;

    info.num_uniform_buffers =
        numUniformBuffers;

    SDL_GPUShader* shader =
        SDL_CreateGPUShader(
            device,
            &info
        );

    if (
        shader == nullptr
    )
    {
        SDL_Log(
            "GlyphRenderer shader creation failed (%s): %s",
            path.c_str(),
            SDL_GetError()
        );
    }

    return shader;
}

// ============================================================
// Pipeline
// ============================================================

bool GlyphRenderer::CreatePipeline(
    SDL_GPUTextureFormat colorTargetFormat
)
{
    SDL_GPUShader* vertexShader =
        LoadShader(
            "assets/shaders/compiled/msl/world_glyph.vert.msl",
            SDL_GPU_SHADERSTAGE_VERTEX,
            0,
            1
        );

    SDL_GPUShader* fragmentShader =
        LoadShader(
            "assets/shaders/compiled/msl/world_glyph.frag.msl",
            SDL_GPU_SHADERSTAGE_FRAGMENT,
            1,
            1
        );

    if (
        vertexShader == nullptr ||
        fragmentShader == nullptr
    )
    {
        if (
            vertexShader != nullptr
        )
        {
            SDL_ReleaseGPUShader(
                device,
                vertexShader
            );
        }

        if (
            fragmentShader != nullptr
        )
        {
            SDL_ReleaseGPUShader(
                device,
                fragmentShader
            );
        }

        return false;
    }

    SDL_GPUVertexBufferDescription
        vertexDescription{};

    vertexDescription.slot =
        0;

    vertexDescription.pitch =
        sizeof(
            GlyphVertex
        );

    vertexDescription.input_rate =
        SDL_GPU_VERTEXINPUTRATE_VERTEX;

    vertexDescription.instance_step_rate =
        0;

    SDL_GPUVertexAttribute attributes[5]{};

    attributes[0].location =
        0;

    attributes[0].buffer_slot =
        0;

    attributes[0].format =
        SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;

    attributes[0].offset =
        offsetof(
            GlyphVertex,
            centerX
        );

    attributes[1].location =
        1;

    attributes[1].buffer_slot =
        0;

    attributes[1].format =
        SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;

    attributes[1].offset =
        offsetof(
            GlyphVertex,
            cornerX
        );

    attributes[2].location =
        2;

    attributes[2].buffer_slot =
        0;

    attributes[2].format =
        SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;

    attributes[2].offset =
        offsetof(
            GlyphVertex,
            glyphIndex
        );

    attributes[3].location =
        3;

    attributes[3].buffer_slot =
        0;

    attributes[3].format =
        SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;

    attributes[3].offset =
        offsetof(
            GlyphVertex,
            size
        );

    attributes[4].location =
        4;

    attributes[4].buffer_slot =
        0;

    attributes[4].format =
        SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;

    attributes[4].offset =
        offsetof(
            GlyphVertex,
            r
        );

    SDL_GPUColorTargetDescription
        colorTarget{};

    colorTarget.format =
        colorTargetFormat;

    colorTarget
        .blend_state
        .enable_blend =
        false;

    colorTarget
        .blend_state
        .color_write_mask =
        0xF;

    SDL_GPUGraphicsPipelineCreateInfo
        pipelineInfo{};

    pipelineInfo.vertex_shader =
        vertexShader;

    pipelineInfo.fragment_shader =
        fragmentShader;

    pipelineInfo.primitive_type =
        SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

    pipelineInfo
        .vertex_input_state
        .vertex_buffer_descriptions =
        &vertexDescription;

    pipelineInfo
        .vertex_input_state
        .num_vertex_buffers =
        1;

    pipelineInfo
        .vertex_input_state
        .vertex_attributes =
        attributes;

    pipelineInfo
        .vertex_input_state
        .num_vertex_attributes =
        5;

    pipelineInfo
        .rasterizer_state
        .fill_mode =
        SDL_GPU_FILLMODE_FILL;

    pipelineInfo
        .rasterizer_state
        .cull_mode =
        SDL_GPU_CULLMODE_NONE;

    pipelineInfo
        .rasterizer_state
        .front_face =
        SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

    pipelineInfo
        .rasterizer_state
        .enable_depth_clip =
        true;

    pipelineInfo
        .multisample_state
        .sample_count =
        SDL_GPU_SAMPLECOUNT_1;

    pipelineInfo
        .depth_stencil_state
        .enable_depth_test =
        true;

    pipelineInfo
        .depth_stencil_state
        .enable_depth_write =
        false;

    pipelineInfo
        .depth_stencil_state
        .compare_op =
        SDL_GPU_COMPAREOP_LESS_OR_EQUAL;

    pipelineInfo
        .target_info
        .num_color_targets =
        1;

    pipelineInfo
        .target_info
        .color_target_descriptions =
        &colorTarget;

    pipelineInfo
        .target_info
        .has_depth_stencil_target =
        true;

    pipelineInfo
        .target_info
        .depth_stencil_format =
        SDL_GPU_TEXTUREFORMAT_D16_UNORM;

    pipeline =
        SDL_CreateGPUGraphicsPipeline(
            device,
            &pipelineInfo
        );

    SDL_ReleaseGPUShader(
        device,
        vertexShader
    );

    SDL_ReleaseGPUShader(
        device,
        fragmentShader
    );

    if (
        pipeline == nullptr
    )
    {
        SDL_Log(
            "GlyphRenderer pipeline creation failed: %s",
            SDL_GetError()
        );

        return false;
    }

    return true;
}

// ============================================================
// Deterministic Hash
// ============================================================

Uint32 GlyphRenderer::HashGrid(
    int x,
    int z
)
{
    Uint32 value =
        static_cast<Uint32>(
            x * 374761393
        ) ^
        static_cast<Uint32>(
            z * 668265263
        );

    value =
        (
            value ^
            (
                value >>
                13
            )
        ) *
        1274126177u;

    return
        value ^
        (
            value >>
            16
        );
}

float GlyphRenderer::Hash01(
    Uint32 value
)
{
    return
        static_cast<float>(
            value &
            0x00FFFFFFu
        ) /
        static_cast<float>(
            0x01000000u
        );
}

// ============================================================
// Ground Grid
// ============================================================

bool GlyphRenderer::CreateGroundGrid(
    float minX,
    float maxX,
    float minZ,
    float maxZ,
    float spacing,
    float worldY
)
{
    if (
        spacing <= 0.0f
    )
    {
        return false;
    }

    std::vector<GlyphVertex>
        vertices;

    glyphCount =
        0;

    int gridZ =
        0;

    for (
        float z = minZ;
        z <= maxZ;
        z += spacing,
        ++gridZ
    )
    {
        int gridX =
            0;

        for (
            float x = minX;
            x <= maxX;
            x += spacing,
            ++gridX
        )
        {
            const Uint32 hash =
                HashGrid(
                    gridX,
                    gridZ
                );

            const float density =
                Hash01(
                    hash
                );

            // Keep negative space.
            // This is the first step away from the "ASCII filter"
            // look where every screen cell contains a character.
            if (
                density <
                0.32f
            )
            {
                continue;
            }

            const float jitterX =
                (
                    Hash01(
                        hash ^
                        0xA53A9E17u
                    ) -
                    0.5f
                ) *
                spacing *
                0.30f;

            const float jitterZ =
                (
                    Hash01(
                        hash ^
                        0x8D12F4A1u
                    ) -
                    0.5f
                ) *
                spacing *
                0.30f;

            const float size =
                0.72f +
                Hash01(
                    hash ^
                    0x46B8C2D3u
                ) *
                0.48f;

            // Use the lighter half of our existing atlas:
            // . : - + *
            const float glyph =
                1.0f +
                static_cast<float>(
                    (
                        hash >>
                        8
                    ) %
                    5u
                );

            const float colorNoise =
                Hash01(
                    hash ^
                    0xD341A77Bu
                );

            glm::vec4 color;

            if (
                colorNoise <
                0.33f
            )
            {
                color =
                {
                    0.24f,
                    0.24f,
                    0.20f,
                    1.0f
                };
            }
            else if (
                colorNoise <
                0.66f
            )
            {
                color =
                {
                    0.18f,
                    0.22f,
                    0.17f,
                    1.0f
                };
            }
            else
            {
                color =
                {
                    0.28f,
                    0.23f,
                    0.17f,
                    1.0f
                };
            }

            // A little deterministic brightness variation.
            const float brightness =
                0.72f +
                Hash01(
                    hash ^
                    0x1B56C4E9u
                ) *
                0.48f;

            color.r *=
                brightness;

            color.g *=
                brightness;

            color.b *=
                brightness;

            const glm::vec3 center
            {
                x +
                    jitterX,

                worldY,

                z +
                    jitterZ
            };

            AppendGlyphQuad(
                vertices,
                center,
                glyph,
                size,
                color
            );

            ++glyphCount;
        }
    }

    if (
        !UploadVertices(
            vertices
        )
    )
    {
        glyphCount =
            0;

        return false;
    }

    SDL_Log(
        "World Glyph Grid created | glyphs=%u vertices=%u",
        glyphCount,
        vertexCount
    );

    return true;
}

// ============================================================
// Upload
// ============================================================

bool GlyphRenderer::UploadVertices(
    const std::vector<GlyphVertex>& vertices
)
{
    if (
        device == nullptr ||
        vertices.empty()
    )
    {
        return false;
    }

    if (
        vertexBuffer != nullptr
    )
    {
        SDL_ReleaseGPUBuffer(
            device,
            vertexBuffer
        );

        vertexBuffer =
            nullptr;
    }

    vertexCount =
        static_cast<Uint32>(
            vertices.size()
        );

    const Uint32 dataSize =
        static_cast<Uint32>(
            vertices.size() *
            sizeof(
                GlyphVertex
            )
        );

    SDL_GPUBufferCreateInfo
        bufferInfo{};

    bufferInfo.usage =
        SDL_GPU_BUFFERUSAGE_VERTEX;

    bufferInfo.size =
        dataSize;

    vertexBuffer =
        SDL_CreateGPUBuffer(
            device,
            &bufferInfo
        );

    if (
        vertexBuffer == nullptr
    )
    {
        SDL_Log(
            "GlyphRenderer vertex buffer creation failed: %s",
            SDL_GetError()
        );

        return false;
    }

    SDL_GPUTransferBufferCreateInfo
        transferInfo{};

    transferInfo.usage =
        SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;

    transferInfo.size =
        dataSize;

    SDL_GPUTransferBuffer* transfer =
        SDL_CreateGPUTransferBuffer(
            device,
            &transferInfo
        );

    if (
        transfer == nullptr
    )
    {
        SDL_Log(
            "GlyphRenderer transfer buffer creation failed: %s",
            SDL_GetError()
        );

        return false;
    }

    void* mapped =
        SDL_MapGPUTransferBuffer(
            device,
            transfer,
            false
        );

    if (
        mapped == nullptr
    )
    {
        SDL_ReleaseGPUTransferBuffer(
            device,
            transfer
        );

        return false;
    }

    std::memcpy(
        mapped,
        vertices.data(),
        dataSize
    );

    SDL_UnmapGPUTransferBuffer(
        device,
        transfer
    );

    SDL_GPUCommandBuffer* commandBuffer =
        SDL_AcquireGPUCommandBuffer(
            device
        );

    if (
        commandBuffer == nullptr
    )
    {
        SDL_ReleaseGPUTransferBuffer(
            device,
            transfer
        );

        return false;
    }

    SDL_GPUCopyPass* copyPass =
        SDL_BeginGPUCopyPass(
            commandBuffer
        );

    if (
        copyPass == nullptr
    )
    {
        SDL_CancelGPUCommandBuffer(
            commandBuffer
        );

        SDL_ReleaseGPUTransferBuffer(
            device,
            transfer
        );

        return false;
    }

    SDL_GPUTransferBufferLocation
        source{};

    source.transfer_buffer =
        transfer;

    source.offset =
        0;

    SDL_GPUBufferRegion
        destination{};

    destination.buffer =
        vertexBuffer;

    destination.offset =
        0;

    destination.size =
        dataSize;

    SDL_UploadToGPUBuffer(
        copyPass,
        &source,
        &destination,
        false
    );

    SDL_EndGPUCopyPass(
        copyPass
    );

    if (
        !SDL_SubmitGPUCommandBuffer(
            commandBuffer
        )
    )
    {
        SDL_ReleaseGPUTransferBuffer(
            device,
            transfer
        );

        return false;
    }

    SDL_WaitForGPUIdle(
        device
    );

    SDL_ReleaseGPUTransferBuffer(
        device,
        transfer
    );

    return true;
}

// ============================================================
// Render
// ============================================================

bool GlyphRenderer::Render(
    SDL_GPUCommandBuffer* commandBuffer,
    SDL_GPUTexture* colorTarget,
    SDL_GPUTexture* depthTarget,
    const glm::mat4& viewProjection,
    const glm::vec3& cameraRight,
    const glm::vec3& cameraUp
)
{
    if (
        pipeline == nullptr ||
        vertexBuffer == nullptr ||
        vertexCount == 0 ||
        commandBuffer == nullptr ||
        colorTarget == nullptr ||
        depthTarget == nullptr
    )
    {
        return true;
    }

    SDL_GPUColorTargetInfo
        colorInfo{};

    colorInfo.texture =
        colorTarget;

    colorInfo.load_op =
        SDL_GPU_LOADOP_LOAD;

    colorInfo.store_op =
        SDL_GPU_STOREOP_STORE;

    SDL_GPUDepthStencilTargetInfo
        depthInfo{};

    depthInfo.texture =
        depthTarget;

    depthInfo.load_op =
        SDL_GPU_LOADOP_LOAD;

    depthInfo.store_op =
        SDL_GPU_STOREOP_DONT_CARE;

    depthInfo.stencil_load_op =
        SDL_GPU_LOADOP_DONT_CARE;

    depthInfo.stencil_store_op =
        SDL_GPU_STOREOP_DONT_CARE;

    SDL_GPURenderPass* renderPass =
        SDL_BeginGPURenderPass(
            commandBuffer,
            &colorInfo,
            1,
            &depthInfo
        );

    if (
        renderPass == nullptr
    )
    {
        SDL_Log(
            "GlyphRenderer render pass failed: %s",
            SDL_GetError()
        );

        return false;
    }

    SDL_BindGPUGraphicsPipeline(
        renderPass,
        pipeline
    );

    SDL_GPUBufferBinding
        vertexBinding{};

    vertexBinding.buffer =
        vertexBuffer;

    vertexBinding.offset =
        0;

    SDL_BindGPUVertexBuffers(
        renderPass,
        0,
        &vertexBinding,
        1
    );

    SDL_GPUTextureSamplerBinding
        samplerBinding{};

    samplerBinding.texture =
        glyphAtlas;

    samplerBinding.sampler =
        glyphSampler;

    SDL_BindGPUFragmentSamplers(
        renderPass,
        0,
        &samplerBinding,
        1
    );

    GlyphCameraUniform
        cameraData{};

    cameraData.viewProjection =
        viewProjection;

    cameraData.cameraRight =
        glm::vec4(
            cameraRight,
            0.0f
        );

    cameraData.cameraUp =
        glm::vec4(
            cameraUp,
            0.0f
        );

    // Small world-space billboards.
    cameraData.glyphSettings =
    {
        0.18f,
        0.27f,
        0.0f,
        0.0f
    };

    SDL_PushGPUVertexUniformData(
        commandBuffer,
        0,
        &cameraData,
        sizeof(
            GlyphCameraUniform
        )
    );

    GlyphAtlasUniform
        atlasData{};

    atlasData.atlasInfo =
    {
        static_cast<float>(
            ASCII_GLYPH_COUNT
        ),

        static_cast<float>(
            ASCII_ATLAS_WIDTH
        ),

        static_cast<float>(
            ASCII_ATLAS_HEIGHT
        ),

        0.0f
    };

    SDL_PushGPUFragmentUniformData(
        commandBuffer,
        0,
        &atlasData,
        sizeof(
            GlyphAtlasUniform
        )
    );

    SDL_DrawGPUPrimitives(
        renderPass,
        vertexCount,
        1,
        0,
        0
    );

    SDL_EndGPURenderPass(
        renderPass
    );

    return true;
}

// ============================================================
// Shutdown
// ============================================================

void GlyphRenderer::Shutdown()
{
    if (
        device == nullptr
    )
    {
        return;
    }

    if (
        vertexBuffer != nullptr
    )
    {
        SDL_ReleaseGPUBuffer(
            device,
            vertexBuffer
        );

        vertexBuffer =
            nullptr;
    }

    if (
        pipeline != nullptr
    )
    {
        SDL_ReleaseGPUGraphicsPipeline(
            device,
            pipeline
        );

        pipeline =
            nullptr;
    }

    vertexCount =
        0;

    glyphCount =
        0;

    glyphAtlas =
        nullptr;

    glyphSampler =
        nullptr;

    device =
        nullptr;
}
