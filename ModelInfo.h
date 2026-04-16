#pragma once
#include <vector>
#include "Vertex.h"
#include "Material.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include <cstdint>
#include <string>

struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

struct MaterialData {
	std::string textureFilePath;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

struct ModelInfo {
	Material material;
	ModelData model;
};