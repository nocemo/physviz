#pragma once
#include <glm/glm.hpp>

struct PBDParticle {
    glm::vec3 x{0.0f};      // position
    glm::vec3 v{0.0f};      // velocity
    glm::vec3 xPrev{0.0f};  // for velocity update
    float invMass = 1.0f;   // 0 => fixed
};

struct DistanceConstraint {
    int i0 = -1;
    int i1 = -1;
    float restLength = 1.0f;

    // XPBD:
    float compliance = 0.0f; // 0 = rigid, larger = softer
    float lambda = 0.0f;     // accumulated Lagrange multiplier
};
