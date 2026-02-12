#pragma once
#include <glm/glm.hpp>
#include "Particle.h"

enum class IntegratorType {
    ExplicitEuler = 0,
    SemiImplicitEuler,
    VelocityVerlet
};

inline const char* IntegratorName(IntegratorType t) {
    switch (t) {
    case IntegratorType::ExplicitEuler:     return "Explicit Euler";
    case IntegratorType::SemiImplicitEuler: return "Semi-Implicit Euler";
    case IntegratorType::VelocityVerlet:    return "Velocity Verlet";
    default:                                return "Unknown";
    }
}

void Integrate(
    Particle& p,
    const glm::vec3& force,
    float dt,
    IntegratorType type
);
