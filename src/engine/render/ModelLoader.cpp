#include "ModelLoader.h"

#include "Mesh.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <SDL3/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

namespace
{
    const cgltf_accessor* FindAttribute(
        const cgltf_primitive& primitive,
        cgltf_attribute_type type,
        cgltf_int index = 0
    )
    {
        for (
            cgltf_size i = 0;
            i < primitive.attributes_count;
            ++i
        )
        {
            const cgltf_attribute& attribute =
                primitive.attributes[i];

            if (
                attribute.type == type &&
                attribute.index == index
            )
            {
                return attribute.data;
            }
        }

        return nullptr;
    }

    bool ReadVec3(
        const cgltf_accessor* accessor,
        cgltf_size vertexIndex,
        glm::vec3& value
    )
    {
        if (
            accessor == nullptr
        )
        {
            return false;
        }

        cgltf_float temp[3] =
        {
            0.0f,
            0.0f,
            0.0f
        };

        if (
            !cgltf_accessor_read_float(
                accessor,
                vertexIndex,
                temp,
                3
            )
        )
        {
            return false;
        }

        value =
        {
            temp[0],
            temp[1],
            temp[2]
        };

        return true;
    }

    bool ReadVec2(
        const cgltf_accessor* accessor,
        cgltf_size vertexIndex,
        glm::vec2& value
    )
    {
        if (
            accessor == nullptr
        )
        {
            return false;
        }

        cgltf_float temp[2] =
        {
            0.0f,
            0.0f
        };

        if (
            !cgltf_accessor_read_float(
                accessor,
                vertexIndex,
                temp,
                2
            )
        )
        {
            return false;
        }

        value =
        {
            temp[0],
            temp[1]
        };

        return true;
    }

