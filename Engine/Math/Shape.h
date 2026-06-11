#pragma once
#include "Vector3.h"

class Matrix4x4;

struct Sphere{
	Vector3 center;
	float radius;
};

struct Line {
	Vector3 origin; // 始点
	Vector3 diff; // 終点への差分ベクトル
};

struct Ray {
	Vector3 origin; // 始点
	Vector3 diff; // 終点への差分ベクトル
};

struct Segment {
	Vector3 origin; // 始点
	Vector3 diff; // 終点への差分ベクトル
};

struct Triangle {
	Vector3 vertices[3]; // 頂点.
};

struct Plane {
	Vector3 normal; // 法線.
	float distance; // 距離.
};

struct AABB {
	Vector3 max; // 最大値.
	Vector3 min; // 最小値.
};

class OBB {
public:
	OBB& operator=(const Matrix4x4 &matrix);
public:
	Vector3 center; // 中心点.
	Vector3 orientations[3]; //座標軸。正規化・直交必須.
	Vector3 size; // 座標軸方向の長さの半分。中心から面までの距離.
};