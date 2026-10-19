#pragma once

#include "Mesh.h"

#include <SDL3/SDL_gpu.h>

namespace ProceduralMeshes
{
    bool CreateRockSmall(
        SDL_GPUDevice* device,
        Mesh& mesh
    );

    bool CreateRockWide(
        SDL_GPUDevice* device,
        Mesh& mesh
    );

    bool CreateRockTall(
        SDL_GPUDevice* device,
        Mesh& mesh
    );

    bool CreateRuinedShrine(
        SDL_GPUDevice* device,
        Mesh& mesh
    );

    bool CreateBrokenWatchtower(
        SDL_GPUDevice* device,
        Mesh& mesh
    );

    bool CreateDeadTreeA(
        SDL_GPUDevice* device,
        Mesh& mesh
    );

    bool CreateDeadTreeB(
        SDL_GPUDevice* device,
        Mesh& mesh
    );

    bool CreateCampfire(
        SDL_GPUDevice* device,
        Mesh& mesh
    );
}
