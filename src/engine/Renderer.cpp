#include "Renderer.h"

#include "Camera.h"
#include "scene/Scene.h"

#include <glm/gtc/type_ptr.hpp>

#include <cstddef>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

// =========================================================
// Internal Renderer Data
// =========================================================

namespace
{
    constexpr Uint32 ASCII_GLYPH_WIDTH =
        8;

    constexpr Uint32 ASCII_GLYPH_HEIGHT =
        8;

    constexpr Uint32 ASCII_GLYPH_COUNT =
        8;

    constexpr Uint32 ASCII_ATLAS_WIDTH =
        ASCII_GLYPH_WIDTH *
        ASCII_GLYPH_COUNT;

    constexpr Uint32 ASCII_ATLAS_HEIGHT =
        ASCII_GLYPH_HEIGHT;

    struct CubeVertex
    {
        float px;
        float py;
        float pz;

        float nx;
        float ny;
        float nz;
    };

    struct alignas(16) TransformUniform
    {
        glm::mat4 mvp;
        glm::mat4 normalMatrix;
    };

    struct alignas(16) LightUniform
    {
        glm::vec4 direction;
        glm::vec4 color;
        glm::vec4 params;
    };

    struct alignas(16) AsciiUniform
    {
        glm::vec4 screenAndCell;
        glm::vec4 atlasInfo;
        glm::vec4 temporalInfo;
    };

    // =====================================================
    // Cube
    // =====================================================

    const CubeVertex cubeVertices[] =
    {
        // FRONT

        {
            -0.5f, -0.5f, -0.5f,
             0.0f,  0.0f, -1.0f
        },

        {
             0.5f, -0.5f, -0.5f,
             0.0f,  0.0f, -1.0f
        },

        {
             0.5f,  0.5f, -0.5f,
             0.0f,  0.0f, -1.0f
        },

        {
            -0.5f, -0.5f, -0.5f,
             0.0f,  0.0f, -1.0f
        },

        {
             0.5f,  0.5f, -0.5f,
             0.0f,  0.0f, -1.0f
        },

        {
            -0.5f,  0.5f, -0.5f,
             0.0f,  0.0f, -1.0f
        },

        // BACK

        {
             0.5f, -0.5f, 0.5f,
             0.0f,  0.0f, 1.0f
        },

        {
            -0.5f, -0.5f, 0.5f,
             0.0f,  0.0f, 1.0f
        },

        {
            -0.5f,  0.5f, 0.5f,
             0.0f,  0.0f, 1.0f
        },

        {
             0.5f, -0.5f, 0.5f,
             0.0f,  0.0f, 1.0f
        },

        {
            -0.5f,  0.5f, 0.5f,
             0.0f,  0.0f, 1.0f
        },

        {
             0.5f,  0.5f, 0.5f,
             0.0f,  0.0f, 1.0f
        },

        // LEFT

        {
            -0.5f, -0.5f,  0.5f,
            -1.0f,  0.0f,  0.0f
        },

        {
            -0.5f, -0.5f, -0.5f,
            -1.0f,  0.0f,  0.0f
        },

        {
            -0.5f,  0.5f, -0.5f,
            -1.0f,  0.0f,  0.0f
        },

        {
            -0.5f, -0.5f,  0.5f,
            -1.0f,  0.0f,  0.0f
        },

        {
            -0.5f,  0.5f, -0.5f,
            -1.0f,  0.0f,  0.0f
        },

        {
            -0.5f,  0.5f,  0.5f,
            -1.0f,  0.0f,  0.0f
        },

        // RIGHT

        {
             0.5f, -0.5f, -0.5f,
             1.0f,  0.0f,  0.0f
        },

        {
             0.5f, -0.5f,  0.5f,
             1.0f,  0.0f,  0.0f
        },

        {
             0.5f,  0.5f,  0.5f,
             1.0f,  0.0f,  0.0f
        },

        {
             0.5f, -0.5f, -0.5f,
             1.0f,  0.0f,  0.0f
        },

        {
             0.5f,  0.5f,  0.5f,
             1.0f,  0.0f,  0.0f
        },

        {
             0.5f,  0.5f, -0.5f,
             1.0f,  0.0f,  0.0f
        },

        // TOP

        {
            -0.5f, 0.5f, -0.5f,
             0.0f, 1.0f, 0.0f
        },

        {
             0.5f, 0.5f, -0.5f,
             0.0f, 1.0f, 0.0f
        },

        {
             0.5f, 0.5f,  0.5f,
             0.0f, 1.0f, 0.0f
        },

        {
            -0.5f, 0.5f, -0.5f,
             0.0f, 1.0f, 0.0f
        },

        {
             0.5f, 0.5f,  0.5f,
             0.0f, 1.0f, 0.0f
        },

        {
            -0.5f, 0.5f,  0.5f,
             0.0f, 1.0f, 0.0f
        },

        // BOTTOM

        {
            -0.5f, -0.5f,  0.5f,
             0.0f, -1.0f, 0.0f
        },

        {
             0.5f, -0.5f,  0.5f,
             0.0f, -1.0f, 0.0f
        },

        {
             0.5f, -0.5f, -0.5f,
             0.0f, -1.0f, 0.0f
        },

        {
            -0.5f, -0.5f,  0.5f,
             0.0f, -1.0f, 0.0f
        },

        {
             0.5f, -0.5f, -0.5f,
             0.0f, -1.0f, 0.0f
        },

        {
            -0.5f, -0.5f, -0.5f,
             0.0f, -1.0f, 0.0f
        }
    };

