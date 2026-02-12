#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

enum class ShapeType { Sphere = 0 };

struct SphereShape {
    float radius = 0.5f;
};

struct RigidBody {
    // pose
    glm::vec3 position{0.0f};
    glm::quat orientation{1,0,0,0}; // w,x,y,z

    // velocities
    glm::vec3 linearVelocity{0.0f};
    glm::vec3 angularVelocity{0.0f}; // world-space ω

    // mass props
    float mass = 1.0f;
    float invMass = 1.0f;

    // inertia in LOCAL (diagonal for sphere)
    glm::mat3 inertiaLocal{1.0f};
    glm::mat3 invInertiaLocal{1.0f};

    // material
    float restitution = 0.6f; // 0..1
    float friction    = 0.2f; // 0..1 (Coulomb)

    // shape
    ShapeType shapeType = ShapeType::Sphere;
    SphereShape sphere;

    bool isStatic() const { return invMass == 0.0f; }
};