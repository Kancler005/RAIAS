#pragma once

#include "Camera.h"
#include "Renderer.h"
#include "scene/Scene.h"

#include <SDL3/SDL.h>

class Application
{
public:

    bool Initialize();

    void Run();

    void Shutdown();

private:

    void ProcessEvents();

    void Update(
        float deltaTime
    );

private:

    SDL_Window* window =
        nullptr;

    Renderer renderer;

    Scene scene;

    Camera camera;

    Entity* player =
        nullptr;

    bool running =
        false;

    Uint64 previousTicks =
        0;

    glm::vec3 playerVelocity{
        0.0f
    };

    float playerSpeed =
        2.6f;

    float acceleration =
        10.0f;

    float deceleration =
        8.0f;
};