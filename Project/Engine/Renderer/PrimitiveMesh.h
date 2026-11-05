#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "PrimitiveGeometry.h"
#include "ModelData.h"

/// <summary>
/// 描画に使う頂点バッファ(+インデックスバッファ)をまとめたもの.
/// Primitive図形やモデルのメッシュを、Renderer / Particles で同じ形で扱うために使う.
/// </summary>
class PrimitiveMesh {
public:
	/// <summary>
	/// Primitive図形のメッシュを作る(頂点・インデックスバッファを生成して書き込む).
	/// </summary>
	void Initialize(PrimitiveType type, const PrimitiveParam& param = {});

	/// <summary>
	/// インデックスバッファを他のメッシュと共有してPrimitive図形のメッシュを作る.
	/// (同じ種類の図形を大量に用意する時に、インデックスバッファを1つで済ませるため).
	/// </summary>
	void Initialize(PrimitiveType type, const PrimitiveParam& param, const PrimitiveMesh& indexSource);

	/// <summary>
	/// モデル(ModelManagerのデータ)の頂点バッファをそのまま使うメッシュを作る(コピーしない).
	/// </summary>
	void InitializeFromModelData(const ModelData& modelData);

	/// <summary>
	/// パラメータ(半径など)を変えて頂点を書き直す.
	/// </summary>
	void UpdateVertices(const PrimitiveParam& param);

	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return vertexBufferView_; };

	/// <summary>
	/// インデックスバッファビュー(インデックスを使わないメッシュはnullptr).
	/// </summary>
	const D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView() const { return indexCount_ > 0 ? &indexBufferView_ : nullptr; };

	/// <summary>
	/// 描画数(インデックスを使う場合はインデックス数、使わない場合は頂点数).
	/// </summary>
	uint32_t GetDrawCount() const { return indexCount_ > 0 ? indexCount_ : vertexCount_; };

	bool UseIndex() const { return indexCount_ > 0; };

	PrimitiveType GetType() const { return type_; };
private:
	void CreateVertexBuffer();
	void CreateIndexBuffer();
private:
	PrimitiveType type_ = PrimitiveType::kPlane;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	VertexData* vertexData_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	uint32_t vertexCount_ = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	uint32_t indexCount_ = 0;
};
