#include "Mesh.h"

#include <cstring>

// =========================================================
// Create arbitrary Mesh
// =========================================================

bool Mesh::Create(
    SDL_GPUDevice* newDevice,
    const std::vector<MeshVertex>& vertices,
    const std::vector<std::uint32_t>& indices
)
{
    Destroy();

    if (
        newDevice == nullptr ||
        vertices.empty()
    )
    {
        SDL_Log(
            "Mesh::Create failed: invalid device or empty vertices."
        );

        return false;
    }

    device =
        newDevice;

    vertexCount =
        static_cast<Uint32>(
            vertices.size()
        );

    indexCount =
        static_cast<Uint32>(
            indices.size()
        );

    const Uint32 vertexBytes =
        static_cast<Uint32>(
            vertices.size() *
            sizeof(MeshVertex)
        );

    const Uint32 indexBytes =
        static_cast<Uint32>(
            indices.size() *
            sizeof(std::uint32_t)
        );

    // =====================================================
    // Vertex Buffer
    // =====================================================

    SDL_GPUBufferCreateInfo
        vertexInfo{};

    vertexInfo.usage =
        SDL_GPU_BUFFERUSAGE_VERTEX;

    vertexInfo.size =
        vertexBytes;

    vertexBuffer =
        SDL_CreateGPUBuffer(
            device,
            &vertexInfo
        );

    if (
        vertexBuffer == nullptr
    )
    {
        SDL_Log(
            "Mesh vertex buffer creation failed: %s",
            SDL_GetError()
        );

        Destroy();

        return false;
    }

    // =====================================================
    // Index Buffer
    // =====================================================

    if (
        indexCount > 0
    )
    {
        SDL_GPUBufferCreateInfo
            indexInfo{};

        indexInfo.usage =
            SDL_GPU_BUFFERUSAGE_INDEX;

        indexInfo.size =
            indexBytes;

        indexBuffer =
            SDL_CreateGPUBuffer(
                device,
                &indexInfo
            );

        if (
            indexBuffer == nullptr
        )
        {
            SDL_Log(
                "Mesh index buffer creation failed: %s",
                SDL_GetError()
            );

            Destroy();

            return false;
        }
    }

    // =====================================================
    // Transfer Buffer
    // =====================================================

    const Uint32 transferSize =
        vertexBytes +
        indexBytes;

    SDL_GPUTransferBufferCreateInfo
        transferInfo{};

    transferInfo.usage =
        SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;

    transferInfo.size =
        transferSize;

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
            "Mesh transfer buffer creation failed: %s",
            SDL_GetError()
        );

        Destroy();

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
        SDL_Log(
            "Mesh transfer buffer map failed: %s",
            SDL_GetError()
        );

        SDL_ReleaseGPUTransferBuffer(
            device,
            transfer
        );

        Destroy();

        return false;
    }

    std::memcpy(
        mapped,
        vertices.data(),
        vertexBytes
    );

    if (
        indexBytes > 0
    )
    {
        std::memcpy(
            static_cast<unsigned char*>(
                mapped
            ) +
            vertexBytes,
            indices.data(),
            indexBytes
        );
    }

    SDL_UnmapGPUTransferBuffer(
        device,
        transfer
    );

    // =====================================================
    // GPU Upload
    // =====================================================

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

        Destroy();

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

        Destroy();

        return false;
    }

    // -----------------------------------------------------
    // Upload vertices
    // -----------------------------------------------------

    SDL_GPUTransferBufferLocation
        vertexSource{};

    vertexSource.transfer_buffer =
        transfer;

    vertexSource.offset =
        0;

    SDL_GPUBufferRegion
        vertexDestination{};

    vertexDestination.buffer =
        vertexBuffer;

    vertexDestination.offset =
        0;

    vertexDestination.size =
        vertexBytes;

    SDL_UploadToGPUBuffer(
        copyPass,
        &vertexSource,
        &vertexDestination,
        false
    );

    // -----------------------------------------------------
    // Upload indices
    // -----------------------------------------------------

    if (
        indexBuffer != nullptr
    )
    {
        SDL_GPUTransferBufferLocation
            indexSource{};

        indexSource.transfer_buffer =
            transfer;

        indexSource.offset =
            vertexBytes;

        SDL_GPUBufferRegion
            indexDestination{};

        indexDestination.buffer =
            indexBuffer;

        indexDestination.offset =
            0;

        indexDestination.size =
            indexBytes;

        SDL_UploadToGPUBuffer(
            copyPass,
            &indexSource,
            &indexDestination,
            false
        );
    }

    SDL_EndGPUCopyPass(
        copyPass
    );

    if (
        !SDL_SubmitGPUCommandBuffer(
            commandBuffer
        )
    )
    {
        SDL_Log(
            "Mesh GPU upload submit failed: %s",
            SDL_GetError()
        );

        SDL_ReleaseGPUTransferBuffer(
            device,
            transfer
        );

        Destroy();

        return false;
    }

    // Initialization upload only.
    // Waiting here keeps transfer-buffer lifetime simple.
    SDL_WaitForGPUIdle(
        device
    );

    SDL_ReleaseGPUTransferBuffer(
        device,
        transfer
    );

    SDL_Log(
        "Mesh created | vertices=%u indices=%u",
        vertexCount,
        indexCount
    );

    return true;
}

