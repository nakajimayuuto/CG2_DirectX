#include "Collider.h"
void Collider::OnCollision([[maybe_unused]] Collider* other) {
	if (pOnCollision_ != nullptr) {
		(this->*pOnCollision_)(other);
	}
}

void Collider::SetOnCollisionFunc(void(Collider::*func)([[maybe_unused]] Collider* other)){
	pOnCollision_ = func;
}
