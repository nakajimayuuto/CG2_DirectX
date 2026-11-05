#include "PrimitiveMesh.h"
#include "../SystemFile/GameSystem.h"
#include <cassert>

void PrimitiveMesh::Initialize(PrimitiveType type, const PrimitiveParam& param) {
	type_ = type;

	CreateVertexBuffer();
	PrimitiveGeometry::WriteVertices(type_, vertexData_, param);

	CreateIndexBuffer();
}

void PrimitiveMesh::Initialize(PrimitiveType type, const PrimitiveParam& param, const PrimitiveMesh& indexSource) {
	assert(type == indexSource.type_);
	type_ = type;

	CreateVertexBuffer();
	PrimitiveGeometry::WriteVertices(type_, vertexData_, param);

	// インデックスは図形の種類で決まるので、同じ種類のメッシュと共有する.
	indexResource_ = indexSource.indexResource_;
	indexBufferView_ = indexSource.indexBufferView_;
	indexCount_ = indexSource.indexCount_;
}

void PrimitiveMesh::InitializeFromModelData(const ModelData& modelData) {
	// 頂点バッファはModelManagerのものを共有する(書き換えないのでMapはしない).
	type_ = PrimitiveType::kBox;
	vertexResource_ = modelData.vertexResource_;
	vertexData_ = nullptr;
	vertexBufferView_ = modelData.vertexBufferView_;
	vertexCount_ = modelData.vertexCount;

	indexResource_ = nullptr;
	indexBufferView_ = {};
	indexCount_ = 0;
}

void PrimitiveMesh::UpdateVertices(const PrimitiveParam& param) {
	// 共有している頂点バッファ(モデル)は書き換えない.
	assert(vertexData_ != nullptr);
	PrimitiveGeometry::WriteVertices(type_, vertexData_, param);
}

void PrimitiveMesh::CreateVertexBuffer() {
	vertexCount_ = PrimitiveGeometry::GetVertexCount(type_);
	const UINT size = static_cast<UINT>(sizeof(VertexData) * vertexCount_);

	vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), size);
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = size;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void PrimitiveMesh::CreateIndexBuffer() {
	indexCount_ = PrimitiveGeometry::GetIndexCount(type_);
	if (indexCount_ == 0) {
		indexResource_ = nullptr;
		indexBufferView_ = {};
		return;
	}

	const UINT size = static_cast<UINT>(sizeof(uint32_t) * indexCount_);
	indexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), size);

	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	PrimitiveGeometry::WriteIndices(type_, indexData);

	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = size;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
}
