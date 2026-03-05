#pragma once
#include <vector>
#include "Mesh.h"

// XZ平面に置かれる 2x2 の平面（中心原点、y=0）
void MakePlaneXZ(std::vector<VertexPN>& outV, std::vector<unsigned int>& outI, float halfSize);

void MakeUvSphere(std::vector<VertexPN>& outV,
                  std::vector<unsigned int>& outI,
                  float radius,
                  int slices,
                  int stacks);