#include "CameraOrbit.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

static float clampf(float v, float a, float b) { return std::max(a, std::min(b, v)); }

glm::vec3 CameraOrbit::position() const {
    glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(yawDeg),   glm::vec3(0,1,0));
    glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(pitchDeg), glm::vec3(1,0,0));
    glm::vec3 offset = glm::vec3(rotY * rotX * glm::vec4(0, 0, distance, 1.0f));
    return target + offset;
}

glm::mat4 CameraOrbit::view() const {
    return glm::lookAt(position(), target, glm::vec3(0,1,0));
}

glm::mat4 CameraOrbit::proj(float aspect) const {
    return glm::perspective(glm::radians(60.0f), aspect, 0.1f, 200.0f);
}

void CameraOrbit::updateFromMouse(
    double mouseX, double mouseY,
    bool lmbDown, bool mmbDown, bool rmbDown,
    bool allowInput
) {
    double dx = mouseX - lastX;
    double dy = mouseY - lastY;
    lastX = mouseX;
    lastY = mouseY;

    // choose controls:
    // - Rotate: RMB drag
    // - Pan:    MMB drag (or Shift+RMB if you want later)
    // - LMB unused (so it doesn't conflict with typical selection later)
    draggingRotate = allowInput && rmbDown;
    draggingPan    = allowInput && mmbDown;

    if (draggingRotate) {
        yawDeg   += (float)dx * rotateSpeed;
        pitchDeg += (float)dy * rotateSpeed;
        pitchDeg = clampf(pitchDeg, -89.0f, 89.0f);
    }

    if (draggingPan) {
        // Pan in camera plane (right/up), scaled by distance
        glm::vec3 camPos = position();
        glm::vec3 fwd = glm::normalize(target - camPos);
        glm::vec3 right = glm::normalize(glm::cross(fwd, glm::vec3(0,1,0)));
        glm::vec3 up = glm::normalize(glm::cross(right, fwd));

        float scale = panSpeed * std::max(0.1f, distance);
        target += (-right * (float)dx + up * (float)dy) * scale;
    }
}

void CameraOrbit::onScroll(double yoffset) {
    // zoom multiplicatively, but clamp
    float zoomFactor = 1.0f - (float)yoffset * zoomSpeed;
    zoomFactor = clampf(zoomFactor, 0.1f, 4.0f);
    distance *= zoomFactor;
    distance = clampf(distance, 0.2f, 200.0f);
}
