#include "GameScene.h"
#include "../Satlib.h"

GameScene::~GameScene() {
}

void GameScene::Initialize() {
}

void GameScene::Update() {
}

void GameScene::Draw() {
}

void GameScene::CheckAllCollisions() {
	CollisionManager::GetInstance()->CheckAllCollision();
}
