#include "Mesh.h"
#include <iostream>

Mesh::~Mesh() {
    destroy();
}

void Mesh::destroy() {
    if (m_ebo) { glDeleteBuffers(1, &m_ebo); m_ebo = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    m_indexCount = 0;
}

bool Mesh::upload(const std::vector<VertexPN>& vertices,
                  const std::vector<unsigned int>& indices)
{
    destroy();

    if (vertices.empty() || indices.empty()) {
        std::cerr << "[Mesh] upload failed: empty buffers\n";
        return false;
    }

    m_indexCount = (GLsizei)indices.size();

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(vertices.size() * sizeof(VertexPN)),
                 vertices.data(),
                 GL_STATIC_DRAW);

    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (GLsizeiptr)(indices.size() * sizeof(unsigned int)),
                 indices.data(),
                 GL_STATIC_DRAW);

    // layout(location=0) vec3 aPos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                        sizeof(VertexPN),
                        (void*)offsetof(VertexPN, pos));

    // layout(location=1) vec3 aNormal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                        sizeof(VertexPN),
                        (void*)offsetof(VertexPN, normal));

    glBindVertexArray(0);
    return true;
}

void Mesh::draw() const
{
    if (!m_vao || m_indexCount <= 0) return;
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
