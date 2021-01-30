#pragma once

#include <glm/glm.hpp>

struct KeyboardControlledComponent
{
    glm::vec2 up_velocity;
    glm::vec2 right_velocity;
    glm::vec2 down_velocity;
    glm::vec2 left_velocity;

    KeyboardControlledComponent(
        glm::vec2 up_velocity = glm::vec2(0),
        glm::vec2 right_velocity = glm::vec2(0),
        glm::vec2 down_velocity = glm::vec2(0),
        glm::vec2 left_velocity = glm::vec2(0)
    ) {
        this->up_velocity = up_velocity;
        this->right_velocity = right_velocity;
        this->down_velocity = down_velocity;
        this->left_velocity = left_velocity;
    }
};