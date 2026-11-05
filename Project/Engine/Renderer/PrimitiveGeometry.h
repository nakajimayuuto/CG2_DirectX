#pragma once
#include <cstdint>
#include "../Math/Vertex.h"

/// <summary>
/// Primitive図形の種類(Renderer / Particles 共通).
/// </summary>
enum class PrimitiveType {
	kPlane,
	kBox,
	kSphere,
	kTours,
	kRing,
	kCylinder,
};

/// <summary>
/// Primitive図形の形を決めるパラメータ.
/// </summary>
struct PrimitiveParam {
	// Torus: 中心からチューブ中心までの半径 / Ring: 外側の半径 / Cylinder: 上の半径.
	float radiusA = 1.0f;
	// Torus: チューブの半径 / Ring: 内側の半径 / Cylinder: 下の半径.
	float radiusB = 0.5f;
};

/// <summary>
/// Primitive図形の頂点・インデックスを作る処理をまとめたもの.
/// Renderer(1つずつ描画)とParticles(インスタンシング描画)の両方から使う.
/// </summary>
namespace PrimitiveGeometry {
	// 分割数.
	inline constexpr uint32_t kSphereSubdivision = 16;
	inline constexpr uint32_t kTorusSubdivision = 32;
	inline constexpr uint32_t kRingDivision = 32;
	inline constexpr uint32_t kCylinderDivision = 32;

	/// <summary>
	/// 頂点数を返す.
	/// </summary>
	uint32_t GetVertexCount(PrimitiveType type);

	/// <summary>
	/// インデックス数を返す(インデックスを使わない図形は0).
	/// </summary>
	uint32_t GetIndexCount(PrimitiveType type);

	/// <summary>
	/// 半径などのパラメータで形が変わる図形か(変わらない図形は1度作れば使い回せる).
	/// </summary>
	bool IsDynamic(PrimitiveType type);

	/// <summary>
	/// 頂点を書き込む(GetVertexCount分の領域が必要).
	/// </summary>
	void WriteVertices(PrimitiveType type, VertexData* vertexData, const PrimitiveParam& param);

	/// <summary>
	/// インデックスを書き込む(GetIndexCount分の領域が必要).
	/// </summary>
	void WriteIndices(PrimitiveType type, uint32_t* indexData);
}