    void AppendPrimitive(
        const cgltf_primitive& primitive,
        const glm::mat4& nodeWorld,
        std::vector<MeshVertex>& vertices,
        std::vector<std::uint32_t>& indices
    )
    {
        if (
            primitive.type !=
            cgltf_primitive_type_triangles
        )
        {
            SDL_Log(
                "ModelLoader: skipping non-triangle primitive."
            );

            return;
        }

        const cgltf_accessor* positions =
            FindAttribute(
                primitive,
                cgltf_attribute_type_position
            );

        if (
            positions == nullptr ||
            positions->count == 0
        )
        {
            SDL_Log(
                "ModelLoader: primitive has no POSITION data."
            );

            return;
        }

        const cgltf_accessor* normals =
            FindAttribute(
                primitive,
                cgltf_attribute_type_normal
            );

        const cgltf_accessor* texcoords =
            FindAttribute(
                primitive,
                cgltf_attribute_type_texcoord,
                0
            );

        const std::uint32_t vertexBase =
            static_cast<std::uint32_t>(
                vertices.size()
            );

        const glm::mat3 normalMatrix =
            glm::transpose(
                glm::inverse(
                    glm::mat3(
                        nodeWorld
                    )
                )
            );

        for (
            cgltf_size i = 0;
            i < positions->count;
            ++i
        )
        {
            glm::vec3 localPosition{
                0.0f
            };

            if (
                !ReadVec3(
                    positions,
                    i,
                    localPosition
                )
            )
            {
                continue;
            }

            glm::vec3 localNormal{
                0.0f,
                1.0f,
                0.0f
            };

            if (
                normals != nullptr
            )
            {
                ReadVec3(
                    normals,
                    i,
                    localNormal
                );
            }

            glm::vec2 uv{
                0.0f,
                0.0f
            };

            if (
                texcoords != nullptr
            )
            {
                ReadVec2(
                    texcoords,
                    i,
                    uv
                );
            }

            glm::vec3 worldPosition =
                glm::vec3(
                    nodeWorld *
                    glm::vec4(
                        localPosition,
                        1.0f
                    )
                );

            glm::vec3 worldNormal =
                normalMatrix *
                localNormal;

            if (
                glm::dot(
                    worldNormal,
                    worldNormal
                ) >
                0.000001f
            )
            {
                worldNormal =
                    glm::normalize(
                        worldNormal
                    );
            }
            else
            {
                worldNormal =
                glm::vec3(
                    0.0f,
                    1.0f,
                    0.0f
                );
            }

            // glTF is right-handed.
            // RAI currently renders with a left-handed camera.
            worldPosition.z =
                -worldPosition.z;

            worldNormal.z =
                -worldNormal.z;

            MeshVertex vertex{};

            vertex.px =
                worldPosition.x;

            vertex.py =
                worldPosition.y;

            vertex.pz =
                worldPosition.z;

            vertex.nx =
                worldNormal.x;

            vertex.ny =
                worldNormal.y;

            vertex.nz =
                worldNormal.z;

            vertex.u =
                uv.x;

            vertex.v =
                uv.y;

            vertices.push_back(
                vertex
            );
        }

        // =================================================
        // Indices
        // =================================================

        if (
            primitive.indices != nullptr
        )
        {
            const cgltf_size indexCount =
                primitive.indices->count;

            // Reflection on Z changes winding.
            // Swap triangle vertices 1 and 2 to keep winding.
            for (
                cgltf_size i = 0;
                i + 2 < indexCount;
                i += 3
            )
            {
                const std::uint32_t i0 =
                    vertexBase +
                    static_cast<std::uint32_t>(
                        cgltf_accessor_read_index(
                            primitive.indices,
                            i
                        )
                    );

                const std::uint32_t i1 =
                    vertexBase +
                    static_cast<std::uint32_t>(
                        cgltf_accessor_read_index(
                            primitive.indices,
                            i + 1
                        )
                    );

                const std::uint32_t i2 =
                    vertexBase +
                    static_cast<std::uint32_t>(
                        cgltf_accessor_read_index(
                            primitive.indices,
                            i + 2
                        )
                    );

                indices.push_back(
                    i0
                );

                indices.push_back(
                    i2
                );

                indices.push_back(
                    i1
                );
            }
        }
        else
        {
            const std::uint32_t count =
                static_cast<std::uint32_t>(
                    positions->count
                );

            for (
                std::uint32_t i = 0;
                i + 2 < count;
                i += 3
            )
            {
                indices.push_back(
                    vertexBase + i
                );

                indices.push_back(
                    vertexBase + i + 2
                );

                indices.push_back(
                    vertexBase + i + 1
                );
            }
        }
    }

    void NormalizeGeometry(
        std::vector<MeshVertex>& vertices
    )
    {
        if (
            vertices.empty()
        )
        {
            return;
        }

        glm::vec3 minimum(
            std::numeric_limits<float>::max()
        );

        glm::vec3 maximum(
            std::numeric_limits<float>::lowest()
        );

        for (
            const MeshVertex& vertex :
            vertices
        )
        {
            const glm::vec3 position{
                vertex.px,
                vertex.py,
                vertex.pz
            };

            minimum =
                glm::min(
                    minimum,
                    position
                );

            maximum =
                glm::max(
                    maximum,
                    position
                );
        }

        const glm::vec3 center =
            (
                minimum +
                maximum
            ) *
            0.5f;

        const glm::vec3 size =
            maximum -
            minimum;

        const float largestDimension =
            std::max(
                size.x,
                std::max(
                    size.y,
                    size.z
                )
            );

        if (
            largestDimension <=
            0.000001f
        )
        {
            return;
        }

        const float scale =
            1.0f /
            largestDimension;

        for (
            MeshVertex& vertex :
            vertices
        )
        {
            glm::vec3 position{
                vertex.px,
                vertex.py,
                vertex.pz
            };

            position =
                (
                    position -
                    center
                ) *
                scale;

            // Place the normalized model on Y=0 instead
            // of centering it through the floor.
            const float normalizedBottom =
                (
                    minimum.y -
                    center.y
                ) *
                scale;

            position.y -=
                normalizedBottom;

            vertex.px =
                position.x;

            vertex.py =
                position.y;

            vertex.pz =
                position.z;
        }
    }

