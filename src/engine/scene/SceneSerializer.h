#pragma once

#include <string>

class Scene;

class SceneSerializer
{
public:
    static bool Save(
        const Scene& scene,
        const std::string& path
    );

    static bool Load(
        Scene& scene,
        const std::string& path
    );
};