// =========================================================
// Default Cube Mesh
//
// Important:
// Renderer no longer knows how cube geometry is built.
// Later this function is replaced by imported GLB/glTF meshes.
// =========================================================

bool Mesh::CreateCube(
    SDL_GPUDevice* newDevice
)
{
    const std::vector<MeshVertex> vertices =
    {
        // FRONT (-Z)

        {-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f},
        { 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f},
        { 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f},
        {-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f},

        // BACK (+Z)

        { 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f},
        {-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f},
        {-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f},
        { 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f},

        // LEFT (-X)

        {-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f},
        {-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f},
        {-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f},
        {-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f},

        // RIGHT (+X)

        { 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f},
        { 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f},
        { 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f},
        { 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f},

        // TOP (+Y)

        {-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f},
        { 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f},
        { 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f},
        {-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f},

        // BOTTOM (-Y)

        {-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f},
        { 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f},
        { 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f},
        {-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f}
    };

    const std::vector<std::uint32_t> indices =
    {
         0,  1,  2,   0,  2,  3,
         4,  5,  6,   4,  6,  7,
         8,  9, 10,   8, 10, 11,
        12, 13, 14,  12, 14, 15,
        16, 17, 18,  16, 18, 19,
        20, 21, 22,  20, 22, 23
    };

    return Create(
        newDevice,
        vertices,
        indices
    );
}

// =========================================================
// Draw
// =========================================================

void Mesh::Draw(
    SDL_GPURenderPass* renderPass
) const
{
    if (
        renderPass == nullptr ||
        vertexBuffer == nullptr ||
        vertexCount == 0
    )
    {
        return;
    }

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

    if (
        indexBuffer != nullptr &&
        indexCount > 0
    )
    {
        SDL_GPUBufferBinding
            indexBinding{};

        indexBinding.buffer =
            indexBuffer;

        indexBinding.offset =
            0;

        SDL_BindGPUIndexBuffer(
            renderPass,
            &indexBinding,
            SDL_GPU_INDEXELEMENTSIZE_32BIT
        );

        SDL_DrawGPUIndexedPrimitives(
            renderPass,
            indexCount,
            1,
            0,
            0,
            0
        );

        return;
    }

    SDL_DrawGPUPrimitives(
        renderPass,
        vertexCount,
        1,
        0,
        0
    );
}

// =========================================================
// Destroy
// =========================================================

void Mesh::Destroy()
{
    if (
        device != nullptr
    )
    {
        if (
            indexBuffer != nullptr
        )
        {
            SDL_ReleaseGPUBuffer(
                device,
                indexBuffer
            );

            indexBuffer =
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
    }

    vertexCount =
        0;

    indexCount =
        0;

    device =
        nullptr;
}
