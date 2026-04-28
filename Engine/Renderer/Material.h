#pragma once
#include "../Math/Vector4.h"
#include "../Math/Matrix4x4.h"
#include <cstdint>
struct Material {
	Vector4 color;
	int32_t lightingType;
	float padding[3];
	Matrix4x4 uvTransform;
};

struct MaterialData {
	Material matarial;
	std::string textureFilePath;
	std::string textureName;
};