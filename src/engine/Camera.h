#pragma once

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <cmath>

class Camera
{
public:
    glm::vec3 target{
        0.0f,
        0.0f,
        0.0f
    };

    glm::vec3 offset{
        7.0f,
        7.0f,
        7.0f
    };

    float halfHeight =
        5.0f;

    // Было 7.0.
    // Теперь камера двигается заметно мягче.
    float followSpeed =
        2.8f;

    void UpdateFollow(
        const glm::vec3& desiredTarget,
        float deltaTime
    )
    {
        const float blend =
            1.0f -
            std::exp(
                -followSpeed *
                deltaTime
            );

        target =
            glm::mix(
                target,
                desiredTarget,
                blend
            );
    }

    [[nodiscard]]
    glm::mat4 GetViewProjection(
        float aspect
    ) const
    {
        const glm::vec3 position =
            target +
            offset;

        const glm::mat4 view =
            glm::lookAtLH(
                position,
                target,
                glm::vec3(
                    0.0f,
                    1.0f,
                    0.0f
                )
            );

        const float halfWidth =
            halfHeight *
            aspect;

        const glm::mat4 projection =
            glm::orthoLH_ZO(
                -halfWidth,
                 halfWidth,
                -halfHeight,
                 halfHeight,
                 0.1f,
                 100.0f
            );

        return
            projection *
            view;
    }
};