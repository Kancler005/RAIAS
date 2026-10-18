#pragma once

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

struct Transform
{
    glm::vec3 position{
        0.0f,
        0.0f,
        0.0f
    };

    glm::vec3 rotation{
        0.0f,
        0.0f,
        0.0f
    };

    glm::vec3 scale{
        1.0f,
        1.0f,
        1.0f
    };

    [[nodiscard]]
    glm::mat4 GetMatrix() const
    {
        glm::mat4 matrix(1.0f);

        matrix = glm::translate(
            matrix,
            position
        );

        matrix = glm::rotate(
            matrix,
            glm::radians(rotation.x),
            glm::vec3(1.0f, 0.0f, 0.0f)
        );

        matrix = glm::rotate(
            matrix,
            glm::radians(rotation.y),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        matrix = glm::rotate(
            matrix,
            glm::radians(rotation.z),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );

        matrix = glm::scale(
            matrix,
            scale
        );

        return matrix;
    }
};