    void AppendMesh(
        const cgltf_mesh& mesh,
        const glm::mat4& world,
        std::vector<MeshVertex>& vertices,
        std::vector<std::uint32_t>& indices
    )
    {
        for (
            cgltf_size primitiveIndex = 0;
            primitiveIndex <
                mesh.primitives_count;
            ++primitiveIndex
        )
        {
            AppendPrimitive(
                mesh.primitives[
                    primitiveIndex
                ],
                world,
                vertices,
                indices
            );
        }
    }
}

// =========================================================
// Load GLB / glTF
// =========================================================

bool ModelLoader::LoadGLTF(
    SDL_GPUDevice* device,
    const std::string& path,
    Mesh& destination
)
{
    if (
        device == nullptr
    )
    {
        return false;
    }

    cgltf_options options{};

    cgltf_data* data =
        nullptr;

    cgltf_result result =
        cgltf_parse_file(
            &options,
            path.c_str(),
            &data
        );

    if (
        result !=
        cgltf_result_success
    )
    {
        SDL_Log(
            "ModelLoader: cgltf_parse_file failed (%d): %s",
            static_cast<int>(
                result
            ),
            path.c_str()
        );

        return false;
    }

    result =
        cgltf_load_buffers(
            &options,
            data,
            path.c_str()
        );

    if (
        result !=
        cgltf_result_success
    )
    {
        SDL_Log(
            "ModelLoader: cgltf_load_buffers failed (%d): %s",
            static_cast<int>(
                result
            ),
            path.c_str()
        );

        cgltf_free(
            data
        );

        return false;
    }

    result =
        cgltf_validate(
            data
        );

    if (
        result !=
        cgltf_result_success
    )
    {
        SDL_Log(
            "ModelLoader: validation failed (%d): %s",
            static_cast<int>(
                result
            ),
            path.c_str()
        );

        cgltf_free(
            data
        );

        return false;
    }

    std::vector<MeshVertex>
        vertices;

    std::vector<std::uint32_t>
        indices;

    // =====================================================
    // Load scene nodes so node transforms are respected.
    // =====================================================

    bool loadedFromNodes =
        false;

    for (
        cgltf_size nodeIndex = 0;
        nodeIndex <
            data->nodes_count;
        ++nodeIndex
    )
    {
        const cgltf_node& node =
            data->nodes[
                nodeIndex
            ];

        if (
            node.mesh ==
            nullptr
        )
        {
            continue;
        }

        cgltf_float worldValues[16];

        cgltf_node_transform_world(
            &node,
            worldValues
        );

        const glm::mat4 world =
            glm::make_mat4(
                worldValues
            );

        AppendMesh(
            *node.mesh,
            world,
            vertices,
            indices
        );

        loadedFromNodes =
            true;
    }

    // Some files may contain meshes without scene nodes.
    if (
        !loadedFromNodes
    )
    {
        const glm::mat4 identity{
            1.0f
        };

        for (
            cgltf_size meshIndex = 0;
            meshIndex <
                data->meshes_count;
            ++meshIndex
        )
        {
            AppendMesh(
                data->meshes[
                    meshIndex
                ],
                identity,
                vertices,
                indices
            );
        }
    }

    cgltf_free(
        data
    );

    if (
        vertices.empty()
    )
    {
        SDL_Log(
            "ModelLoader: no triangle geometry found: %s",
            path.c_str()
        );

        return false;
    }

    NormalizeGeometry(
        vertices
    );

    if (
        !destination.Create(
            device,
            vertices,
            indices
        )
    )
    {
        SDL_Log(
            "ModelLoader: GPU Mesh creation failed: %s",
            path.c_str()
        );

        return false;
    }

    SDL_Log(
        "GLTF loaded: %s | vertices=%zu indices=%zu",
        path.c_str(),
        vertices.size(),
        indices.size()
    );

    return true;
}
