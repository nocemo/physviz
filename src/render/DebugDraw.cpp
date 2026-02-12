#include "DebugDraw.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

static unsigned CompileShader(GLenum type, const char* src) {
    unsigned sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    int ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetShaderInfoLog(sh, 2048, nullptr, log);
        glDeleteShader(sh);
        return 0;
    }
    return sh;
}

static unsigned LinkProgram(unsigned vs, unsigned fs) {
    unsigned p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    int ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetProgramInfoLog(p, 2048, nullptr, log);
        glDeleteProgram(p);
        return 0;
    }
    glDetachShader(p, vs);
    glDetachShader(p, fs);
    return p;
}

void DebugDraw::clear() { m_lines.clear(); }

void DebugDraw::addLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color) {
    m_lines.push_back({a, color});
    m_lines.push_back({b, color});
}

void DebugDraw::addAxes(float s) {
    addLine({0,0,0}, {s,0,0}, {1,0,0});
    addLine({0,0,0}, {0,s,0}, {0,1,0});
    addLine({0,0,0}, {0,0,s}, {0,0,1});
}

void DebugDraw::addGrid(int halfExtent, float spacing, const glm::vec3& color) {
    const float e = halfExtent * spacing;
    for (int i = -halfExtent; i <= halfExtent; ++i) {
        float x = i * spacing;
        addLine({x, 0, -e}, {x, 0,  e}, color);
        float z = i * spacing;
        addLine({-e, 0, z}, { e, 0, z}, color);
    }
}

void DebugDraw::addCross(const glm::vec3& p, float s, const glm::vec3& c) {
    addLine(p + glm::vec3(-s, 0, 0), p + glm::vec3(+s, 0, 0), c);
    addLine(p + glm::vec3(0, -s, 0), p + glm::vec3(0, +s, 0), c);
    addLine(p + glm::vec3(0, 0, -s), p + glm::vec3(0, 0, +s), c);
}

void DebugDraw::addArrow(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color, float headScale) {
    addLine(from, to, color);

    glm::vec3 dir = to - from;
    float len = glm::length(dir);
    if (len < 1e-6f) return;
    dir /= len;

    // Build a small arrow head in a stable way (pick any perpendicular)
    glm::vec3 up = (std::abs(dir.y) < 0.99f) ? glm::vec3(0,1,0) : glm::vec3(1,0,0);
    glm::vec3 right = glm::normalize(glm::cross(dir, up));
    glm::vec3 headUp = glm::normalize(glm::cross(right, dir));

    float hs = len * headScale;
    glm::vec3 tip = to;
    glm::vec3 base = to - dir * hs;

    addLine(tip, base + right * (hs * 0.5f), color);
    addLine(tip, base - right * (hs * 0.5f), color);
    addLine(tip, base + headUp * (hs * 0.5f), color);
    addLine(tip, base - headUp * (hs * 0.5f), color);
}

bool DebugDraw::init() {
    const char* vsSrc = R"(
        #version 330 core
        layout(location=0) in vec3 aPos;
        layout(location=1) in vec3 aCol;
        uniform mat4 uViewProj;
        out vec3 vCol;
        void main() {
            vCol = aCol;
            gl_Position = uViewProj * vec4(aPos, 1.0);
        }
    )";
    const char* fsSrc = R"(
        #version 330 core
        in vec3 vCol;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(vCol, 1.0);
        }
    )";

    unsigned vs = CompileShader(GL_VERTEX_SHADER, vsSrc);
    unsigned fs = CompileShader(GL_FRAGMENT_SHADER, fsSrc);
    if (!vs || !fs) return false;

    m_prog = LinkProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!m_prog) return false;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)(sizeof(glm::vec3)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
}

void DebugDraw::shutdown() {
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_prog) glDeleteProgram(m_prog);
    m_vbo = m_vao = m_prog = 0;
}

void DebugDraw::draw(const glm::mat4& viewProj) {
    if (m_lines.empty()) return;

    glUseProgram(m_prog);
    glUniformMatrix4fv(glGetUniformLocation(m_prog, "uViewProj"), 1, GL_FALSE, glm::value_ptr(viewProj));

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(m_lines.size() * sizeof(LineVertex)), m_lines.data(), GL_DYNAMIC_DRAW);

    glDrawArrays(GL_LINES, 0, (GLsizei)m_lines.size());

    glBindVertexArray(0);
    glUseProgram(0);
}
