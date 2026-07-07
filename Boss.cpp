#include "Boss.h"

void Boss::Initialize() {
	model_.Initialize("creeking");
	anchorPointCenter_ = Vector3(0.0f, 0.0f, 0.0f);
	anchorPoints_.push_back(anchorPointCenter_);

	for (uint32_t i = 0; i < 8; i++) {
		Vector3 pos = {0.0f,0.0f,0.0f};
		pos.x = Rotate({ 50.0f,0.0f }, {anchorPointCenter_.x,anchorPointCenter_.z},(i * 45.0f)).x;
		pos.z = Rotate({ 50.0f,0.0f }, {anchorPointCenter_.x,anchorPointCenter_.z},(i * 45.0f)).y;
		anchorPoints_.push_back(anchorPointCenter_ + pos);
	}

	for (uint32_t i = 0; i < 8; i++) {
		Vector3 pos = {0.0f,0.0f,0.0f};
		pos.x = Rotate({ 100.0f,0.0f }, {anchorPointCenter_.x,anchorPointCenter_.z},(i * 45.0f) + 22.5f).x;
		pos.z = Rotate({ 100.0f,0.0f }, {anchorPointCenter_.x,anchorPointCenter_.z},(i * 45.0f) + 22.5f).y;
		anchorPoints_.push_back(anchorPointCenter_ + pos);
	}
}

void Boss::Update() {

}

void Boss::Draw() {
	Renderer* renderer = Renderer::GetInstance();
	renderer->DrawModel(transform_,&model_,true);
	renderer->DrawShadow(transform_, &model_, {0.0f,0.0f,0.0f,1.0f});

	for (Vector3& pos :anchorPoints_) {
		renderer->DrawSphereWireFrame(Transform::GetInitialValue({ 0.1f,0.1f,0.1f }, { 0.0f,0.0f,0.0f }, pos ), {1.0f,1.0f,1.0f,1.0f});
	}
}

void Boss::OnCollision(Collider* other){
}
