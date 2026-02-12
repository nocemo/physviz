#include "Integrator.h"
#include <algorithm>

void Integrate(Particle& p, const glm::vec3& force, float dt, IntegratorType type)
{
    if (p.mass <= 0.0f) return;

    // Clamp dt to avoid accidental blowups when UI slider goes wild
    dt = std::clamp(dt, 1e-6f, 1.0f);

    const glm::vec3 a = force / p.mass;

    switch (type)
    {
    case IntegratorType::ExplicitEuler:
        // x_{n+1} = x_n + v_n dt
        // v_{n+1} = v_n + a_n dt
        p.position += p.velocity * dt;
        p.velocity += a * dt;
        break;

    case IntegratorType::SemiImplicitEuler:
        // v_{n+1} = v_n + a_n dt
        // x_{n+1} = x_n + v_{n+1} dt
        p.velocity += a * dt;
        p.position += p.velocity * dt;
        break;

    case IntegratorType::VelocityVerlet:
        // x_{n+1} = x_n + v_n dt + 0.5 a_n dt^2
        // v_{n+1} = v_n + a_n dt   (for constant a; good enough for gravity-only)
        p.position += p.velocity * dt + 0.5f * a * (dt * dt);
        p.velocity += a * dt;
        break;
    }
}
