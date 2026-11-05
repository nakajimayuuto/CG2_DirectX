#include "PrimitiveGeometry.h"
#include "../../Managers/ModelManager.h"
#include <cmath>
#include <cstring>
#include <numbers>
#include <cassert>

namespace {
	// 箱は block_template モデルの頂点をそのまま使う(DrawBoxと同じ見た目にするため).
	const ModelData& GetBoxModelData() {
		return ModelManager::GetInstance()->GetModelInfo("block_template").modelData[0];
	}

	/// <summary>
	/// 「1分割 = 頂点4つ(0~3)の四角形」が並んだ形状のインデックスを書き込む(Sphere/Plane/Ring/Cylinder共通).
	/// </summary>
	void WriteQuadIndices(uint32_t* indexData, uint32_t quadCount) {
		for (uint32_t i = 0; i < quadCount; i++) {
			const uint32_t index = i * 6;  // 書き込み先(この四角形のインデックスの先頭).
			const uint32_t vertex = i * 4; // 参照する頂点(この四角形の頂点の先頭).
			indexData[index] = vertex;
			indexData[index + 1] = vertex + 1;
			indexData[index + 2] = vertex + 2;
			indexData[index + 3] = vertex + 1;
			indexData[index + 4] = vertex + 3;
			indexData[index + 5] = vertex + 2;
		}
	}

	void WritePlaneVertices(VertexData* vertexData) {
		const Vector3 normal = { 0.0f,0.0f,-1.0f };
		vertexData[0] = { { -0.5f,-0.5f,0.0f,1.0f },{ 0.0f,0.0f },normal };
		vertexData[1] = { { -0.5f, 0.5f,0.0f,1.0f },{ 0.0f,1.0f },normal };
		vertexData[2] = { {  0.5f,-0.5f,0.0f,1.0f },{ 1.0f,0.0f },normal };
		vertexData[3] = { {  0.5f, 0.5f,0.0f,1.0f },{ 1.0f,1.0f },normal };
	}

	void WriteSphereVertices(VertexData* vertexData) {
		const uint32_t kSubdivision = PrimitiveGeometry::kSphereSubdivision;
		const float pi = std::numbers::pi_v<float>;
		const float kLonEvery = pi * 2.0f / kSubdivision;
		const float kLatEvery = pi / kSubdivision;
		const float kTexEvery = 1.0f / static_cast<float>(kSubdivision);

		for (uint32_t latIndex = 0; latIndex < kSubdivision; latIndex++) {
			float lat = -pi / 2.0f + kLatEvery * latIndex;
			float latNext = lat + kLatEvery;

			for (uint32_t lonIndex = 0; lonIndex < kSubdivision; lonIndex++) {
				uint32_t start = (latIndex * kSubdivision + lonIndex) * 4;
				float lon = lonIndex * kLonEvery;
				float lonNext = lon + kLonEvery;

				float u = static_cast<float>(lonIndex) / static_cast<float>(kSubdivision);
				float v = 1.0f - static_cast<float>(latIndex) / static_cast<float>(kSubdivision);

				vertexData[start].position = { cos(lat) * cos(lon),sin(lat),cos(lat) * sin(lon),1.0f };
				vertexData[start].texcoord = { u - kTexEvery,v };
				vertexData[start + 1].position = { cos(latNext) * cos(lon),sin(latNext),cos(latNext) * sin(lon),1.0f };
				vertexData[start + 1].texcoord = { u - kTexEvery,v - kTexEvery };
				vertexData[start + 2].position = { cos(lat) * cos(lonNext),sin(lat),cos(lat) * sin(lonNext),1.0f };
				vertexData[start + 2].texcoord = { u,v };
				vertexData[start + 3].position = { cos(latNext) * cos(lonNext),sin(latNext),cos(latNext) * sin(lonNext),1.0f };
				vertexData[start + 3].texcoord = { u,v - kTexEvery };

				// 単位球なので法線は座標と同じ.
				for (uint32_t i = 0; i < 4; i++) {
					vertexData[start + i].normal = { vertexData[start + i].position.x,vertexData[start + i].position.y,vertexData[start + i].position.z };
				}
			}
		}
	}

