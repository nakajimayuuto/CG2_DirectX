#include "PlaneProjectionShadow.h"

void PlaneProjectionShadow::Initialize(Transform* casterWorldTransform, Model* model) {
	casterTransform_ = casterWorldTransform;
	model_ = model;

	transform_.Initialize();

	// 【ShadowMatrixを単位行列で初期化】
	shadowMatrix_ = Matrix4x4::Identity();

	// 【ShadowMatrixの要素[1][1]に0.0fを代入】
	shadowMatrix_.matrix[1][1] = 0.01f;

}

void PlaneProjectionShadow::Update() {
	transform_ = *casterTransform_;
	worldMatrix_ = transform_.GetAffineMatrix();

	Camera* camera = Camera::GetInstance();

	if (casterTransform_) {
		worldMatrix_ = worldMatrix_ * shadowMatrix_;
	}
}

void PlaneProjectionShadow::Draw() {
	Renderer::GetInstance()->DrawModel(worldMatrix_, model_);

}