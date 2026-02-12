#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "render/DebugDraw.h"
#include "physics/Particle.h"
#include "physics/Integrator.h"
#include "sim/AppSim.h"
#include "render/CameraOrbit.h"

// ImGui
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

static void glfw_error_callback(int error, const char* description) {
    std::fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "physviz (OpenGL+GLFW+ImGui)", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    CameraOrbit cam;
    glfwSetWindowUserPointer(window, &cam);
    glfwSetScrollCallback(window, [](GLFWwindow* w, double /*xoff*/, double yoff){
        auto* c = (CameraOrbit*)glfwGetWindowUserPointer(w);
        if (c) c->onScroll(yoff);
    });

    if (!gladLoadGL()) {
        std::fprintf(stderr, "Failed to init glad\n");
        return 1;
    }

    // ImGui init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Debug draw
    DebugDraw dbg;
    if (!dbg.init()) {
        std::fprintf(stderr, "Failed to init DebugDraw\n");
        return 1;
    }

    // Camera params
    float camYaw = 35.0f;
    float camPitch = -25.0f;
    float camDist = 18.0f;

    AppSim sim;
    sim.init();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

         // Mouse state
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);

        bool lmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)   == GLFW_PRESS;
        bool mmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
        bool rmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)  == GLFW_PRESS;

        // If ImGui wants mouse, don't move camera
        ImGuiIO& io = ImGui::GetIO();
        bool allowCam = !io.WantCaptureMouse;

        static bool first = true;
        if (first) { cam.beginFrame(mx, my); first = false; }

        cam.updateFromMouse(mx, my, lmb, mmb, rmb, allowCam);

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glEnable(GL_DEPTH_TEST);

        // Build a simple orbit camera
        glm::vec3 target(0, 0, 0);
        glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(camYaw), glm::vec3(0,1,0));
        glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(camPitch), glm::vec3(1,0,0));
        glm::vec3 camPos = target + glm::vec3(rotY * rotX * glm::vec4(0, 0, camDist, 1.0f));
        glm::mat4 view = cam.view();
        glm::mat4 proj = cam.proj((float)w / (float)h);
        glm::mat4 viewProj = proj * view;

        // Build debug geometry
        dbg.clear();
        sim.update(0.0f);

        dbg.addGrid(20, 1.0f);
        dbg.addAxes(2.0f);

        sim.buildDebugDraw(dbg);

        // --- ImGui frame ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        sim.drawUI();

        // --- Render ---
        glClearColor(0.08f, 0.09f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        dbg.draw(viewProj);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    dbg.shutdown();

    // ImGui shutdown
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
