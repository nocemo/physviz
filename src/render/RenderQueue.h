#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Mesh.h"

struct RenderItem {
    const Mesh* mesh = nullptr;
    glm::mat4 model{1.0f};
    glm::vec3 color{1.0f};
};

struct RenderQueue {
    std::vector<RenderItem> items;

    void clear() { items.clear(); }

    void add(const Mesh& mesh, const glm::mat4& model, const glm::vec3& color) {
        items.push_back(RenderItem{ &mesh, model, color });
    }
};
