cmake_minimum_required(VERSION 3.24)

project(RAIAS
        VERSION 0.0.19
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.4.18
        GIT_SHALLOW TRUE
)

set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SDL3)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.3
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glm)

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(nlohmann_json)


# cgltf - small glTF 2.0 loader

FetchContent_Declare(
        cgltf
        GIT_REPOSITORY https://github.com/jkuhlmann/cgltf.git
        GIT_TAG v1.15
        GIT_SHALLOW TRUE
)

FetchContent_GetProperties(cgltf)

if(NOT cgltf_POPULATED)
    FetchContent_Populate(cgltf)
endif()

add_executable(RAI
        src/main.cpp

        src/engine/Application.cpp
        src/engine/Renderer.cpp

        src/engine/render/Mesh.cpp
        src/engine/render/ModelLoader.cpp

        src/engine/scene/Scene.cpp
        src/engine/scene/SceneSerializer.cpp

        src/engine/physics/CollisionSystem.cpp

        src/game/GameWorld.cpp
        src/game/EnemySystem.cpp
)

target_include_directories(RAI
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${cgltf_SOURCE_DIR}
)

target_link_libraries(RAI
        PRIVATE
        SDL3::SDL3
        glm::glm
        nlohmann_json::nlohmann_json
)
