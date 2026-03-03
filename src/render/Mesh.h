#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

struct VertexPN {
    glm::vec3 pos;
    glm::vec3 normal;
};

class Mesh {
public:
    Mesh() = default;
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    bool upload(const std::vector<VertexPN>& vertices,
                const std::vector<unsigned int>& indices);

    void destroy();

    void draw() const;

    bool valid() const { return m_vao != 0; }

private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    GLsizei m_indexCount = 0;
};
