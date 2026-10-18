#include "ProceduralMeshes.h"

#include <glm/glm.hpp>

#include <array>
#include <cmath>
#include <vector>
#include <utility>

namespace
{
    constexpr float PI =
        3.14159265358979323846f;

    void AddTriangle(
        std::vector<MeshVertex>& vertices,
        glm::vec3 a,
        glm::vec3 b,
        glm::vec3 c
    )
    {
        glm::vec3 normal =
            glm::cross(
                b - a,
                c - a
            );

        const glm::vec3 center =
            (
                a +
                b +
                c
            ) /
            3.0f;

        // Keep normals facing away from the local origin.
        if (
            glm::dot(
                normal,
                center
            ) <
            0.0f
        )
        {
            std::swap(
                b,
                c
            );

            normal =
                -normal;
        }

        const float lengthSquared =
            glm::dot(
                normal,
                normal
            );

        if (
            lengthSquared <
            0.000001f
        )
        {
            normal =
            {
                0.0f,
                1.0f,
                0.0f
            };
        }
        else
        {
            normal =
                glm::normalize(
                    normal
                );
        }

        vertices.push_back(
            {
                a.x,
                a.y,
                a.z,
                normal.x,
                normal.y,
                normal.z,
                0.0f,
                0.0f
            }
        );

        vertices.push_back(
            {
                b.x,
                b.y,
                b.z,
                normal.x,
                normal.y,
                normal.z,
                1.0f,
                0.0f
            }
        );

        vertices.push_back(
            {
                c.x,
                c.y,
                c.z,
                normal.x,
                normal.y,
                normal.z,
                0.5f,
                1.0f
            }
        );
    }

    void AddQuad(
        std::vector<MeshVertex>& vertices,
        const glm::vec3& a,
        const glm::vec3& b,
        const glm::vec3& c,
        const glm::vec3& d
    )
    {
        AddTriangle(
            vertices,
            a,
            b,
            c
        );

        AddTriangle(
            vertices,
            a,
            c,
            d
        );
    }

    void AddBox(
        std::vector<MeshVertex>& vertices,
        const glm::vec3& center,
        const glm::vec3& halfSize
    )
    {
        const float minX =
            center.x -
            halfSize.x;

        const float maxX =
            center.x +
            halfSize.x;

        const float minY =
            center.y -
            halfSize.y;

        const float maxY =
            center.y +
            halfSize.y;

        const float minZ =
            center.z -
            halfSize.z;

        const float maxZ =
            center.z +
            halfSize.z;

        const glm::vec3 p000
        {
            minX,
            minY,
            minZ
        };

        const glm::vec3 p100
        {
            maxX,
            minY,
            minZ
        };

        const glm::vec3 p110
        {
            maxX,
            maxY,
            minZ
        };

        const glm::vec3 p010
        {
            minX,
            maxY,
            minZ
        };

        const glm::vec3 p001
        {
            minX,
            minY,
            maxZ
        };

        const glm::vec3 p101
        {
            maxX,
            minY,
            maxZ
        };

        const glm::vec3 p111
        {
            maxX,
            maxY,
            maxZ
        };

        const glm::vec3 p011
        {
            minX,
            maxY,
            maxZ
        };

        AddQuad(
            vertices,
            p000,
            p100,
            p110,
            p010
        );

        AddQuad(
            vertices,
            p101,
            p001,
            p011,
            p111
        );

        AddQuad(
            vertices,
            p001,
            p000,
            p010,
            p011
        );

        AddQuad(
            vertices,
            p100,
            p101,
            p111,
            p110
        );

        AddQuad(
            vertices,
            p010,
            p110,
            p111,
            p011
        );

        AddQuad(
            vertices,
            p001,
            p101,
            p100,
            p000
        );
    }

