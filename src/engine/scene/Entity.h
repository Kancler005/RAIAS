#pragma once

#include "Transform.h"

#include <glm/glm.hpp>

#include <string>

struct Entity
{
    std::string name = "Entity";

    Transform transform{};

    glm::vec4 color{
        1.0f,
        1.0f,
        1.0f,
        1.0f
    };

    bool visible = true;

    bool collidable = false;
};