    // =====================================================
    // ASCII glyphs
    //
    // 0 space
    // 1 .
    // 2 :
    // 3 -
    // 4 +
    // 5 *
    // 6 #
    // 7 @
    // =====================================================

    constexpr const char* asciiGlyphs
        [ASCII_GLYPH_COUNT]
        [ASCII_GLYPH_HEIGHT] =
    {
        // SPACE

        {
            "00000000",
            "00000000",
            "00000000",
            "00000000",
            "00000000",
            "00000000",
            "00000000",
            "00000000"
        },

        // .

        {
            "00000000",
            "00000000",
            "00000000",
            "00000000",
            "00000000",
            "00000000",
            "00011000",
            "00011000"
        },

        // :

        {
            "00000000",
            "00011000",
            "00011000",
            "00000000",
            "00000000",
            "00011000",
            "00011000",
            "00000000"
        },

        // -

        {
            "00000000",
            "00000000",
            "00000000",
            "01111110",
            "01111110",
            "00000000",
            "00000000",
            "00000000"
        },

        // +

        {
            "00000000",
            "00011000",
            "00011000",
            "01111110",
            "01111110",
            "00011000",
            "00011000",
            "00000000"
        },

        // *

        {
            "00000000",
            "01011010",
            "00111100",
            "11111111",
            "00111100",
            "01011010",
            "00000000",
            "00000000"
        },

        // #

        {
            "00100100",
            "00100100",
            "11111111",
            "00100100",
            "00100100",
            "11111111",
            "00100100",
            "00100100"
        },

        // @

        {
            "00111100",
            "01000010",
            "10011010",
            "10101010",
            "10111100",
            "10000000",
            "01000010",
            "00111100"
        }
    };

    // =====================================================
    // File Loading
    // =====================================================

    std::vector<Uint8> LoadFile(
        const std::string& path
    )
    {
        std::ifstream file(
            path,
            std::ios::binary |
            std::ios::ate
        );

        if (!file.is_open())
        {
            SDL_Log(
                "Could not open file: %s",
                path.c_str()
            );

            return {};
        }

        const std::streamsize size =
            file.tellg();

        if (size <= 0)
        {
            return {};
        }

        file.seekg(
            0,
            std::ios::beg
        );

        std::vector<Uint8> data(
            static_cast<size_t>(
                size
            )
        );

        if (!file.read(
                reinterpret_cast<char*>(
                    data.data()
                ),
                size))
        {
            return {};
        }

        return data;
    }

    // =====================================================
    // Build ASCII Atlas Pixels
    // =====================================================

    std::vector<Uint8> BuildAsciiAtlasPixels()
    {
        constexpr Uint32 channels =
            4;

        std::vector<Uint8> pixels(
            ASCII_ATLAS_WIDTH *
            ASCII_ATLAS_HEIGHT *
            channels,
            0
        );

        for (
            Uint32 glyph = 0;
            glyph < ASCII_GLYPH_COUNT;
            ++glyph
        )
        {
            for (
                Uint32 y = 0;
                y < ASCII_GLYPH_HEIGHT;
                ++y
            )
            {
                for (
                    Uint32 x = 0;
                    x < ASCII_GLYPH_WIDTH;
                    ++x
                )
                {
                    const bool enabled =
                        asciiGlyphs[glyph][y][x] ==
                        '1';

                    const Uint32 atlasX =
                        glyph *
                        ASCII_GLYPH_WIDTH +
                        x;

                    const Uint32 index =
                        (
                            y *
                            ASCII_ATLAS_WIDTH +
                            atlasX
                        ) *
                        channels;

                    const Uint8 value =
                        enabled ?
                        255 :
                        0;

                    pixels[index + 0] =
                        value;

                    pixels[index + 1] =
                        value;

                    pixels[index + 2] =
                        value;

                    pixels[index + 3] =
                        255;
                }
            }
        }

        return pixels;
    }
}