	void WriteTorusVertices(VertexData* vertexData, float majorRadius, float minorRadius) {
		const uint32_t kSubdivision = PrimitiveGeometry::kTorusSubdivision;
		const float pi = std::numbers::pi_v<float>;
		const uint32_t vertexPerRow = kSubdivision + 1;

		for (uint32_t y = 0; y <= kSubdivision; y++) {
			float v = static_cast<float>(y) / kSubdivision;
			float theta = v * pi * 2.0f;
			float cosTheta = std::cos(theta);
			float sinTheta = std::sin(theta);

			for (uint32_t x = 0; x <= kSubdivision; x++) {
				uint32_t index = y * vertexPerRow + x;
				float u = static_cast<float>(x) / kSubdivision;
				float phi = u * pi * 2.0f;
				float cosPhi = std::cos(phi);
				float sinPhi = std::sin(phi);

				// チューブ中心.
				Vector3 center = { majorRadius * cosTheta,0.0f,majorRadius * sinTheta };
				// 法線.
				Vector3 normal = { cosTheta * cosPhi,sinPhi,sinTheta * cosPhi };
				normal = normal.Normalize();
				// 頂点座標.
				Vector3 position = center + normal * minorRadius;

				vertexData[index].position = { position.x,position.y,position.z,1.0f };
				vertexData[index].normal = normal;
				vertexData[index].texcoord = { u,v };
			}
		}
	}

	void WriteTorusIndices(uint32_t* indexData) {
		const uint32_t kSubdivision = PrimitiveGeometry::kTorusSubdivision;
		const uint32_t stride = kSubdivision + 1;

		for (uint32_t y = 0; y < kSubdivision; y++) {
			for (uint32_t x = 0; x < kSubdivision; x++) {
				uint32_t index = (y * kSubdivision + x) * 6;
				uint32_t a = y * stride + x;
				uint32_t b = a + 1;
				uint32_t c = a + stride;
				uint32_t d = c + 1;

				indexData[index] = a;
				indexData[index + 1] = b;
				indexData[index + 2] = c;
				indexData[index + 3] = b;
				indexData[index + 4] = d;
				indexData[index + 5] = c;
			}
		}
	}

	void WriteRingVertices(VertexData* vertexData, float outerRadius, float innerRadius) {
		const uint32_t kDivision = PrimitiveGeometry::kRingDivision;
		// 1分割あたりの角度.
		const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kDivision);
		// リングは-Z方向(カメラ側)を向ける.
		const Vector3 normal = { 0.0f,0.0f,-1.0f };

		for (uint32_t index = 0; index < kDivision; ++index) {
			float sin = std::sin(radianPerDivide * static_cast<float>(index));
			float cos = std::cos(radianPerDivide * static_cast<float>(index));
			float sinNext = std::sin(radianPerDivide * static_cast<float>(index + 1));
			float cosNext = std::cos(radianPerDivide * static_cast<float>(index + 1));
			float u = static_cast<float>(index) / static_cast<float>(kDivision);
			float uNext = static_cast<float>(index + 1) / static_cast<float>(kDivision);

			// 0:外側 1:内側 2:次の外側 3:次の内側(WriteQuadIndicesで時計回りになる順番).
			vertexData[(index * 4)] = { { -sin * outerRadius,cos * outerRadius,0.0f,1.0f },{ u,0.0f },normal };
			vertexData[(index * 4) + 1] = { { -sin * innerRadius,cos * innerRadius,0.0f,1.0f },{ u,1.0f },normal };
			vertexData[(index * 4) + 2] = { { -sinNext * outerRadius,cosNext * outerRadius,0.0f,1.0f },{ uNext,0.0f },normal };
			vertexData[(index * 4) + 3] = { { -sinNext * innerRadius,cosNext * innerRadius,0.0f,1.0f },{ uNext,1.0f },normal };
		}
	}

	void WriteCylinderVertices(VertexData* vertexData, float topRadius, float bottomRadius) {
		const uint32_t kDivision = PrimitiveGeometry::kCylinderDivision;
		// 1分割あたりの角度.
		const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kDivision);
		const float kHeight = 1.0f;
		const float top = kHeight / 2.0f;
		const float bottom = -kHeight / 2.0f;

		for (uint32_t index = 0; index < kDivision; ++index) {
			float sin = std::sin(radianPerDivide * static_cast<float>(index));
			float cos = std::cos(radianPerDivide * static_cast<float>(index));
			float sinNext = std::sin(radianPerDivide * static_cast<float>(index + 1));
			float cosNext = std::cos(radianPerDivide * static_cast<float>(index + 1));
			float u = static_cast<float>(index) / static_cast<float>(kDivision);
			float uNext = static_cast<float>(index + 1) / static_cast<float>(kDivision);

			// 0:上 1:次の上 2:下 3:次の下(WriteQuadIndicesで外側から見て時計回りになる順番).
			vertexData[(index * 4)] = { { -sin * topRadius,top,cos * topRadius,1.0f },{ u,0.0f },{ -sin,0.0f,cos } };
			vertexData[(index * 4) + 1] = { { -sinNext * topRadius,top,cosNext * topRadius,1.0f },{ uNext,0.0f },{ -sinNext,0.0f,cosNext } };
			vertexData[(index * 4) + 2] = { { -sin * bottomRadius,bottom,cos * bottomRadius,1.0f },{ u,1.0f },{ -sin,0.0f,cos } };
			vertexData[(index * 4) + 3] = { { -sinNext * bottomRadius,bottom,cosNext * bottomRadius,1.0f },{ uNext,1.0f },{ -sinNext,0.0f,cosNext } };
		}
	}
}

