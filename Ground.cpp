#include "Ground.h"

void Ground::Initialize() {

}

void Ground::Update() {

}

void Ground::Draw() {
	Renderer::GetInstance()->DrawModel(Transform::GetInitialValue(), "ground", {1.0f,1.0f,1.0f,1.0f});
}