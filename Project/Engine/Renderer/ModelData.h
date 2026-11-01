#pragma once
#include <vector>
#include "../Math/Vertex.h"
#include "Material.h"

#include "../../externals/DirectXTex/DirectXTex.h"
#include "../../externals/DirectXTex/d3dx12.h"
struct Node {
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData materialData;

	Node rootNode;
	std::string meshName;
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandlesCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandlesGPU;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;
	VertexData* vertexData;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
};
