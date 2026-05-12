#pragma once
#include "Satlib.h"
#include "Collider.h"

class Player;

class BaseEnemy : public Collider {
public:
	virtual void Initialize(Vector3 position) = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	
	void OnCollision() override = 0;

	bool GetIsAlive() { return isAlive_; };
	void SetPlayer(Player* player) { player_ = player; };
	Vector3 GetWorldPosition() override{ return { transform_.GetAffineMatrix().matrix[3][0],transform_.GetAffineMatrix().matrix[3][1],transform_.GetAffineMatrix().matrix[3][2] }; };

	Transform GetTransform()const { return transform_; };
protected:
	bool isAlive_ = false;

	Player* player_ = nullptr;

	Transform transform_;
	Renderer::ModelBox model_;
};

