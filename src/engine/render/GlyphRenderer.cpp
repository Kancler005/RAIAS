#include "GlyphRenderer.h"

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <vector>

namespace
{
    constexpr Uint32 ASCII_GLYPH_COUNT =
        32;

    constexpr Uint32 ASCII_ATLAS_WIDTH =
        256;

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

        vertices.push_back(
            topLeft
        );

        vertices.push_back(
            topRight
        );

        vertices.push_back(
            bottomRight
        );

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
// Hash / Noise
// ============================================================

Uint32 GlyphRenderer::HashGrid(
    int x,
    int z,
    Uint32 seed
)
{
    Uint32 value =
        static_cast<Uint32>(
            x * 374761393
        ) ^
        static_cast<Uint32>(
            z * 668265263
        ) ^
        seed;

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

float GlyphRenderer::SmoothStep(
    float value
)
{
    value =
        std::clamp(
            value,
            0.0f,
            1.0f
        );

    return
        value *
        value *
        (
            3.0f -
            2.0f *
            value
        );
}

float GlyphRenderer::ValueNoise(
    float x,
    float z,
    float frequency,
    Uint32 seed
)
{
    const float sampleX =
        x *
        frequency;

    const float sampleZ =
        z *
        frequency;

    const int x0 =
        static_cast<int>(
            std::floor(
                sampleX
            )
        );

    const int z0 =
        static_cast<int>(
            std::floor(
                sampleZ
            )
        );

    const int x1 =
        x0 + 1;

    const int z1 =
        z0 + 1;

    const float tx =
        SmoothStep(
            sampleX -
            static_cast<float>(
                x0
            )
        );

    const float tz =
        SmoothStep(
            sampleZ -
            static_cast<float>(
                z0
            )
        );

    const float v00 =
        Hash01(
            HashGrid(
                x0,
                z0,
                seed
            )
        );

    const float v10 =
        Hash01(
            HashGrid(
                x1,
                z0,
                seed
            )
        );

    const float v01 =
        Hash01(
            HashGrid(
                x0,
                z1,
                seed
            )
        );

    const float v11 =
        Hash01(
            HashGrid(
                x1,
                z1,
                seed
            )
        );

    const float a =
        v00 +
        (
            v10 -
            v00
        ) *
        tx;

    const float b =
        v01 +
        (
            v11 -
            v01
        ) *
        tx;

    return
        a +
        (
            b -
            a
        ) *
        tz;
}

// ============================================================
// Material Glyph
// ============================================================

void GlyphRenderer::AppendMaterialGlyph(
    std::vector<GlyphVertex>& vertices,
    GlyphMaterialKind material,
    const glm::vec3& center,
    Uint32 hash,
    float materialStrength
)
{
    float glyph =
        1.0f;

    float size =
        1.0f;

    glm::vec4 color
    {
        0.22f,
        0.22f,
        0.20f,
        1.0f
    };

    const float variationA =
        Hash01(
            hash ^
            0xA51E3B27u
        );

    const float variationB =
        Hash01(
            hash ^
            0xD73184C9u
        );

    const float variationC =
        Hash01(
            hash ^
            0x379B12EFu
        );

    switch (
        material
    )
    {
        case GlyphMaterialKind::AshSoil:
        {
            // . : , - with rare +
            const Uint32 choice =
                (
                    hash >>
                    8
                ) %
                12u;

            if (
                choice < 4u
            )
            {
                glyph =
                    1.0f; // .
            }
            else if (
                choice < 7u
            )
            {
                glyph =
                    2.0f; // :
            }
            else if (
                choice < 9u
            )
            {
                glyph =
                    8.0f; // ,
            }
            else if (
                choice < 11u
            )
            {
                glyph =
                    3.0f; // -
            }
            else
            {
                glyph =
                    4.0f; // +
            }

            const glm::vec3 coldAsh
            {
                0.25f,
                0.25f,
                0.24f
            };

            const glm::vec3 dirtyBrown
            {
                0.29f,
                0.24f,
                0.18f
            };

            const glm::vec3 mixed =
                coldAsh +
                (
                    dirtyBrown -
                    coldAsh
                ) *
                variationA;

            color =
            {
                mixed.r,
                mixed.g,
                mixed.b,
                1.0f
            };

            size =
                0.68f +
                variationB *
                0.44f;

            ++ashGlyphCount;

            break;
        }

        case GlyphMaterialKind::DeadGrass:
        {
            // Dedicated environment glyphs:
            // , ' / \ | with rare *
            const Uint32 choice =
                (
                    hash >>
                    10
                ) %
                12u;

            if (
                choice < 2u
            )
            {
                glyph =
                    8.0f;  // ,
            }
            else if (
                choice < 5u
            )
            {
                glyph =
                    9.0f;  // '
            }
            else if (
                choice < 7u
            )
            {
                glyph =
                    10.0f; // /
            }
            else if (
                choice < 9u
            )
            {
                glyph =
                    11.0f; // backslash glyph
            }
            else if (
                choice < 11u
            )
            {
                glyph =
                    12.0f; // |
            }
            else
            {
                glyph =
                    5.0f;  // *
            }

            const glm::vec3 deadOlive
            {
                0.25f,
                0.29f,
                0.16f
            };

            const glm::vec3 dryGold
            {
                0.42f,
                0.34f,
                0.16f
            };

            const glm::vec3 mixed =
                deadOlive +
                (
                    dryGold -
                    deadOlive
                ) *
                variationA;

            color =
            {
                mixed.r,
                mixed.g,
                mixed.b,
                1.0f
            };

            size =
                1.08f +
                variationB *
                0.62f +
                materialStrength *
                0.22f;

            ++grassGlyphCount;

            break;
        }

        case GlyphMaterialKind::Stone:
        {
            // Stone uses heavy marks plus ^ and %.
            const Uint32 choice =
                (
                    hash >>
                    12
                ) %
                10u;

            if (
                choice < 2u
            )
            {
                glyph =
                    3.0f;  // -
            }
            else if (
                choice < 4u
            )
            {
                glyph =
                    5.0f;  // *
            }
            else if (
                choice < 6u
            )
            {
                glyph =
                    6.0f;  // #
            }
            else if (
                choice < 8u
            )
            {
                glyph =
                    14.0f; // ^
            }
            else
            {
                glyph =
                    15.0f; // %
            }

            const glm::vec3 charcoal
            {
                0.27f,
                0.28f,
                0.29f
            };

            const glm::vec3 paleStone
            {
                0.43f,
                0.42f,
                0.38f
            };

            const glm::vec3 mixed =
                charcoal +
                (
                    paleStone -
                    charcoal
                ) *
                variationA;

            color =
            {
                mixed.r,
                mixed.g,
                mixed.b,
                1.0f
            };

            size =
                0.78f +
                variationB *
                0.62f;

            ++stoneGlyphCount;

            break;
        }
    }

    // Keep the world dark. Strong material pockets become more visible,
    // while weaker transitions remain subtle.
    const float brightness =
        0.70f +
        variationC *
        0.26f +
        materialStrength *
        0.18f;

    color.r *=
        brightness;

    color.g *=
        brightness;

    color.b *=
        brightness;

    AppendGlyphQuad(
        vertices,
        center,
        glyph,
        size,
        color
    );

    ++glyphCount;
}

// ============================================================
// Dark Fantasy Ground
// ============================================================

bool GlyphRenderer::CreateDarkFantasyGround(
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

    ashGlyphCount =
        0;

    grassGlyphCount =
        0;

    stoneGlyphCount =
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
            const Uint32 cellHash =
                HashGrid(
                    gridX,
                    gridZ,
                    0x11A7B5C3u
                );

            const float detail =
                Hash01(
                    cellHash
                );

            // =================================================
            // Low-frequency material fields.
            //
            // Two frequencies are mixed so patches do not look
            // like smooth circular "blobs".
            // =================================================

            const float broadA =
                ValueNoise(
                    x,
                    z,
                    0.34f,
                    0x4B91D23Fu
                );

            const float broadB =
                ValueNoise(
                    x + 12.7f,
                    z - 8.3f,
                    0.61f,
                    0xA13C7E59u
                );

            const float materialField =
                broadA *
                0.68f +
                broadB *
                0.32f;

            const float stoneField =
                ValueNoise(
                    x - 19.0f,
                    z + 31.0f,
                    0.48f,
                    0xC03F5A71u
                );

            const float emptyField =
                ValueNoise(
                    x + 4.0f,
                    z + 17.0f,
                    0.29f,
                    0x72D98B45u
                );

            // =================================================
            // Negative space
            //
            // The world should breathe. Some coherent patches have
            // no glyphs at all instead of filling every grid cell.
            // =================================================

            float density =
                0.58f;

            if (
                materialField >
                0.57f
            )
            {
                density +=
                    0.17f;
            }

            if (
                stoneField >
                0.69f
            )
            {
                density +=
                    0.08f;
            }

            if (
                emptyField <
                0.35f
            )
            {
                density -=
                    0.26f;
            }

            density =
                std::clamp(
                    density,
                    0.18f,
                    0.90f
                );

            if (
                detail >
                density
            )
            {
                continue;
            }

            // =================================================
            // Material selection
            // =================================================

            GlyphMaterialKind material =
                GlyphMaterialKind::AshSoil;

            float materialStrength =
                0.30f;

            if (
                stoneField >
                0.73f
            )
            {
                material =
                    GlyphMaterialKind::Stone;

                materialStrength =
                    std::clamp(
                        (
                            stoneField -
                            0.73f
                        ) /
                        0.27f,
                        0.0f,
                        1.0f
                    );
            }
            else if (
                materialField >
                0.55f
            )
            {
                material =
                    GlyphMaterialKind::DeadGrass;

                materialStrength =
                    std::clamp(
                        (
                            materialField -
                            0.55f
                        ) /
                        0.45f,
                        0.0f,
                        1.0f
                    );
            }
            else
            {
                materialStrength =
                    1.0f -
                    materialField;
            }

            // =================================================
            // Position jitter
            // =================================================

            const float jitterX =
                (
                    Hash01(
                        cellHash ^
                        0xA53A9E17u
                    ) -
                    0.5f
                ) *
                spacing *
                0.56f;

            const float jitterZ =
                (
                    Hash01(
                        cellHash ^
                        0x8D12F4A1u
                    ) -
                    0.5f
                ) *
                spacing *
                0.56f;

            float heightOffset =
                0.0f;

            switch (
                material
            )
            {
                case GlyphMaterialKind::AshSoil:
                {
                    heightOffset =
                        0.005f;

                    break;
                }

                case GlyphMaterialKind::DeadGrass:
                {
                    heightOffset =
                        0.018f +
                        materialStrength *
                        0.010f;

                    break;
                }

                case GlyphMaterialKind::Stone:
                {
                    heightOffset =
                        0.010f;

                    break;
                }
            }

            const glm::vec3 center
            {
                x +
                    jitterX,

                worldY +
                    heightOffset,

                z +
                    jitterZ
            };

            AppendMaterialGlyph(
                vertices,
                material,
                center,
                cellHash,
                materialStrength
            );

            // =================================================
            // Rare second grass tuft
            //
            // This makes grassy zones feel clustered without
            // increasing the base grid density everywhere.
            // =================================================

            if (
                material ==
                    GlyphMaterialKind::DeadGrass &&
                materialStrength >
                    0.38f &&
                Hash01(
                    cellHash ^
                    0xF08763B1u
                ) <
                    0.36f
            )
            {
                const glm::vec3 secondCenter
                {
                    center.x +
                        (
                            Hash01(
                                cellHash ^
                                0x33D17A5Bu
                            ) -
                            0.5f
                        ) *
                        spacing *
                        0.58f,

                    center.y +
                        0.006f,

                    center.z +
                        (
                            Hash01(
                                cellHash ^
                                0x67AF0C19u
                            ) -
                            0.5f
                        ) *
                        spacing *
                        0.58f
                };

                AppendMaterialGlyph(
                    vertices,
                    GlyphMaterialKind::DeadGrass,
                    secondCenter,
                    cellHash ^
                        0x913A6F2Du,
                    materialStrength
                );
            }
        }
    }

