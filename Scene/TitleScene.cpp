#include "TitleScene.h"

void TitleScene::Initialize() {
}

void TitleScene::Update() {
}

void TitleScene::Draw() {
	Renderer::GetInstance()->DrawSprite(Transform2D({ 1.1f,1.1f }, 0.0f, { 1.0f,1.0f }).GetTransformValue(), { 1280.0f,720.0f }, "white_template", { 0.1f,0.25f,0.5f,1.0f });
};