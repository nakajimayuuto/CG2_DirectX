#include "Collider.h"
#include "../Renderer/Renderer.h"

void Collider::DebugDraw() {
	Transform transform = Transform::GetInitialValue({ radius_,radius_,radius_ }, { 0.0f,0.0f,0.0f }, transform_.GetAffineMatrix().GetMatrixToTranslate());
	Renderer::GetInstance()->DrawSphere(transform, "white_template", { 1.0f,1.0f,1.0f,1.0f });
}