#pragma once
#include <vector>
#include <glm/glm.hpp>

struct LineVertex {
    glm::vec3 p;
    glm::vec3 c;
};

class DebugDraw {
public:
    void clear();
    void addLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color);
    void addAxes(float scale = 1.0f);
    void addGrid(int halfExtent = 10, float spacing = 1.0f, const glm::vec3& color = {0.3f,0.3f,0.3f});
    void addArrow(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color, float headScale = 0.15f);
    void addCross(const glm::vec3& p, float size, const glm::vec3& color);
    
    // OpenGL resources
    bool init();
    void shutdown();
    void draw(const glm::mat4& viewProj);

private:
    std::vector<LineVertex> m_lines;

    unsigned m_vao = 0;
    unsigned m_vbo = 0;
    unsigned m_prog = 0;
};
