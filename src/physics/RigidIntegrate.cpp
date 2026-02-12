#include "RigidIntegrate.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

static glm::mat3 RotMat(const glm::quat& q) {
    return glm::mat3_cast(q);
}

void IntegrateRigid(RigidBody& b, float dt, const glm::vec3& gravity)
{
    if (b.isStatic()) return;

    // linear
    b.linearVelocity += gravity * dt;
    b.position += b.linearVelocity * dt;

    // angular: (外力トルクなしなら ω は一定) まずはそのまま
    // orientation update: q' = 0.5 * ω_quat * q
    glm::quat wq(0.0f, b.angularVelocity.x, b.angularVelocity.y, b.angularVelocity.z);
    b.orientation = glm::normalize(b.orientation + (0.5f * dt) * (wq * b.orientation));
}
