#include "Primitives.h"

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
