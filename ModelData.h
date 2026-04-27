#pragma once
#include <vector>
#include "Vertex.h"
#include "Material.h"
struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData materialData;
};