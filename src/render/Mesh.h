#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

struct VertexP {
    glm::vec3 pos;
};

class Mesh {
public:
    Mesh() = default;
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    bool upload(const std::vector<VertexP>& vertices,
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
