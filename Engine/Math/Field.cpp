#include "Field.h"
#include "../Renderer/Renderer.h"

void Field::Initialize(const Vector3 acceleration, const AABB& area){
	acceleration_ = acceleration;
	area_ = area;
}

void Field::DebugDraw()const{
	Renderer::GetInstance()->DrawBoxWireFrame(area_, {1.0f,1.0f,1.0f,1.0f});
}
