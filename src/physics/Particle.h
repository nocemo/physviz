#pragma once
#include <glm/glm.hpp>

struct Particle {
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};
    float mass = 1.0f;
};
