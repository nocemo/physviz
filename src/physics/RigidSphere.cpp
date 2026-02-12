#include "RigidSphere.h"
#include "Integrator.h"

#include <algorithm>

static float clamp01(float x) { return std::max(0.0f, std::min(1.0f, x)); }

void StepRigidSphere(
    RigidSphere& s,
    IntegratorType integrator,
    float dt,
    const glm::vec3& gravity,
    float floorY
)
{
    // integrate translation using the same integrator logic
    Particle p;
    p.position = s.position;
    p.velocity = s.velocity;
    p.mass     = s.mass;

    glm::vec3 force = p.mass * gravity;
    Integrate(p, force, dt, integrator);

    s.position = p.position;
    s.velocity = p.velocity;

    // --- collision vs plane y=floorY ---
    // Plane normal = (0,1,0)
    float penetration = (floorY + s.radius) - s.position.y;
    if (penetration > 0.0f) {
        // positional correction: push out
        s.position.y += penetration;

        // velocity response
        float vn = s.velocity.y; // normal component along +Y
        if (vn < 0.0f) {
            float e = clamp01(s.restitution);
            s.velocity.y = -e * vn;

            // simple friction: damp tangential velocity a bit on impact
            float mu = clamp01(s.friction);
            float damp = std::max(0.0f, 1.0f - mu);
            s.velocity.x *= damp;
            s.velocity.z *= damp;
        }
    }
}
