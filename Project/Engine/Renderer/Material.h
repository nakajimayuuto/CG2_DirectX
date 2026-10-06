#pragma once
#include "../Math/Vector4.h"
#include "../Math/Matrix4x4.h"
#include <cstdint>
#include <stdfloat>
struct Material {
	Vector4 color;
	int32_t lightingType;
	float padding[3];
	Matrix4x4 uvTransform;
	float shininess;
	int32_t reflectionType;
};

struct MaterialData {
	Material matarial;
	std::string textureFilePath;
	std::string textureName;
};