// =========================================================
// Initialize
// =========================================================

bool Renderer::Initialize(
    SDL_Window* newWindow
)
{
    window =
        newWindow;

    device =
        SDL_CreateGPUDevice(
            SDL_GPU_SHADERFORMAT_MSL |
            SDL_GPU_SHADERFORMAT_DXIL |
            SDL_GPU_SHADERFORMAT_SPIRV,
            true,
            nullptr
        );

    if (
        device == nullptr
    )
    {
        SDL_Log(
            "GPU creation failed: %s",
            SDL_GetError()
        );

        return false;
    }

    SDL_Log(
        "RAI GPU backend: %s",
        SDL_GetGPUDeviceDriver(
            device
        )
    );

    if (
        !SDL_ClaimWindowForGPUDevice(
            device,
            window
        )
    )
    {
        SDL_Log(
            "GPU window claim failed: %s",
            SDL_GetError()
        );

        return false;
    }

    if (
        !CreateVertexBuffer()
    )
    {
        return false;
    }

    if (
        !UploadCubeVertices()
    )
    {
        return false;
    }

    if (
        !CreateSampler()
    )
    {
        return false;
    }

    if (
        !CreateAsciiAtlas()
    )
    {
        return false;
    }

    if (
        !UploadAsciiAtlas()
    )
    {
        return false;
    }

    if (
        !CreatePipelines()
    )
    {
        return false;
    }

    SDL_Log(
        "ASCII atlas created: %ux%u",
        ASCII_ATLAS_WIDTH,
        ASCII_ATLAS_HEIGHT
    );

    return true;
}

// =========================================================
// Shader
// =========================================================

SDL_GPUShader* Renderer::LoadShader(
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
            "Shader creation failed (%s): %s",
            path.c_str(),
            SDL_GetError()
        );
    }

    return shader;
}

// =========================================================
// Vertex Buffer
// =========================================================

bool Renderer::CreateVertexBuffer()
{
    SDL_GPUBufferCreateInfo info{};

    info.usage =
        SDL_GPU_BUFFERUSAGE_VERTEX;

    info.size =
        static_cast<Uint32>(
            sizeof(cubeVertices)
        );

    vertexBuffer =
        SDL_CreateGPUBuffer(
            device,
            &info
        );

    if (
        vertexBuffer == nullptr
    )
    {
        SDL_Log(
            "Vertex buffer creation failed: %s",
            SDL_GetError()
        );

        return false;
    }

    return true;
}

// =========================================================
// Upload Cube
// =========================================================

