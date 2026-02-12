#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "RigidBody.h"
#include "Integrator.h"

struct Contact {
    int a = -1;     // body index
    int b = -1;     // body index, or -1 for plane
    glm::vec3 point{0.0f};
    glm::vec3 normal{0,1,0};  // push direction for A (A should move -normal when penetrating)
    float penetration = 0.0f;
};

class RigidWorld {
public:
    // world params
    glm::vec3 gravity{0.0f, -9.8f, 0.0f};
    float floorY = 0.0f;

    // solver params
    int   solverIterations = 8;
    float posSlop = 0.001f;
    float posPercent = 0.8f;

    // integration
    IntegratorType linearIntegrator = IntegratorType::SemiImplicitEuler;

    // bodies
    std::vector<RigidBody> bodies;

    int addSphere(const glm::vec3& pos, float radius, float mass, const glm::vec3& vel);

    void clear();
    void step(float dt);

    const std::vector<Contact>& contacts() const { return m_contacts; }

private:
    std::vector<Contact> m_contacts;

    void integrateBodies(float dt);
    void generateContacts();
    void solveContacts();
    void positionalCorrection();

    // collision routines
    void collideSpherePlane(int a);
    void collideSphereSphere(int a, int b);

    // solver routine
    void solveOne(const Contact& c);
};
