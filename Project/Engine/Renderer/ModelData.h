#pragma once
#include <vector>
#include "../Math/Vertex.h"
#include "Material.h"

#include "../../externals/DirectXTex/DirectXTex.h"
#include "../../externals/DirectXTex/d3dx12.h"
#include "Node.h"
/// <summary>
/// 1回のドローコールで描画する単位(同じマテリアルを使うメッシュをまとめたもの).
/// </summary>
struct ModelData {
	// CPU側の頂点データ(読み込み時とParticles等の初期化でのみ使用。描画時には使わない).
	std::vector<VertexData> vertices;
	// 描画する頂点数.
	uint32_t vertexCount = 0;
	MaterialData materialData;

	// メッシュ名(同じマテリアルのメッシュをまとめているのでマテリアル名が入る).
	std::string meshName;
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandlesCPU{};
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandlesGPU{};
	// モデル全体で共有している頂点バッファ(このModelDataはその一部をvertexBufferView_で参照する).
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;
	VertexData* vertexData = nullptr;

	Node rootNode;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
};

/// <summary>
/// Modelクラスがインスタンスごとに持つ軽量なメッシュ情報(頂点データのコピーを持たない).
/// </summary>
struct ModelMesh {
	std::string meshName;
	uint32_t vertexCount = 0;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandlesGPU{};
};
