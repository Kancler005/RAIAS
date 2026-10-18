#pragma once

#include "Transform.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <string>

using EntityId = std::uint64_t;

struct Entity
{
    EntityId id = 0;

    std::string name =
        "Entity";

    // Пока это только логическое имя prefab.
    // Позже оно будет ссылаться на настоящий prefab asset.
    std::string prefab =
        "cube";

    Transform transform{};

    glm::vec4 color{
        1.0f,
        1.0f,
        1.0f,
        1.0f
    };

    bool visible =
        true;

    bool collidable =
        false;
};