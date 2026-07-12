#include "Collider.h"
void Collider::OnCollision([[maybe_unused]] Collider* other) {
	if (pOnCollision_ != nullptr) {
		(pOnCollision_)(other);
	}
}

void Collider::SetOnCollisionFunc(void(*func)([[maybe_unused]] Collider* other)) {
	pOnCollision_ = func;
}

void Collider::CreateObbCollider(){
	obb_.center = transform_.GetWorldPosition();
	obb_.size.x = size_.x;
	obb_.size.y = size_.y;
	obb_ = transform_.GetRotateMatrix();
}
