#include "Collider.h"
void Collider::OnCollision([[maybe_unused]] Collider* other) {
	if (pOnCollision_ != nullptr) {
		(this->*pOnCollision_)();
	}
}

void Collider::SetOnCollisionFunc(void(Collider::*func)()){
	pOnCollision_ = func;
}