namespace PrimitiveGeometry {
	uint32_t GetVertexCount(PrimitiveType type) {
		switch (type) {
		case PrimitiveType::kPlane:    return 4;
		case PrimitiveType::kBox:      return GetBoxModelData().vertexCount;
		case PrimitiveType::kSphere:   return kSphereSubdivision * kSphereSubdivision * 4;
		case PrimitiveType::kTours:    return (kTorusSubdivision + 1) * (kTorusSubdivision + 1);
		case PrimitiveType::kRing:     return kRingDivision * 4;
		case PrimitiveType::kCylinder: return kCylinderDivision * 4;
		}
		assert(false);
		return 0;
	}

	uint32_t GetIndexCount(PrimitiveType type) {
		switch (type) {
		case PrimitiveType::kPlane:    return 6;
		case PrimitiveType::kBox:      return 0; // モデルの頂点をそのまま並べて描画する.
		case PrimitiveType::kSphere:   return kSphereSubdivision * kSphereSubdivision * 6;
		case PrimitiveType::kTours:    return kTorusSubdivision * kTorusSubdivision * 6;
		case PrimitiveType::kRing:     return kRingDivision * 6;
		case PrimitiveType::kCylinder: return kCylinderDivision * 6;
		}
		assert(false);
		return 0;
	}

	bool IsDynamic(PrimitiveType type) {
		return type == PrimitiveType::kTours || type == PrimitiveType::kRing || type == PrimitiveType::kCylinder;
	}

	void WriteVertices(PrimitiveType type, VertexData* vertexData, const PrimitiveParam& param) {
		switch (type) {
		case PrimitiveType::kPlane:
			WritePlaneVertices(vertexData);
			break;
		case PrimitiveType::kBox: {
			const ModelData& box = GetBoxModelData();
			std::memcpy(vertexData, box.vertices.data(), sizeof(VertexData) * box.vertexCount);
			break;
		}
		case PrimitiveType::kSphere:
			WriteSphereVertices(vertexData);
			break;
		case PrimitiveType::kTours:
			WriteTorusVertices(vertexData, param.radiusA, param.radiusB);
			break;
		case PrimitiveType::kRing:
			WriteRingVertices(vertexData, param.radiusA, param.radiusB);
			break;
		case PrimitiveType::kCylinder:
			WriteCylinderVertices(vertexData, param.radiusA, param.radiusB);
			break;
		}
	}

	void WriteIndices(PrimitiveType type, uint32_t* indexData) {
		switch (type) {
		case PrimitiveType::kPlane:
			WriteQuadIndices(indexData, 1);
			break;
		case PrimitiveType::kBox:
			break;
		case PrimitiveType::kSphere:
			WriteQuadIndices(indexData, kSphereSubdivision * kSphereSubdivision);
			break;
		case PrimitiveType::kTours:
			WriteTorusIndices(indexData);
			break;
		case PrimitiveType::kRing:
			WriteQuadIndices(indexData, kRingDivision);
			break;
		case PrimitiveType::kCylinder:
			WriteQuadIndices(indexData, kCylinderDivision);
			break;
		}
	}
}
