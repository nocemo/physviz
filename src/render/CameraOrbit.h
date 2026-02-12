#pragma once
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

struct CameraOrbit {
    // orbit parameters
    float yawDeg   = 35.0f;
    float pitchDeg = -25.0f;
    float distance = 18.0f;
    glm::vec3 target{0.0f, 0.0f, 0.0f};

    // speeds
    float rotateSpeed = 0.20f;   // deg per pixel
    float panSpeed    = 0.010f;  // world units per pixel (scaled by distance)
    float zoomSpeed   = 0.10f;   // per wheel step

    // internal state
    bool draggingRotate = false;
    bool draggingPan    = false;
    double lastX = 0.0;
    double lastY = 0.0;

    void beginFrame(double mouseX, double mouseY) {
        lastX = mouseX; lastY = mouseY;
    }

    // Call this every frame with current mouse position and buttons state.
    // If allowInput=false, camera won't react (useful when ImGui wants the mouse).
    void updateFromMouse(
        double mouseX, double mouseY,
        bool lmbDown, bool mmbDown, bool rmbDown,
        bool allowInput
    );

    // Wheel callback calls this
    void onScroll(double yoffset);

    glm::vec3 position() const;
    glm::mat4 view() const;
    glm::mat4 proj(float aspect) const;
};
