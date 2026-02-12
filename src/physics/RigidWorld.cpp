#include "RigidWorld.h"

#include <algorithm>
#include <cmath>

#include <glm/gtc/quaternion.hpp>

#include "RigidBodyUtil.h"
#include "Particle.h"
#include "Integrator.h"

static float clamp01_local(float x) { return std::max(0.0f, std::min(1.0f, x)); }

int RigidWorld::addSphere(const glm::vec3& pos, float radius, float mass, const glm::vec3& vel)
{
    RigidBody b;
    b.position = pos;
    b.orientation = glm::quat(1,0,0,0);
    b.linearVelocity = vel;
    b.angularVelocity = glm::vec3(0.0f);

    SetSphereShape(b, radius);
    SetMass(b, mass);
    SetSphereInertia(b);

    // material defaults
    b.restitution = 0.6f;
    b.friction = 0.2f;

    bodies.push_back(b);
    return (int)bodies.size() - 1;
}

void RigidWorld::clear()
{
    bodies.clear();
    m_contacts.clear();
}

void RigidWorld::step(float dt)
{
    dt = std::clamp(dt, 1e-6f, 1.0f);

    integrateBodies(dt);
    generateContacts();

    // iterative impulse solve
    for (int it = 0; it < solverIterations; ++it) {
        solveContacts();
    }

    positionalCorrection();
}

void RigidWorld::integrateBodies(float dt)
{
    for (auto& b : bodies) {
        if (b.isStatic()) continue;

        // linear integrate using your existing Integrator (gravity-only force)
        Particle p;
        p.position = b.position;
        p.velocity = b.linearVelocity;
        p.mass     = b.mass;

        glm::vec3 force = p.mass * gravity;
        Integrate(p, force, dt, linearIntegrator);

        b.position = p.position;
        b.linearVelocity = p.velocity;

        // angular integrate: q' = 0.5 * ω_quat * q
        glm::quat wq(0.0f, b.angularVelocity.x, b.angularVelocity.y, b.angularVelocity.z);
        b.orientation = glm::normalize(b.orientation + (0.5f * dt) * (wq * b.orientation));
    }
}

void RigidWorld::generateContacts()
{
    m_contacts.clear();

    // Sphere-Plane for all
    for (int i = 0; i < (int)bodies.size(); ++i) {
        collideSpherePlane(i);
    }

    // Sphere-Sphere pairs
    for (int i = 0; i < (int)bodies.size(); ++i) {
        for (int j = i + 1; j < (int)bodies.size(); ++j) {
            collideSphereSphere(i, j);
        }
    }
}

void RigidWorld::collideSpherePlane(int a)
{
    const auto& A = bodies[a];
    if (A.shapeType != ShapeType::Sphere) return;

    float r = A.sphere.radius;
    float penetration = (floorY + r) - A.position.y;
    if (penetration <= 0.0f) return;

    Contact c;
    c.a = a;
    c.b = -1; // plane
    c.normal = glm::vec3(0, 1, 0); // plane normal pointing up

    // contact point on plane under sphere center
    c.point = glm::vec3(A.position.x, floorY, A.position.z);
    c.penetration = penetration;

    m_contacts.push_back(c);
}

void RigidWorld::collideSphereSphere(int a, int b)
{
    const auto& A = bodies[a];
    const auto& B = bodies[b];
    if (A.shapeType != ShapeType::Sphere || B.shapeType != ShapeType::Sphere) return;

    float ra = A.sphere.radius;
    float rb = B.sphere.radius;
    glm::vec3 d = B.position - A.position;
    float dist2 = glm::dot(d, d);
    float r = ra + rb;

    if (dist2 >= r * r) return;

    float dist = std::sqrt(std::max(dist2, 1e-12f));
    glm::vec3 n = (dist > 1e-6f) ? (d / dist) : glm::vec3(0, 1, 0);

    Contact c;
    c.a = a;
    c.b = b;

    // normal points from A to B
    c.normal = n;
    c.penetration = r - dist;

    // approximate contact point between surfaces (mid-ish)
    c.point = A.position + n * (ra - 0.5f * c.penetration);

    m_contacts.push_back(c);
}

static void ApplyImpulse(RigidBody& b, const glm::vec3& impulse, const glm::vec3& r)
{
    if (b.isStatic()) return;
    b.linearVelocity += impulse * b.invMass;
    b.angularVelocity += InvInertiaWorld(b) * glm::cross(r, impulse);
}

void RigidWorld::solveContacts()
{
    for (const auto& c : m_contacts) {
        solveOne(c);
    }
}

