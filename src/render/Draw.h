#pragma once
#include <glm/glm.hpp>
#include "Shader.h"
#include "Mesh.h"
#include "RenderQueue.h"

// ライト・カメラ設定（毎フレーム共通）
struct LitParams {
    glm::mat4 viewProj{1.0f};
    glm::vec3 viewPos{0.0f};

    glm::vec3 lightDir{0.0f, -1.0f, 0.5f}; // “光が進む方向”
    glm::vec3 lightColor{1.0f};
    float ambient = 0.12f;
};

// 1メッシュを lit シェーダで描画（uModel, uBaseColor）
inline void DrawMeshLit(Shader& lit, const Mesh& mesh,
                        const LitParams& p,
                        const glm::mat4& model,
                        const glm::vec3& baseColor)
{
    if (!lit.valid() || !mesh.valid()) return;

    lit.use();
    lit.setMat4("uViewProj", p.viewProj);
    lit.setVec3("uViewPos", p.viewPos);

    lit.setVec3("uLightDir", glm::normalize(p.lightDir));
    lit.setVec3("uLightColor", p.lightColor);
    lit.setFloat("uAmbient", p.ambient);

    lit.setMat4("uModel", model);
    lit.setVec3("uBaseColor", baseColor);

    mesh.draw();
}

inline void DrawQueueLit(Shader& lit, const RenderQueue& rq, const LitParams& p)
{
    if (!lit.valid()) return;

    // 共通uniformは1回だけ
    lit.use();
    lit.setMat4("uViewProj", p.viewProj);
    lit.setVec3("uViewPos", p.viewPos);
    lit.setVec3("uLightDir", glm::normalize(p.lightDir));
    lit.setVec3("uLightColor", p.lightColor);
    lit.setFloat("uAmbient", p.ambient);

    for (const auto& it : rq.items) {
        if (!it.mesh || !it.mesh->valid()) continue;
        lit.setMat4("uModel", it.model);
        lit.setVec3("uBaseColor", it.color);
        it.mesh->draw();
    }
}