    bool CreateRock(
        SDL_GPUDevice* device,
        Mesh& mesh,
        const std::array<float, 8>& lowerRadii,
        const std::array<float, 8>& upperRadii,
        const std::array<float, 8>& upperHeights,
        float baseY,
        float topY
    )
    {
        std::vector<MeshVertex>
            vertices;

        std::array<glm::vec3, 8>
            lower{};

        std::array<glm::vec3, 8>
            upper{};

        for (
            int i = 0;
            i < 8;
            ++i
        )
        {
            const float angle =
                static_cast<float>(
                    i
                ) /
                8.0f *
                PI *
                2.0f;

            lower[i] =
            {
                std::cos(
                    angle
                ) *
                    lowerRadii[i],

                baseY,

                std::sin(
                    angle
                ) *
                    lowerRadii[i]
            };

            upper[i] =
            {
                std::cos(
                    angle +
                    0.10f
                ) *
                    upperRadii[i],

                upperHeights[i],

                std::sin(
                    angle +
                    0.10f
                ) *
                    upperRadii[i]
            };
        }

        for (
            int i = 0;
            i < 8;
            ++i
        )
        {
            const int next =
                (
                    i + 1
                ) %
                8;

            AddTriangle(
                vertices,
                lower[i],
                lower[next],
                upper[next]
            );

            AddTriangle(
                vertices,
                lower[i],
                upper[next],
                upper[i]
            );
        }

        const glm::vec3 top
        {
            0.08f,
            topY,
            -0.06f
        };

        for (
            int i = 0;
            i < 8;
            ++i
        )
        {
            const int next =
                (
                    i + 1
                ) %
                8;

            AddTriangle(
                vertices,
                upper[i],
                upper[next],
                top
            );
        }

        const glm::vec3 bottom
        {
            0.0f,
            baseY -
                0.04f,
            0.0f
        };

        for (
            int i = 0;
            i < 8;
            ++i
        )
        {
            const int next =
                (
                    i + 1
                ) %
                8;

            AddTriangle(
                vertices,
                bottom,
                lower[next],
                lower[i]
            );
        }

        return mesh.Create(
            device,
            vertices,
            {}
        );
    }
}

// ============================================================
// Rocks
// ============================================================

bool ProceduralMeshes::CreateRockSmall(
    SDL_GPUDevice* device,
    Mesh& mesh
)
{
    return CreateRock(
        device,
        mesh,
        {
            0.52f,
            0.61f,
            0.55f,
            0.48f,
            0.58f,
            0.51f,
            0.63f,
            0.49f
        },
        {
            0.34f,
            0.40f,
            0.31f,
            0.38f,
            0.35f,
            0.42f,
            0.30f,
            0.37f
        },
        {
            0.24f,
            0.30f,
            0.28f,
            0.35f,
            0.27f,
            0.32f,
            0.25f,
            0.31f
        },
        -0.46f,
        0.58f
    );
}

bool ProceduralMeshes::CreateRockWide(
    SDL_GPUDevice* device,
    Mesh& mesh
)
{
    return CreateRock(
        device,
        mesh,
        {
            0.86f,
            0.74f,
            0.92f,
            0.78f,
            0.88f,
            0.81f,
            0.95f,
            0.76f
        },
        {
            0.58f,
            0.63f,
            0.55f,
            0.61f,
            0.51f,
            0.64f,
            0.57f,
            0.60f
        },
        {
            0.22f,
            0.33f,
            0.29f,
            0.36f,
            0.25f,
            0.31f,
            0.27f,
            0.34f
        },
        -0.44f,
        0.67f
    );
}

bool ProceduralMeshes::CreateRockTall(
    SDL_GPUDevice* device,
    Mesh& mesh
)
{
    return CreateRock(
        device,
        mesh,
        {
            0.49f,
            0.56f,
            0.47f,
            0.53f,
            0.44f,
            0.58f,
            0.50f,
            0.46f
        },
        {
            0.42f,
            0.37f,
            0.43f,
            0.35f,
            0.39f,
            0.44f,
            0.36f,
            0.41f
        },
        {
            0.63f,
            0.71f,
            0.66f,
            0.77f,
            0.62f,
            0.73f,
            0.68f,
            0.75f
        },
        -0.47f,
        1.23f
    );
}

// ============================================================
// Ruined Shrine
// ============================================================

