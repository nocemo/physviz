#include "Primitives.h"

void MakePlaneXZ(std::vector<VertexP>& outV, std::vector<unsigned int>& outI, float halfSize)
{
    outV.clear();
    outI.clear();

    outV.push_back({ {-halfSize, 0.0f, -halfSize} }); // 0
    outV.push_back({ { halfSize, 0.0f, -halfSize} }); // 1
    outV.push_back({ { halfSize, 0.0f,  halfSize} }); // 2
    outV.push_back({ {-halfSize, 0.0f,  halfSize} }); // 3

    // two triangles
    outI = { 0, 1, 2,  0, 2, 3 };
}
