#pragma once

#include <SDL3/SDL_gpu.h>

#include <string>

class Mesh;

class ModelLoader
{
public:
    // Loads .glb or .gltf geometry into one RAI Mesh.
    //
    // Current milestone:
    // - positions
    // - normals
    // - UV0
    // - indices
    // - multiple nodes / meshes / triangle primitives
    // - node transforms
    // - automatic normalization to RAI unit size
    //
    // Materials, textures, skinning and animation come later.
    static bool LoadGLTF(
        SDL_GPUDevice* device,
        const std::string& path,
        Mesh& destination
    );
};