    // =====================================================
    // Fixed ambient campfire glyph effects.
    //
    // Campfire mesh bases live in the Scene at the same X/Z.
    // The bright warm colors are recognized by the vertex shader
    // and animated as flickering flame/spark glyphs.
    // =====================================================

    const std::array<glm::vec3, 3>
        campfirePositions =
    {{
        {
            1.8f,
            worldY,
            -6.3f
        },
        {
            -6.6f,
            worldY,
            3.6f
        },
        {
            8.2f,
            worldY,
            8.4f
        }
    }};

    for (
        std::size_t fireIndex = 0;
        fireIndex <
            campfirePositions.size();
        ++fireIndex
    )
    {
        const glm::vec3& fire =
            campfirePositions[
                fireIndex
            ];

        for (
            int layer = 0;
            layer < 9;
            ++layer
        )
        {
            const Uint32 fireHash =
                HashGrid(
                    static_cast<int>(
                        fireIndex
                    ) *
                        31 +
                    layer,
                    layer *
                        17,
                    0xF17ECA5Eu
                );

            const float angle =
                Hash01(
                    fireHash
                ) *
                6.28318530718f;

            const float radius =
                0.06f +
                Hash01(
                    fireHash ^
                    0xAAB31D7Fu
                ) *
                0.20f;

            const float height =
                0.36f +
                static_cast<float>(
                    layer
                ) *
                0.075f +
                Hash01(
                    fireHash ^
                    0x7321F09Bu
                ) *
                0.10f;

            const glm::vec3 center
            {
                fire.x +
                    std::cos(
                        angle
                    ) *
                    radius,

                fire.y +
                    height,

                fire.z +
                    std::sin(
                        angle
                    ) *
                    radius
            };

            float glyph =
                5.0f;

            switch (
                layer %
                4
            )
            {
                case 0:
                {
                    glyph =
                        4.0f; // +

                    break;
                }

                case 1:
                {
                    glyph =
                        5.0f; // *

                    break;
                }

                case 2:
                {
                    glyph =
                        14.0f; // ^

                    break;
                }

                default:
                {
                    glyph =
                        9.0f; // apostrophe spark

                    break;
                }
            }

            const float heat =
                Hash01(
                    fireHash ^
                    0x18C423D1u
                );

            const glm::vec4 color
            {
                0.95f,
                0.25f +
                    heat *
                    0.30f,
                0.035f,
                1.0f
            };

            const float size =
                0.78f +
                heat *
                0.46f;

            AppendGlyphQuad(
                vertices,
                center,
                glyph,
                size,
                color
            );

            ++glyphCount;
        }

        // A few rising sparks.
        for (
            int spark = 0;
            spark < 4;
            ++spark
        )
        {
            const Uint32 sparkHash =
                HashGrid(
                    spark +
                        static_cast<int>(
                            fireIndex
                        ) *
                        97,
                    spark *
                        29,
                    0x99431AC7u
                );

            const glm::vec3 sparkCenter
            {
                fire.x +
                    (
                        Hash01(
                            sparkHash
                        ) -
                        0.5f
                    ) *
                    0.56f,

                fire.y +
                    0.95f +
                    static_cast<float>(
                        spark
                    ) *
                    0.18f,

                fire.z +
                    (
                        Hash01(
                            sparkHash ^
                            0xCA7B104Fu
                        ) -
                        0.5f
                    ) *
                    0.56f
            };

            AppendGlyphQuad(
                vertices,
                sparkCenter,
                9.0f,
                0.62f,
                {
                    1.0f,
                    0.42f,
                    0.05f,
                    1.0f
                }
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

        ashGlyphCount =
            0;

        grassGlyphCount =
            0;

        stoneGlyphCount =
            0;

        return false;
    }

    SDL_Log(
        "RAI Glyph Materials generated | total=%u ash=%u grass=%u stone=%u",
        glyphCount,
        ashGlyphCount,
        grassGlyphCount,
        stoneGlyphCount
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

    const float timeSeconds =
        static_cast<float>(
            SDL_GetTicks()
        ) *
        0.001f;

    cameraData.glyphSettings =
    {
        0.18f,
        0.27f,
        timeSeconds,
        1.0f
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

    ashGlyphCount =
        0;

    grassGlyphCount =
        0;

    stoneGlyphCount =
        0;

    glyphAtlas =
        nullptr;

    glyphSampler =
        nullptr;

    device =
        nullptr;
}
