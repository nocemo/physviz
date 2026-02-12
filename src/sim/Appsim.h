#pragma once
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

#include "physics/Particle.h"
#include "physics/Integrator.h"
#include "physics/RigidWorld.h"
#include "physics/PBDWorld.h"

class DebugDraw;

class AppSim {
public:
    void init();
    void reset();
    void update(float realDtSeconds);
    void update_step(float realDtSeconds);
    void buildDebugDraw(DebugDraw& dbg) const;
    void drawUI();

private:
    enum class DemoMode { Particle = 0, RigidWorld, PBD };
    DemoMode mode = DemoMode::PBD;

    // Step1 particle
    Particle p;

    // shared sim settings
    IntegratorType integrator = IntegratorType::SemiImplicitEuler;
    float simDt = 1.0f / 60.0f;
    float gravity = 9.8f;
    float velDrawScale = 0.25f;
    bool paused = false;

    // Step2-A rigid world
    RigidWorld world;

    PBDWorld pbd;

    // demo init helpers
    void setupRigidDemo();
    void setupPbdDemo();
};
