#include "Ground.h"

void Ground::Initialize() {

}

void Ground::Update() {

}

void Ground::Draw() {
	Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,1.0f ,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,-180.0f}), "ground", { 1.0f,1.0f,1.0f,1.0f }, false);
}