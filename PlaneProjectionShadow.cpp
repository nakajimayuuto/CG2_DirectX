#include "PlaneProjectionShadow.h"

void PlaneProjectionShadow::Initialize(Transform* casterWorldTransform, Model* model){
	casterTransform_ = casterWorldTransform;
	model_ = model;

	transform_.Initialize();

	shadowMatrix_.Identity();
	shadowMatrix_.matrix[1][1] = 0.0f;

}

void PlaneProjectionShadow::Update() {
}

void PlaneProjectionShadow::Draw() {
	if (casterTransform_) {
		Renderer::GetInstance()->DrawShadow(*casterTransform_,model_, {0.0f,0.0f,0.0f,1.0f});
	}

}