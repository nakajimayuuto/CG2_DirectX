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
	if (colliderType_ == ColliderType::kBox) {
		CreateObbCollider();
		Renderer::GetInstance()->DrawBoxWireFrame(colliderObb_, colliderColor_);
	} else {
		Transform colliderTransform = transform_;
		colliderTransform.scale = { colliderRadius_ * 2.0f,colliderRadius_ * 2.0f ,colliderRadius_ * 2.0f };
		Renderer::GetInstance()->DrawSphereWireFrame(colliderTransform, colliderColor_);
	}
#endif // _DEBUG
}

void Collider::CreateObbCollider(){
	colliderObb_.center = transform_.GetWorldPosition();
	colliderObb_.size.x = colliderSize_.x;
	colliderObb_.size.y = colliderSize_.y;
	colliderObb_.size.z = colliderSize_.z;
	colliderObb_ = transform_.GetRotateMatrix();
}
