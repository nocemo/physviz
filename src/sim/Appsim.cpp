#include "AppSim.h"

#include <algorithm>
#include <glm/gtc/quaternion.hpp>

#include "render/DebugDraw.h"
#include "imgui.h"

void AppSim::init() {
    reset();
}

void AppSim::setupRigidDemo()
{
    world.clear();

    world.gravity = glm::vec3(0.0f, -gravity, 0.0f);
    world.floorY = 0.0f;
    world.linearIntegrator = integrator;
    world.solverIterations = 8;
    world.posSlop = 0.001f;
    world.posPercent = 0.8f;

    // 1) “回転が出やすい”斜め衝突球（床摩擦でスピン）
    {
        int id = world.addSphere(glm::vec3(-2.0f, 6.0f, 0.0f), 0.75f, 1.0f, glm::vec3(4.0f, 0.0f, 1.5f));
        auto& b = world.bodies[id];
        b.restitution = 0.3f;  // 跳ねにくくして転がりが見やすい
        b.friction = 0.8f;     // 摩擦強め（回転が出やすい）
    }

    // 2) 剛体同士衝突確認（2球）
    {
        int a = world.addSphere(glm::vec3(2.0f, 3.0f, 0.0f), 0.6f, 1.0f, glm::vec3(-2.5f, 0.0f, 0.0f));
        int b = world.addSphere(glm::vec3(5.0f, 3.0f, 0.0f), 0.6f, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
        world.bodies[a].restitution = 0.8f;
        world.bodies[b].restitution = 0.8f;
        world.bodies[a].friction = 0.2f;
        world.bodies[b].friction = 0.2f;
    }
}

void AppSim::setupPbdDemo()
{
    pbd.clear();
    pbd.dt = simDt;
    pbd.gravity = glm::vec3(0.0f, -gravity, 0.0f);
    pbd.solverIterations = 15;
    pbd.enableGround = true;
    pbd.groundY = 0.0f;

    // ---- Demo A: pendulum (1 fixed + 1 free) ----
    int a = pbd.addParticle(glm::vec3(-4.0f, 6.0f, 0.0f), 0.0f); // fixed (mass=0 => invMass=0)
    int b = pbd.addParticle(glm::vec3(-2.0f, 3.0f, 0.0f), 1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
    pbd.addDistance(a, b, 3.2f);

    // ---- Demo B: rope ----
    int n = 20;
    float spacing = 0.35f;
    glm::vec3 start(1.0f, 6.0f, 0.0f);

    int prev = pbd.addParticle(start, 0.0f); // fixed
    for (int i = 1; i < n; ++i) {
        int id = pbd.addParticle(start + glm::vec3(spacing * i, 0.0f, 0.0f), 1.0f);
        pbd.addDistance(prev, id, spacing);
        prev = id;
    }

    // ---- Demo C: cloth ----
    int m = 20;
    float spacing_cloth = 0.35f;
    glm::vec3 start_cloth(6.0f, 6.0f, 0.0f);

    int prev_i;
    int prev_j[20];
    for (int j = 0; j < m; ++j) {
        for (int i = 0; i < m; ++i) {
            float mass = ((i == 0 && j == 0) || (i == 19 && j == 19)) ? 0.0f: 1.0f;
            int id = pbd.addParticle(start_cloth + glm::vec3(spacing_cloth * i, 0.0f, spacing_cloth * j), mass);
            if (i > 0) {
                pbd.addDistance(prev_i, id, spacing_cloth);
            }
            if (j > 0) {
                pbd.addDistance(prev_j[i], id, spacing_cloth);
            }
            prev_i = id;
            prev_j[i] = id;
        }
    }

}

void AppSim::reset() {
    // Step1 particle
    p.position = glm::vec3(0.0f, 8.0f, 0.0f);
    p.velocity = glm::vec3(2.0f, 0.0f, 0.0f);
    p.mass = 1.0f;

    //setupRigidDemo();
    setupPbdDemo();
}

void AppSim::update(float /*realDtSeconds*/) {
    if (paused) return;

    if (mode == DemoMode::Particle) {
        glm::vec3 g(0.0f, -gravity, 0.0f);
        glm::vec3 force = p.mass * g;
        Integrate(p, force, simDt, integrator);
        return;
    }

    if (mode == DemoMode::PBD) {
        pbd.dt = simDt;
        pbd.gravity = glm::vec3(0.0f, -gravity, 0.0f);
        pbd.step();
        return;
    }

    // Rigid world
    world.gravity = glm::vec3(0.0f, -gravity, 0.0f);
    world.linearIntegrator = integrator;
    world.step(simDt);
}

void AppSim::update_step(float /*realDtSeconds*/) {
    if (mode == DemoMode::Particle) {
        glm::vec3 g(0.0f, -gravity, 0.0f);
        glm::vec3 force = p.mass * g;
        Integrate(p, force, simDt, integrator);
        return;
    }

    if (mode == DemoMode::PBD) {
        pbd.dt = simDt;
        pbd.gravity = glm::vec3(0.0f, -gravity, 0.0f);
        pbd.step();
        return;
    }

    // Rigid world
    world.gravity = glm::vec3(0.0f, -gravity, 0.0f);
    world.linearIntegrator = integrator;
    world.step(simDt);
}

static void DrawBodyAxes(DebugDraw& dbg, const glm::vec3& pos, const glm::quat& q, float s)
{
    glm::mat3 R = glm::mat3_cast(q);
    glm::vec3 x = pos + R * glm::vec3(s,0,0);
    glm::vec3 y = pos + R * glm::vec3(0,s,0);
    glm::vec3 z = pos + R * glm::vec3(0,0,s);

    dbg.addLine(pos, x, glm::vec3(1,0,0));
    dbg.addLine(pos, y, glm::vec3(0,1,0));
    dbg.addLine(pos, z, glm::vec3(0,0,1));
}

void AppSim::buildDebugDraw(DebugDraw& dbg) const {
    if (mode == DemoMode::Particle) {
        dbg.addCross(p.position, 0.15f, glm::vec3(1.0f, 1.0f, 0.2f));
        dbg.addArrow(p.position, p.position + p.velocity * velDrawScale, glm::vec3(0.2f, 1.0f, 1.0f), 0.25f);
        return;
    }

    if (mode == DemoMode::PBD) {
    // ground line
    dbg.addLine(glm::vec3(-50.0f, pbd.groundY, 0.0f), glm::vec3(50.0f, pbd.groundY, 0.0f), glm::vec3(0.8f));

    // constraints
    for (const auto& c : pbd.dist) {
        const auto& p0 = pbd.particles[c.i0];
        const auto& p1 = pbd.particles[c.i1];
        dbg.addLine(p0.x, p1.x, glm::vec3(0.6f, 0.9f, 0.6f));
    }

    // particles
    for (const auto& p : pbd.particles) {
        glm::vec3 col = (p.invMass == 0.0f) ? glm::vec3(1.0f, 0.4f, 0.2f) : glm::vec3(1.0f, 1.0f, 0.2f);
        dbg.addCross(p.x, 0.08f, col);
    }
    return;
    }

    // floor line
    dbg.addLine(glm::vec3(-50.0f, world.floorY, 0.0f), glm::vec3(50.0f, world.floorY, 0.0f), glm::vec3(0.8f));

    // bodies
    for (const auto& b : world.bodies) {
        dbg.addCross(b.position, 0.12f, glm::vec3(1.0f, 0.9f, 0.2f));
        dbg.addArrow(b.position, b.position + b.linearVelocity * velDrawScale, glm::vec3(0.2f, 1.0f, 1.0f), 0.25f);

        // show angular velocity direction
        dbg.addArrow(b.position, b.position + b.angularVelocity * 0.25f, glm::vec3(1.0f, 0.2f, 1.0f), 0.25f);

        // local axes (this is the “xyz axes” you noticed in Bullet debug draw)
        DrawBodyAxes(dbg, b.position, b.orientation, 0.9f);
    }

    // contacts
    for (const auto& c : world.contacts()) {
        dbg.addCross(c.point, 0.10f, glm::vec3(1.0f, 0.2f, 0.2f));
        dbg.addArrow(c.point, c.point + c.normal * 1.0f, glm::vec3(1.0f, 0.2f, 0.2f), 0.35f);
        dbg.addArrow(c.point, c.point + c.normal * c.penetration, glm::vec3(0.2f, 1.0f, 1.0f), 0.35f);
    }
}

void AppSim::drawUI() {
    ImGui::Begin("Sim");

    ImGui::Checkbox("Paused", &paused);
    if (ImGui::Button("Step")) update_step(simDt);

    const char* modes[] = { "Particle (Step1)", "RigidWorld (Step2-A)", "PBD" };
    int m = (int)mode;
    if (ImGui::Combo("Demo", &m, modes, IM_ARRAYSIZE(modes))) {
        mode = (DemoMode)m;
    }

    const char* items[] = { "Explicit Euler", "Semi-Implicit Euler", "Velocity Verlet" };
    int it = (int)integrator;
    if (ImGui::Combo("Integrator", &it, items, IM_ARRAYSIZE(items))) {
        integrator = (IntegratorType)it;
    }

    ImGui::SliderFloat("Sim dt", &simDt, 1.0f/240.0f, 1.0f/15.0f, "%.6f");
    ImGui::SliderFloat("Gravity", &gravity, 0.0f, 30.0f, "%.3f");
    ImGui::SliderFloat("Vel draw scale", &velDrawScale, 0.01f, 2.0f, "%.3f");

    if (ImGui::Button("Reset")) reset();

    if (mode == DemoMode::RigidWorld) {
        ImGui::Separator();
        ImGui::Text("RigidWorld Solver");
        ImGui::SliderInt("Iterations", &world.solverIterations, 1, 30);
        ImGui::SliderFloat("Pos slop", &world.posSlop, 0.0f, 0.02f, "%.5f");
        ImGui::SliderFloat("Pos percent", &world.posPercent, 0.0f, 1.0f, "%.3f");
        ImGui::DragFloat("Floor Y", &world.floorY, 0.01f, -10.0f, 10.0f);

        ImGui::Text("Bodies: %d", (int)world.bodies.size());
        ImGui::Text("Contacts: %d", (int)world.contacts().size());
        ImGui::Text("Tip: friction high => spin more visible on floor.");
    }

    if (mode == DemoMode::PBD) {
        ImGui::Separator();
        ImGui::Text("PBD");
        ImGui::SliderInt("Iterations", &pbd.solverIterations, 1, 60);
        ImGui::Checkbox("Apply global compliance", &pbd.applyGlobalCompliance);
        ImGui::DragFloat("Compliance", &pbd.globalCompliance, 1e-6f, 0.0f, 0.05f, "%.6f");
        ImGui::Checkbox("Ground", &pbd.enableGround);
        ImGui::DragFloat("Ground Y", &pbd.groundY, 0.01f, -10.0f, 10.0f);
        ImGui::Text("Particles: %d", (int)pbd.particles.size());
        ImGui::Text("Constraints: %d", (int)pbd.dist.size());
    }

    ImGui::End();
}
