#include "Collider.h"
#include "../Renderer/Renderer.h"
void Collider::OnCollision([[maybe_unused]] Collider* other) {
	if (pOnCollision_ != nullptr) {
		(pOnCollision_)(other);
	}
}

void Collider::SetOnCollisionFunc(void(*func)([[maybe_unused]] Collider* other)) {
	pOnCollision_ = func;
}

void Collider::DrawCollider(){
#ifdef _DEBUG
	Transform colliderTransform = transform_;
	colliderTransform.scale = {radius_ * 2.0f,radius_ * 2.0f ,radius_ * 2.0f };
	Renderer::GetInstance()->DrawSphereWireFrame(colliderTransform,colliderColor_);

	if (colliderType_ == ColliderType::kBox) {
		Renderer::GetInstance()->DrawSphereWireFrame(colliderTransform, colliderColor_);
	}
#endif // _DEBUG
}

void Collider::CreateObbCollider(){
	obb_.center = transform_.GetWorldPosition();
	obb_.size.x = size_.x;
	obb_.size.y = size_.y;
	obb_ = transform_.GetRotateMatrix();
}
