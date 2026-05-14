#pragma once
#include "../Satlib.h"
#include "IScene.h"

#include "../Player.h"
#include "../Skydome.h"
#include "../BaseEnemy.h"
#include "../RailCameraController.h"

class GameScene : public IScene{
public:
	~GameScene();
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void RegisterGlobalVariables();
	void ApplyGlobalVariables();
private:
	void CheckAllCollision();

	void CheckCollisionPair(Collider* colliderA,Collider* colliderB);
private:
	Player* player_ = nullptr;

	BaseEnemy* enemy_ = nullptr;

	Skydome* skydome_ = nullptr;

	RailCameraController* railCameraController_ = nullptr;

	Renderer::Model groundModel_;

	std::vector<Vector3> controlPoints_;
};

