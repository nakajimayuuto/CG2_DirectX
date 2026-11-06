#pragma once
#include "Vector3.h"
#include "Quaternion.h"
template<typename tValue>
struct Keyframe{
	tValue value;
	float time;
};

using KeyframeVector3 = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;