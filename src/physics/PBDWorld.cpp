#include "PBDWorld.h"
#include <algorithm>
#include <cmath>

static float safeInv(float x) { return (std::abs(x) > 1e-8f) ? (1.0f / x) : 0.0f; }

void PBDWorld::clear() {
    particles.clear();
    dist.clear();
}

int PBDWorld::addParticle(const glm::vec3& pos, float mass, const glm::vec3& vel) {
    PBDParticle p;
    p.x = pos;
    p.v = vel;
    p.xPrev = pos;
    p.invMass = (mass > 0.0f) ? (1.0f / mass) : 0.0f;
    particles.push_back(p);
    return (int)particles.size() - 1;
}

void PBDWorld::addDistance(int a, int b, float restLength, float compliance) {
    DistanceConstraint c;
    c.i0 = a;
    c.i1 = b;
    c.restLength = restLength;
    c.compliance = compliance;
    c.lambda = 0.0f;
    dist.push_back(c);
}

static void SolveDistanceXPBD(std::vector<PBDParticle>& ps, DistanceConstraint& c, float dt)
{
    auto& p0 = ps[c.i0];
    auto& p1 = ps[c.i1];

    float w0 = p0.invMass;
    float w1 = p1.invMass;
    float wSum = w0 + w1;
    if (wSum <= 0.0f) return;

    glm::vec3 d = p1.x - p0.x;
    float len2 = glm::dot(d, d);
    if (len2 < 1e-12f) return;

    float len = std::sqrt(len2);
    glm::vec3 n = d / len;

    float C = len - c.restLength;

    // XPBD: alpha = compliance / dt^2
    float alpha = (dt > 0.0f) ? (c.compliance / (dt * dt)) : 0.0f;

    // Δλ = -(C + alpha*λ) / (wSum + alpha)
    float denom = wSum + alpha;
    if (denom <= 0.0f) return;

    float dLambda = -(C + alpha * c.lambda) / denom;
    c.lambda += dLambda;

    // position corrections
    p0.x += (-w0 * dLambda) * n;
    p1.x += ( w1 * dLambda) * n;
}

void PBDWorld::step() {
    if (applyGlobalCompliance) {
    for (auto& c : dist) c.compliance = globalCompliance;
    }

    // XPBD: reset lambdas each time step (simple, stable)
    for (auto& c : dist) c.lambda = 0.0f;

    dt = std::clamp(dt, 1e-6f, 1.0f);

    // 1) Predict positions
    for (auto& p : particles) {
        p.xPrev = p.x;

        if (p.invMass == 0.0f) continue; // fixed
        p.v += gravity * dt;
        p.x += p.v * dt;
    }

    // 2) Project constraints (iterate)
    for (int it = 0; it < solverIterations; ++it) {
        for (auto& c : dist) {
            SolveDistanceXPBD(particles, c, dt);
        }

        // simple ground collision as positional projection
        if (enableGround) {
            for (auto& p : particles) {
                if (p.invMass == 0.0f) continue;
                if (p.x.y < groundY) p.x.y = groundY;
            }
        }
    }

    // 3) Update velocities from position change
    for (auto& p : particles) {
        if (p.invMass == 0.0f) continue;
        p.v = (p.x - p.xPrev) / dt;
    }
}
