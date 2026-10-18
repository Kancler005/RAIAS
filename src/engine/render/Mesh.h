#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <cstdint>
#include <vector>

// =========================================================
// Generic vertex used by RAI meshes.
//
// UV is already present now so the next glTF/material
// milestones do not require changing the vertex format again.
// =========================================================

struct MeshVertex
{
    float px;
    float py;
    float pz;

    float nx;
    float ny;
    float nz;

    float u;
    float v;
};

class Mesh
{
public:
    Mesh() = default;

    Mesh(
        const Mesh&
    ) = delete;

    Mesh& operator=(
        const Mesh&
    ) = delete;

    bool Create(
        SDL_GPUDevice* device,
        const std::vector<MeshVertex>& vertices,
        const std::vector<std::uint32_t>& indices
    );

    bool CreateCube(
        SDL_GPUDevice* device
    );

    void Draw(
        SDL_GPURenderPass* renderPass
    ) const;

    void Destroy();

    [[nodiscard]]
    bool IsValid() const
    {
        return vertexBuffer != nullptr;
    }

    [[nodiscard]]
    Uint32 GetVertexCount() const
    {
        return vertexCount;
    }

    [[nodiscard]]
    Uint32 GetIndexCount() const
    {
        return indexCount;
    }

private:
    SDL_GPUDevice* device =
        nullptr;

    SDL_GPUBuffer* vertexBuffer =
        nullptr;

    SDL_GPUBuffer* indexBuffer =
        nullptr;

    Uint32 vertexCount =
        0;

    Uint32 indexCount =
        0;
};