bool ProceduralMeshes::CreateRuinedShrine(
    SDL_GPUDevice* device,
    Mesh& mesh
)
{
    std::vector<MeshVertex>
        vertices;

    // Foundation.
    AddBox(
        vertices,
        {
            0.0f,
            0.14f,
            0.0f
        },
        {
            2.55f,
            0.14f,
            1.90f
        }
    );

    // Broken rear wall.
    AddBox(
        vertices,
        {
            0.0f,
            1.45f,
            1.52f
        },
        {
            2.20f,
            1.28f,
            0.20f
        }
    );

    // Missing chunk effect:
    // build side wall sections instead of a closed box.
    AddBox(
        vertices,
        {
            -2.12f,
            1.05f,
            0.44f
        },
        {
            0.18f,
            0.92f,
            1.05f
        }
    );

    AddBox(
        vertices,
        {
            2.12f,
            0.92f,
            0.72f
        },
        {
            0.18f,
            0.79f,
            0.78f
        }
    );

    // Front columns.
    AddBox(
        vertices,
        {
            -1.66f,
            1.42f,
            -1.34f
        },
        {
            0.23f,
            1.42f,
            0.23f
        }
    );

    AddBox(
        vertices,
        {
            1.66f,
            1.18f,
            -1.34f
        },
        {
            0.23f,
            1.18f,
            0.23f
        }
    );

    // Broken lintel.
    AddBox(
        vertices,
        {
            -0.82f,
            2.79f,
            -1.34f
        },
        {
            0.82f,
            0.17f,
            0.24f
        }
    );

    AddBox(
        vertices,
        {
            1.15f,
            2.44f,
            -1.34f
        },
        {
            0.43f,
            0.17f,
            0.24f
        }
    );

    // Altar.
    AddBox(
        vertices,
        {
            0.0f,
            0.60f,
            0.96f
        },
        {
            0.78f,
            0.46f,
            0.46f
        }
    );

    // Broken stone slabs.
    AddBox(
        vertices,
        {
            -0.95f,
            0.27f,
            -0.30f
        },
        {
            0.55f,
            0.14f,
            0.32f
        }
    );

    AddBox(
        vertices,
        {
            1.20f,
            0.24f,
            -0.08f
        },
        {
            0.42f,
            0.12f,
            0.54f
        }
    );

    return mesh.Create(
        device,
        vertices,
        {}
    );
}

// ============================================================
// Broken Watchtower
// ============================================================

bool ProceduralMeshes::CreateBrokenWatchtower(
    SDL_GPUDevice* device,
    Mesh& mesh
)
{
    std::vector<MeshVertex>
        vertices;

    // Base.
    AddBox(
        vertices,
        {
            0.0f,
            0.20f,
            0.0f
        },
        {
            1.65f,
            0.20f,
            1.65f
        }
    );

    // Main tower core.
    AddBox(
        vertices,
        {
            0.0f,
            2.10f,
            0.0f
        },
        {
            1.30f,
            1.90f,
            1.30f
        }
    );

    // Slightly wider upper ledge.
    AddBox(
        vertices,
        {
            0.0f,
            3.90f,
            0.0f
        },
        {
            1.52f,
            0.20f,
            1.52f
        }
    );

    // Uneven battlements.
    const std::array<glm::vec3, 7>
        battlements =
    {{
        {-1.12f, 4.42f, -1.13f},
        { 0.00f, 4.58f, -1.13f},
        { 1.12f, 4.30f, -1.13f},
        {-1.12f, 4.50f,  1.13f},
        { 0.08f, 4.24f,  1.13f},
        { 1.12f, 4.55f,  1.13f},
        {-1.13f, 4.36f,  0.00f}
    }};

    for (
        const glm::vec3& position :
        battlements
    )
    {
        AddBox(
            vertices,
            position,
            {
                0.28f,
                0.42f,
                0.28f
            }
        );
    }

    // Broken side buttresses.
    AddBox(
        vertices,
        {
            -1.47f,
            1.24f,
            0.72f
        },
        {
            0.20f,
            1.04f,
            0.37f
        }
    );

    AddBox(
        vertices,
        {
            1.47f,
            0.94f,
            -0.58f
        },
        {
            0.20f,
            0.74f,
            0.42f
        }
    );

    // Rubble pieces at base.
    AddBox(
        vertices,
        {
            -1.82f,
            0.25f,
            -0.42f
        },
        {
            0.45f,
            0.22f,
            0.34f
        }
    );

    AddBox(
        vertices,
        {
            1.62f,
            0.18f,
            1.18f
        },
        {
            0.38f,
            0.16f,
            0.43f
        }
    );

    return mesh.Create(
        device,
        vertices,
        {}
    );
}
