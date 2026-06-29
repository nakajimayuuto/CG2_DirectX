#include "Collider.h"
void Collider::OnCollision([[maybe_unused]] Collider* other) {
	if (pOnCollision_ != nullptr) {
		(pOnCollision_)(other);
	}
}

void Collider::SetOnCollisionFunc(void(*func)([[maybe_unused]] Collider* other)) {
	pOnCollision_ = func;
}
