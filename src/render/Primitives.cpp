#include "Primitives.h"
#include <cmath>

void MakePlaneXZ(std::vector<VertexPN>& outV, std::vector<unsigned int>& outI, float halfSize)
{
    outV.clear();
    outI.clear();

    glm::vec3 n(0.0f, 1.0f, 0.0f);

    outV.push_back({ {-halfSize, 0.0f, -halfSize}, n }); // 0
    outV.push_back({ { halfSize, 0.0f, -halfSize}, n }); // 1
    outV.push_back({ { halfSize, 0.0f,  halfSize}, n }); // 2
    outV.push_back({ {-halfSize, 0.0f,  halfSize}, n }); // 3

    outI = { 0, 1, 2,  0, 2, 3 };
}

void MakeUvSphere(std::vector<VertexPN>& outV,
                  std::vector<unsigned int>& outI,
                  float radius,
                  int slices,
                  int stacks)
{
    outV.clear();
    outI.clear();

    slices = std::max(slices, 3);
    stacks = std::max(stacks, 2);

    const float pi = 3.14159265358979323846f;

    // vertices
    // phi: 0..pi (top to bottom), theta: 0..2pi
    for (int y = 0; y <= stacks; ++y) {
        float v = (float)y / (float)stacks;     // 0..1
        float phi = v * pi;                     // 0..pi
        float sp = std::sin(phi);
        float cp = std::cos(phi);

        for (int x = 0; x <= slices; ++x) {
            float u = (float)x / (float)slices; // 0..1
            float theta = u * (2.0f * pi);      // 0..2pi
            float st = std::sin(theta);
            float ct = std::cos(theta);

            glm::vec3 n(ct * sp, cp, st * sp);   // unit normal
            glm::vec3 p = radius * n;

            outV.push_back({ p, glm::normalize(n) });
        }
    }

    // indices (two triangles per quad)
    int stride = slices + 1;
    for (int y = 0; y < stacks; ++y) {
        for (int x = 0; x < slices; ++x) {
            int i0 = y * stride + x;
            int i1 = i0 + 1;
            int i2 = i0 + stride;
            int i3 = i2 + 1;

            // winding CCW
            outI.push_back((unsigned int)i0);
            outI.push_back((unsigned int)i2);
            outI.push_back((unsigned int)i1);

            outI.push_back((unsigned int)i1);
            outI.push_back((unsigned int)i2);
            outI.push_back((unsigned int)i3);
        }
    }
}
