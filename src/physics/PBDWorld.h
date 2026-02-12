#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "PBDTypes.h"

class PBDWorld {
public:
    glm::vec3 gravity{0.0f, -9.8f, 0.0f};

    float dt = 1.0f / 60.0f;
    int solverIterations = 10;

    // simple ground
    bool enableGround = true;
    float groundY = 0.0f;

    float globalCompliance = 0.0f; // demo knob
    bool  applyGlobalCompliance = true;

    std::vector<PBDParticle> particles;
    std::vector<DistanceConstraint> dist;

    void clear();
    void step();

    // helpers
    int addParticle(const glm::vec3& pos, float mass, const glm::vec3& vel = glm::vec3(0));
    void addDistance(int a, int b, float restLength, float compliance = 0.0f);
};