bool Renderer::UploadCubeVertices()
{
    const Uint32 dataSize =
        static_cast<Uint32>(
            sizeof(cubeVertices)
        );

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
        cubeVertices,
        sizeof(cubeVertices)
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

// =========================================================
// Sampler
// =========================================================

bool Renderer::CreateSampler()
{
    SDL_GPUSamplerCreateInfo info{};

    info.min_filter =
        SDL_GPU_FILTER_NEAREST;

    info.mag_filter =
        SDL_GPU_FILTER_NEAREST;

    info.mipmap_mode =
        SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;

    info.address_mode_u =
        SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

    info.address_mode_v =
        SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

    info.address_mode_w =
        SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

    info.min_lod =
        0.0f;

    info.max_lod =
        0.0f;

    info.enable_anisotropy =
        false;

    info.enable_compare =
        false;

    offscreenSampler =
        SDL_CreateGPUSampler(
            device,
            &info
        );

    if (
        offscreenSampler == nullptr
    )
    {
        SDL_Log(
            "Sampler creation failed: %s",
            SDL_GetError()
        );

        return false;
    }

    return true;
}

// =========================================================
// Create ASCII Atlas Texture
// =========================================================

bool Renderer::CreateAsciiAtlas()
{
    SDL_GPUTextureCreateInfo info{};

    info.type =
        SDL_GPU_TEXTURETYPE_2D;

    info.format =
        SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

    info.usage =
        SDL_GPU_TEXTUREUSAGE_SAMPLER;

    info.width =
        ASCII_ATLAS_WIDTH;

    info.height =
        ASCII_ATLAS_HEIGHT;

    info.layer_count_or_depth =
        1;

    info.num_levels =
        1;

    info.sample_count =
        SDL_GPU_SAMPLECOUNT_1;

    asciiAtlasTexture =
        SDL_CreateGPUTexture(
            device,
            &info
        );

    if (
        asciiAtlasTexture == nullptr
    )
    {
        SDL_Log(
            "ASCII atlas texture failed: %s",
            SDL_GetError()
        );

        return false;
    }

    return true;
}

// =========================================================
// Upload ASCII Atlas
// =========================================================

bool Renderer::UploadAsciiAtlas()
{
    const std::vector<Uint8> pixels =
        BuildAsciiAtlasPixels();

    const Uint32 dataSize =
        static_cast<Uint32>(
            pixels.size()
        );

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
            "ASCII transfer buffer failed: %s",
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
        pixels.data(),
        pixels.size()
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

    SDL_GPUTextureTransferInfo
        source{};

    source.transfer_buffer =
        transfer;

    source.offset =
        0;

    source.pixels_per_row =
        ASCII_ATLAS_WIDTH;

    source.rows_per_layer =
        ASCII_ATLAS_HEIGHT;

    SDL_GPUTextureRegion
        destination{};

    destination.texture =
        asciiAtlasTexture;

    destination.mip_level =
        0;

    destination.layer =
        0;

    destination.x =
        0;

    destination.y =
        0;

    destination.z =
        0;

    destination.w =
        ASCII_ATLAS_WIDTH;

    destination.h =
        ASCII_ATLAS_HEIGHT;

    destination.d =
        1;

    SDL_UploadToGPUTexture(
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

// =========================================================
// Pipelines
// =========================================================

bool Renderer::CreatePipelines()
{
    SDL_GPUShader* sceneVertexShader =
        LoadShader(
            "assets/shaders/compiled/msl/cube.vert.msl",
            SDL_GPU_SHADERSTAGE_VERTEX,
            0,
            2
        );

    SDL_GPUShader* sceneFragmentShader =
        LoadShader(
            "assets/shaders/compiled/msl/cube.frag.msl",
            SDL_GPU_SHADERSTAGE_FRAGMENT,
            0,
            1
        );

    SDL_GPUShader* fullscreenVertexShader =
        LoadShader(
            "assets/shaders/compiled/msl/fullscreen.vert.msl",
            SDL_GPU_SHADERSTAGE_VERTEX,
            0,
            0
        );

    SDL_GPUShader* postFragmentShader =
        LoadShader(
            "assets/shaders/compiled/msl/postprocess.frag.msl",
            SDL_GPU_SHADERSTAGE_FRAGMENT,
            3,
            1
        );

    if (
        sceneVertexShader == nullptr ||
        sceneFragmentShader == nullptr ||
        fullscreenVertexShader == nullptr ||
        postFragmentShader == nullptr
    )
    {
        if (sceneVertexShader)
        {
            SDL_ReleaseGPUShader(
                device,
                sceneVertexShader
            );
        }

        if (sceneFragmentShader)
        {
            SDL_ReleaseGPUShader(
                device,
                sceneFragmentShader
            );
        }

        if (fullscreenVertexShader)
        {
            SDL_ReleaseGPUShader(
                device,
                fullscreenVertexShader
            );
        }

        if (postFragmentShader)
        {
            SDL_ReleaseGPUShader(
                device,
                postFragmentShader
            );
        }

        return false;
    }

    // =====================================================
    // Scene Pipeline
    // =====================================================

    SDL_GPUVertexBufferDescription
        vertexDescription{};

    vertexDescription.slot =
        0;

    vertexDescription.pitch =
        sizeof(CubeVertex);

    vertexDescription.input_rate =
        SDL_GPU_VERTEXINPUTRATE_VERTEX;

    vertexDescription.instance_step_rate =
        0;

    SDL_GPUVertexAttribute attributes[2]{};

    attributes[0].location =
        0;

    attributes[0].buffer_slot =
        0;

    attributes[0].format =
        SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;

    attributes[0].offset =
        offsetof(
            CubeVertex,
            px
        );

    attributes[1].location =
        1;

    attributes[1].buffer_slot =
        0;

    attributes[1].format =
        SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;

    attributes[1].offset =
        offsetof(
            CubeVertex,
            nx
        );

    SDL_GPUColorTargetDescription
        sceneColorTarget{};

    sceneColorTarget.format =
        SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

    sceneColorTarget
        .blend_state
        .enable_blend =
        false;

    sceneColorTarget
        .blend_state
        .color_write_mask =
        0xF;

    SDL_GPUGraphicsPipelineCreateInfo
        sceneInfo{};

    sceneInfo.vertex_shader =
        sceneVertexShader;

    sceneInfo.fragment_shader =
        sceneFragmentShader;

    sceneInfo.primitive_type =
        SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

    sceneInfo
        .vertex_input_state
        .vertex_buffer_descriptions =
        &vertexDescription;

    sceneInfo
        .vertex_input_state
        .num_vertex_buffers =
        1;

    sceneInfo
        .vertex_input_state
        .vertex_attributes =
        attributes;

    sceneInfo
        .vertex_input_state
        .num_vertex_attributes =
        2;

    sceneInfo
        .rasterizer_state
        .fill_mode =
        SDL_GPU_FILLMODE_FILL;

    sceneInfo
        .rasterizer_state
        .cull_mode =
        SDL_GPU_CULLMODE_NONE;

    sceneInfo
        .rasterizer_state
        .front_face =
        SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

    sceneInfo
        .rasterizer_state
        .enable_depth_clip =
        true;

    sceneInfo
        .multisample_state
        .sample_count =
        SDL_GPU_SAMPLECOUNT_1;

    sceneInfo
        .depth_stencil_state
        .enable_depth_test =
        true;

    sceneInfo
        .depth_stencil_state
        .enable_depth_write =
        true;

    sceneInfo
        .depth_stencil_state
        .compare_op =
        SDL_GPU_COMPAREOP_LESS;

    sceneInfo
        .target_info
        .num_color_targets =
        1;

    sceneInfo
        .target_info
        .color_target_descriptions =
        &sceneColorTarget;

    sceneInfo
        .target_info
        .has_depth_stencil_target =
        true;

    sceneInfo
        .target_info
        .depth_stencil_format =
        SDL_GPU_TEXTUREFORMAT_D16_UNORM;

    scenePipeline =
        SDL_CreateGPUGraphicsPipeline(
            device,
            &sceneInfo
        );

    // =====================================================
    // ASCII Post Process Pipeline
    // =====================================================

    SDL_GPUColorTargetDescription
        postColorTarget{};

    postColorTarget.format =
        SDL_GetGPUSwapchainTextureFormat(
            device,
            window
        );

    postColorTarget
        .blend_state
        .enable_blend =
        false;

    postColorTarget
        .blend_state
        .color_write_mask =
        0xF;

    SDL_GPUGraphicsPipelineCreateInfo
        postInfo{};

    postInfo.vertex_shader =
        fullscreenVertexShader;

    postInfo.fragment_shader =
        postFragmentShader;

    postInfo.primitive_type =
        SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

    postInfo
        .rasterizer_state
        .fill_mode =
        SDL_GPU_FILLMODE_FILL;

    postInfo
        .rasterizer_state
        .cull_mode =
        SDL_GPU_CULLMODE_NONE;

    postInfo
        .rasterizer_state
        .front_face =
        SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

    postInfo
        .rasterizer_state
        .enable_depth_clip =
        true;

    postInfo
        .multisample_state
        .sample_count =
        SDL_GPU_SAMPLECOUNT_1;

    postInfo
        .target_info
        .num_color_targets =
        1;

    postInfo
        .target_info
        .color_target_descriptions =
        &postColorTarget;

    postInfo
        .target_info
        .has_depth_stencil_target =
        false;

    postProcessPipeline =
        SDL_CreateGPUGraphicsPipeline(
            device,
            &postInfo
        );

    SDL_ReleaseGPUShader(
        device,
        sceneVertexShader
    );

    SDL_ReleaseGPUShader(
        device,
        sceneFragmentShader
    );

    SDL_ReleaseGPUShader(
        device,
        fullscreenVertexShader
    );

    SDL_ReleaseGPUShader(
        device,
        postFragmentShader
    );

    if (
        scenePipeline == nullptr
    )
    {
        SDL_Log(
            "Scene pipeline failed: %s",
            SDL_GetError()
        );

        return false;
    }

    if (
        postProcessPipeline == nullptr
    )
    {
        SDL_Log(
            "ASCII pipeline failed: %s",
            SDL_GetError()
        );

        return false;
    }

    return true;
}

// =========================================================
// Depth Texture
// =========================================================

SDL_GPUTexture* Renderer::CreateDepthTexture(
    Uint32 width,
    Uint32 height
)
{
    SDL_GPUTextureCreateInfo info{};

    info.type =
        SDL_GPU_TEXTURETYPE_2D;

    info.format =
        SDL_GPU_TEXTUREFORMAT_D16_UNORM;

    info.usage =
        SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;

    info.width =
        width;

    info.height =
        height;

    info.layer_count_or_depth =
        1;

    info.num_levels =
        1;

    info.sample_count =
        SDL_GPU_SAMPLECOUNT_1;

    return SDL_CreateGPUTexture(
        device,
        &info
    );
}

// =========================================================
// Offscreen Texture
// =========================================================

SDL_GPUTexture* Renderer::CreateOffscreenColorTexture(
    Uint32 width,
    Uint32 height
)
{
    SDL_GPUTextureCreateInfo info{};

    info.type =
        SDL_GPU_TEXTURETYPE_2D;

    info.format =
        SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

    info.usage =
        SDL_GPU_TEXTUREUSAGE_COLOR_TARGET |
        SDL_GPU_TEXTUREUSAGE_SAMPLER;

    info.width =
        width;

    info.height =
        height;

    info.layer_count_or_depth =
        1;

    info.num_levels =
        1;

    info.sample_count =
        SDL_GPU_SAMPLECOUNT_1;

    return SDL_CreateGPUTexture(
        device,
        &info
    );
}

// =========================================================
// Draw Entity
// =========================================================

void Renderer::DrawEntity(
    SDL_GPUCommandBuffer* commandBuffer,
    SDL_GPURenderPass* renderPass,
    const glm::mat4& viewProjection,
    const Entity& entity
)
{
    if (
        !entity.visible
    )
    {
        return;
    }

    const glm::mat4 model =
        entity.transform.GetMatrix();

    TransformUniform transformData{};

    transformData.mvp =
        viewProjection *
        model;

    transformData.normalMatrix =
        glm::transpose(
            glm::inverse(
                model
            )
        );

    SDL_PushGPUVertexUniformData(
        commandBuffer,
        0,
        &transformData,
        sizeof(TransformUniform)
    );

    SDL_PushGPUVertexUniformData(
        commandBuffer,
        1,
        glm::value_ptr(
            entity.color
        ),
        sizeof(glm::vec4)
    );

    SDL_DrawGPUPrimitives(
        renderPass,
        36,
        1,
        0,
        0
    );
}

// =========================================================
// Render
// =========================================================

bool Renderer::Render(
    const Scene& scene,
    const Camera& camera
)
{
    SDL_GPUCommandBuffer* commandBuffer =
        SDL_AcquireGPUCommandBuffer(
            device
        );

    if (
        commandBuffer == nullptr
    )
    {
        return false;
    }

    SDL_GPUTexture* swapchain =
        nullptr;

    Uint32 width =
        0;

    Uint32 height =
        0;

    if (
        !SDL_WaitAndAcquireGPUSwapchainTexture(
            commandBuffer,
            window,
            &swapchain,
            &width,
            &height
        )
    )
    {
        SDL_CancelGPUCommandBuffer(
            commandBuffer
        );

        return false;
    }

    if (
        swapchain == nullptr ||
        width == 0 ||
        height == 0
    )
    {
        SDL_CancelGPUCommandBuffer(
            commandBuffer
        );

        return true;
    }

    // =====================================================
    // Resize Targets
    // =====================================================

    if (
        depthTexture == nullptr ||
        offscreenColorTexture == nullptr ||
        historyColorTexture == nullptr ||
        width != renderWidth ||
        height != renderHeight
    )
    {
        if (
            depthTexture != nullptr
        )
        {
            SDL_ReleaseGPUTexture(
                device,
                depthTexture
            );

            depthTexture =
                nullptr;
        }

        if (
            offscreenColorTexture != nullptr
        )
        {
            SDL_ReleaseGPUTexture(
                device,
                offscreenColorTexture
            );

            offscreenColorTexture =
                nullptr;
        }

        if (
            historyColorTexture != nullptr
        )
        {
            SDL_ReleaseGPUTexture(
                device,
                historyColorTexture
            );

            historyColorTexture =
                nullptr;
        }

        depthTexture =
            CreateDepthTexture(
                width,
                height
            );

        offscreenColorTexture =
            CreateOffscreenColorTexture(
                width,
                height
            );

        historyColorTexture =
            CreateOffscreenColorTexture(
                width,
                height
            );

        historyValid =
            false;

        if (
            depthTexture == nullptr ||
            offscreenColorTexture == nullptr ||
            historyColorTexture == nullptr
        )
        {
            SDL_CancelGPUCommandBuffer(
                commandBuffer
            );

            return false;
        }

        renderWidth =
            width;

        renderHeight =
            height;
    }

    // =====================================================
    // Camera
    // =====================================================

    const float aspect =
        static_cast<float>(
            width
        ) /
        static_cast<float>(
            height
        );

    const glm::mat4 viewProjection =
        camera.GetViewProjection(
            aspect
        );

    // =====================================================
    // PASS 1
    // 3D WORLD -> OFFSCREEN
    // =====================================================

    SDL_GPUColorTargetInfo
        sceneColorTarget{};

    sceneColorTarget.texture =
        offscreenColorTexture;

    sceneColorTarget.clear_color =
    {
        0.015f,
        0.008f,
        0.025f,
        1.0f
    };

    sceneColorTarget.load_op =
        SDL_GPU_LOADOP_CLEAR;

    sceneColorTarget.store_op =
        SDL_GPU_STOREOP_STORE;

    SDL_GPUDepthStencilTargetInfo
        depthTarget{};

    depthTarget.texture =
        depthTexture;

    depthTarget.clear_depth =
        1.0f;

    depthTarget.load_op =
        SDL_GPU_LOADOP_CLEAR;

    depthTarget.store_op =
        SDL_GPU_STOREOP_DONT_CARE;

    depthTarget.stencil_load_op =
        SDL_GPU_LOADOP_DONT_CARE;

    depthTarget.stencil_store_op =
        SDL_GPU_STOREOP_DONT_CARE;

    SDL_GPURenderPass* scenePass =
        SDL_BeginGPURenderPass(
            commandBuffer,
            &sceneColorTarget,
            1,
            &depthTarget
        );

    if (
        scenePass == nullptr
    )
    {
        SDL_CancelGPUCommandBuffer(
            commandBuffer
        );

        return false;
    }

    SDL_BindGPUGraphicsPipeline(
        scenePass,
        scenePipeline
    );

    SDL_GPUBufferBinding
        vertexBinding{};

    vertexBinding.buffer =
        vertexBuffer;

    vertexBinding.offset =
        0;

    SDL_BindGPUVertexBuffers(
        scenePass,
        0,
        &vertexBinding,
        1
    );

    LightUniform light{};

    light.direction =
        glm::vec4(
            glm::normalize(
                glm::vec3(
                    -0.6f,
                    -1.0f,
                    -0.4f
                )
            ),
            0.0f
        );

    light.color =
    {
        1.0f,
        0.92f,
        0.82f,
        1.0f
    };

    light.params =
    {
        0.25f,
        0.90f,
        0.0f,
        0.0f
    };

    SDL_PushGPUFragmentUniformData(
        commandBuffer,
        0,
        &light,
        sizeof(LightUniform)
    );

    for (
        const Entity& entity :
        scene.GetEntities()
    )
    {
        DrawEntity(
            commandBuffer,
            scenePass,
            viewProjection,
            entity
        );
    }

    SDL_EndGPURenderPass(
        scenePass
    );

    // =====================================================
    // PASS 2
    // OFFSCREEN -> ASCII -> SCREEN
    // =====================================================

    SDL_GPUColorTargetInfo
        screenTarget{};

    screenTarget.texture =
        swapchain;

    screenTarget.clear_color =
    {
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };

    screenTarget.load_op =
        SDL_GPU_LOADOP_CLEAR;

    screenTarget.store_op =
        SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* postPass =
        SDL_BeginGPURenderPass(
            commandBuffer,
            &screenTarget,
            1,
            nullptr
        );

    if (
        postPass == nullptr
    )
    {
        SDL_CancelGPUCommandBuffer(
            commandBuffer
        );

        return false;
    }

    SDL_BindGPUGraphicsPipeline(
        postPass,
        postProcessPipeline
    );

    SDL_GPUTextureSamplerBinding
        samplerBindings[3]{};

    // 3D scene

    samplerBindings[0].texture =
        offscreenColorTexture;

    samplerBindings[0].sampler =
        offscreenSampler;

    // ASCII glyph atlas

    samplerBindings[1].texture =
        asciiAtlasTexture;

    samplerBindings[1].sampler =
        offscreenSampler;

    // Previous 3D frame for temporal stabilization

    samplerBindings[2].texture =
        historyColorTexture;

    samplerBindings[2].sampler =
        offscreenSampler;

    SDL_BindGPUFragmentSamplers(
        postPass,
        0,
        samplerBindings,
        3
    );

    // =====================================================
    // ASCII Settings
    // =====================================================

    AsciiUniform asciiData{};

    asciiData.screenAndCell =
    {
        static_cast<float>(
            width
        ),

        static_cast<float>(
            height
        ),

        8.0f, // character width

        12.0f  // character height
    };

    asciiData.atlasInfo =
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

    asciiData.temporalInfo =
    {
        historyValid ? 0.58f : 0.0f,
        historyValid ? 1.0f : 0.0f,
        0.0f,
        0.0f
    };

    SDL_PushGPUFragmentUniformData(
        commandBuffer,
        0,
        &asciiData,
        sizeof(AsciiUniform)
    );

    SDL_DrawGPUPrimitives(
        postPass,
        3,
        1,
        0,
        0
    );

    SDL_EndGPURenderPass(
        postPass
    );

    // =====================================================
    // Save current 3D frame as history
    // =====================================================

    SDL_GPUCopyPass* historyCopyPass =
        SDL_BeginGPUCopyPass(
            commandBuffer
        );

    if (
        historyCopyPass == nullptr
    )
    {
        SDL_CancelGPUCommandBuffer(
            commandBuffer
        );

        return false;
    }

    SDL_GPUTextureLocation historySource{};

    historySource.texture =
        offscreenColorTexture;

    historySource.mip_level =
        0;

    historySource.layer =
        0;

    historySource.x =
        0;

    historySource.y =
        0;

    historySource.z =
        0;

    SDL_GPUTextureLocation historyDestination{};

    historyDestination.texture =
        historyColorTexture;

    historyDestination.mip_level =
        0;

    historyDestination.layer =
        0;

    historyDestination.x =
        0;

    historyDestination.y =
        0;

    historyDestination.z =
        0;

    SDL_CopyGPUTextureToTexture(
        historyCopyPass,
        &historySource,
        &historyDestination,
        width,
        height,
        1,
        false
    );

    SDL_EndGPUCopyPass(
        historyCopyPass
    );

    // =====================================================
    // Submit
    // =====================================================

    if (
        !SDL_SubmitGPUCommandBuffer(
            commandBuffer
        )
    )
    {
        SDL_Log(
            "GPU submit failed: %s",
            SDL_GetError()
        );

        return false;
    }

    historyValid =
        true;

    return true;
}

// =========================================================
// Shutdown
// =========================================================

void Renderer::Shutdown()
{
    if (
        device == nullptr
    )
    {
        return;
    }

    SDL_WaitForGPUIdle(
        device
    );

    if (
        depthTexture != nullptr
    )
    {
        SDL_ReleaseGPUTexture(
            device,
            depthTexture
        );

        depthTexture =
            nullptr;
    }

    if (
        offscreenColorTexture != nullptr
    )
    {
        SDL_ReleaseGPUTexture(
            device,
            offscreenColorTexture
        );

        offscreenColorTexture =
            nullptr;
    }

    if (
        historyColorTexture != nullptr
    )
    {
        SDL_ReleaseGPUTexture(
            device,
            historyColorTexture
        );

        historyColorTexture =
            nullptr;
    }

    historyValid =
        false;

    if (
        asciiAtlasTexture != nullptr
    )
    {
        SDL_ReleaseGPUTexture(
            device,
            asciiAtlasTexture
        );

        asciiAtlasTexture =
            nullptr;
    }

    if (
        offscreenSampler != nullptr
    )
    {
        SDL_ReleaseGPUSampler(
            device,
            offscreenSampler
        );

        offscreenSampler =
            nullptr;
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
        scenePipeline != nullptr
    )
    {
        SDL_ReleaseGPUGraphicsPipeline(
            device,
            scenePipeline
        );

        scenePipeline =
            nullptr;
    }

    if (
        postProcessPipeline != nullptr
    )
    {
        SDL_ReleaseGPUGraphicsPipeline(
            device,
            postProcessPipeline
        );

        postProcessPipeline =
            nullptr;
    }

    if (
        window != nullptr
    )
    {
        SDL_ReleaseWindowFromGPUDevice(
            device,
            window
        );

        window =
            nullptr;
    }

    SDL_DestroyGPUDevice(
        device
    );

    device =
        nullptr;

    renderWidth =
        0;

    renderHeight =
        0;
}
