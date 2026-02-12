#pragma once
#include "RigidBody.h"
#include <glm/glm.hpp>

void IntegrateRigid(
    RigidBody& b,
    float dt,
    const glm::vec3& gravity
);

struct Contact {
    int a = -1;
    int b = -1;               // -1 なら静的平面など
    glm::vec3 point{0.0f};
    glm::vec3 normal{0,1,0};  // A→B 方向（押し出し方向）
    float penetration = 0.0f;
};

inline glm::mat3 InvInertiaWorld(const RigidBody& b) {
    glm::mat3 R = glm::mat3_cast(b.orientation);
    return R * b.invInertiaLocal * glm::transpose(R);
}
