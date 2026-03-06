#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
class Vertex4 {
public:
	Vector3 leftTop;
	Vector3 rightTop;
	Vector3 leftBottom;
	Vector3 rightBottom;
};

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
};