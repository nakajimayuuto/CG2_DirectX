#include "Boss.h"

Boss::~Boss(){
	delete targetTransform;
}

void Boss::Initialize() {
	model_.Initialize("creeking");
	anchorPointCenter_ = Vector3(0.0f, 0.5f, 0.0f);
	anchorPoints_.push_back(anchorPointCenter_);

	for (uint32_t i = 0; i < 8; i++) {
		Vector3 pos = {0.0f,0.5f,0.0f};
		pos.x = Rotate({ 30.0f,0.0f }, {anchorPointCenter_.x,anchorPointCenter_.z},(i * 45.0f)).x;
		pos.z = Rotate({ 30.0f,0.0f }, {anchorPointCenter_.x,anchorPointCenter_.z},(i * 45.0f)).y;
		anchorPoints_.push_back(anchorPointCenter_ + pos);
	}

	for (uint32_t i = 0; i < 16; i++) {
		Vector3 pos = {0.0f,0.5f,0.0f};
		pos.x = Rotate({ 60.0f,0.0f }, {anchorPointCenter_.x,anchorPointCenter_.z},(i * 22.5f)).x;
		pos.z = Rotate({ 60.0f,0.0f }, {anchorPointCenter_.x,anchorPointCenter_.z},(i * 22.5f)).y;
		anchorPoints_.push_back(anchorPointCenter_ + pos);
	}
}

void Boss::Update() {
	ImGui::Begin("BossDebug");
	Vector3 imRotate = Degree(transform_.rotate);
	ImGui::DragFloat3("scale",reinterpret_cast<float*>(&transform_.scale),0.05f,0.0f,5.0f);
	ImGui::DragFloat3("rotate",reinterpret_cast<float*>(&imRotate),1.0f,-360.0f,360.0f);
	ImGui::DragFloat3("translate",reinterpret_cast<float*>(&transform_.translate),0.25f,-100.0f,100.0f);
	transform_.rotate = Radian(imRotate);
	ImGui::End();





}

void Boss::Draw() {
	Renderer* renderer = Renderer::GetInstance();
	renderer->DrawModel(transform_,&model_,true);
	renderer->DrawShadow(transform_, &model_, {0.0f,0.0f,0.0f,1.0f});

	for (Vector3& pos :anchorPoints_) {
		renderer->DrawSphereWireFrame(Transform::GetInitialValue({ 0.1f,0.1f,0.1f }, { 0.0f,0.0f,0.0f }, pos ), {0.5f,0.5f,1.0f,1.0f});
	}
}

void Boss::OnCollision(Collider* other){
}

void Boss::WarpInitialize(){
}

void Boss::WarpUpdate(){
}