void RigidWorld::solveOne(const Contact& c)
{
    RigidBody& A = bodies[c.a];

    // plane contact: treat B as static infinite mass
    if (c.b == -1) {
        glm::vec3 n = c.normal;
        glm::vec3 ra = c.point - A.position;

        glm::vec3 va = A.linearVelocity + glm::cross(A.angularVelocity, ra);
        glm::vec3 vb(0.0f); // plane

        glm::vec3 rv = vb - va;
        float vn = glm::dot(rv, n);
        if (vn > 0.0f) return; // separating

        float e = clamp01_local(A.restitution);

        glm::mat3 Ia = InvInertiaWorld(A);

        float denom =
            A.invMass
            + glm::dot(n, glm::cross(Ia * glm::cross(ra, n), ra));

        float j = -(1.0f + e) * vn;
        j = (denom > 0.0f) ? (j / denom) : 0.0f;

        glm::vec3 impulseN = j * n;
        ApplyImpulse(A, -impulseN, ra);

        // friction
        va = A.linearVelocity + glm::cross(A.angularVelocity, ra);
        rv = vb - va;

        glm::vec3 t = rv - glm::dot(rv, n) * n;
        float tlen2 = glm::dot(t, t);
        if (tlen2 > 1e-12f) {
            t /= std::sqrt(tlen2);

            float vt = glm::dot(rv, t);

            float denomT =
                A.invMass
                + glm::dot(t, glm::cross(Ia * glm::cross(ra, t), ra));

            float jt = (denomT > 0.0f) ? (-vt / denomT) : 0.0f;

            float mu = clamp01_local(A.friction);
            float jtMax = mu * j;
            jt = std::clamp(jt, -jtMax, jtMax);

            glm::vec3 impulseT = jt * t;
            ApplyImpulse(A, -impulseT, ra);
        }
        return;
    }

    RigidBody& B = bodies[c.b];

    glm::vec3 n = c.normal;

    glm::vec3 ra = c.point - A.position;
    glm::vec3 rb = c.point - B.position;

    glm::vec3 va = A.linearVelocity + glm::cross(A.angularVelocity, ra);
    glm::vec3 vb = B.linearVelocity + glm::cross(B.angularVelocity, rb);

    glm::vec3 rv = vb - va;
    float vn = glm::dot(rv, n);
    if (vn > 0.0f) return; // separating

    float e = std::min(clamp01_local(A.restitution), clamp01_local(B.restitution));

    glm::mat3 Ia = InvInertiaWorld(A);
    glm::mat3 Ib = InvInertiaWorld(B);

    float denom =
        A.invMass + B.invMass
        + glm::dot(n, glm::cross(Ia * glm::cross(ra, n), ra) + glm::cross(Ib * glm::cross(rb, n), rb));

    float j = -(1.0f + e) * vn;
    j = (denom > 0.0f) ? (j / denom) : 0.0f;

    glm::vec3 impulseN = j * n;
    ApplyImpulse(A, -impulseN, ra);
    ApplyImpulse(B,  impulseN, rb);

    // friction
    va = A.linearVelocity + glm::cross(A.angularVelocity, ra);
    vb = B.linearVelocity + glm::cross(B.angularVelocity, rb);
    rv = vb - va;

    glm::vec3 t = rv - glm::dot(rv, n) * n;
    float tlen2 = glm::dot(t, t);
    if (tlen2 > 1e-12f) {
        t /= std::sqrt(tlen2);

        float vt = glm::dot(rv, t);

        float denomT =
            A.invMass + B.invMass
            + glm::dot(t, glm::cross(Ia * glm::cross(ra, t), ra) + glm::cross(Ib * glm::cross(rb, t), rb));

        float jt = (denomT > 0.0f) ? (-vt / denomT) : 0.0f;

        float mu = 0.5f * (clamp01_local(A.friction) + clamp01_local(B.friction));
        float jtMax = mu * j;
        jt = std::clamp(jt, -jtMax, jtMax);

        glm::vec3 impulseT = jt * t;
        ApplyImpulse(A, -impulseT, ra);
        ApplyImpulse(B,  impulseT, rb);
    }
}

void RigidWorld::positionalCorrection()
{
    for (const auto& c : m_contacts) {
        RigidBody& A = bodies[c.a];

        float pen = std::max(c.penetration - posSlop, 0.0f);
        if (pen <= 0.0f) continue;

        glm::vec3 correctionDir = c.normal;
        float percent = posPercent;

        if (c.b == -1) {
            // plane: only move A
            if (!A.isStatic()) {
                A.position += correctionDir * (percent * pen);
            }
            continue;
        }

        RigidBody& B = bodies[c.b];
        float invMassSum = A.invMass + B.invMass;
        if (invMassSum <= 0.0f) continue;

        glm::vec3 correction = (percent * pen / invMassSum) * correctionDir;
        if (!A.isStatic()) A.position -= correction * A.invMass;
        if (!B.isStatic()) B.position += correction * B.invMass;
    }
}
