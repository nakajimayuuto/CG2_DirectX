#pragma once
#include <vector>
#include "../Math/Vertex.h"
#include "Material.h"

#include "../../externals/DirectXTex/DirectXTex.h"
#include "../../externals/DirectXTex/d3dx12.h"
struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData materialData;
	std::string meshName;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandlesGPU;
};