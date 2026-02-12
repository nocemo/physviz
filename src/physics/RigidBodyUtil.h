#pragma once
#include "RigidBody.h"
#include <glm/glm.hpp>
#include <algorithm>

inline float clamp01(float x) { return std::max(0.0f, std::min(1.0f, x)); }

inline void SetMass(RigidBody& b, float m)
{
    b.mass = m;
    b.invMass = (m > 0.0f) ? (1.0f / m) : 0.0f;
}

inline void SetSphereShape(RigidBody& b, float radius)
{
    b.shapeType = ShapeType::Sphere;
    b.sphere.radius = radius;
}

// Solid sphere inertia: I = 2/5 m r^2
inline void SetSphereInertia(RigidBody& b)
{
    float r = b.sphere.radius;
    float I = (2.0f/5.0f) * b.mass * r * r;

    b.inertiaLocal = glm::mat3(0.0f);
    b.inertiaLocal[0][0] = I;
    b.inertiaLocal[1][1] = I;
    b.inertiaLocal[2][2] = I;

    b.invInertiaLocal = glm::mat3(0.0f);
    if (I > 0.0f) {
        float invI = 1.0f / I;
        b.invInertiaLocal[0][0] = invI;
        b.invInertiaLocal[1][1] = invI;
        b.invInertiaLocal[2][2] = invI;
    }
}

inline glm::mat3 InvInertiaWorld(const RigidBody& b)
{
    glm::mat3 R = glm::mat3_cast(b.orientation);
    return R * b.invInertiaLocal * glm::transpose(R);
}