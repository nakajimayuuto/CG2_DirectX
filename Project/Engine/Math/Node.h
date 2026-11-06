#pragma once
#include "Matrix4x4.h"
#include <string>
#include <vector>
#include <map>
#include "Keyframe.h"
struct Node {
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

template<typename tValue>
struct AnimationCurve {
	std::vector<Keyframe<tValue>> keyframes;
};

template<typename tValue>
struct NodeAnimation {
	AnimationCurve<Vector3> translate;
	AnimationCurve<tValue> rotate;
	AnimationCurve<Vector3> scale;
};

struct Animation {
	float duration;
	std::map<std::string,NodeAnimation<Quaternion>> nodeAnimations;
};