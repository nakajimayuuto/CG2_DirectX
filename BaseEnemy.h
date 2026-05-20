#pragma once
#include "Satlib.h"
#include "Player.h"

class BaseEnemy : public Collider {
public:
	BaseEnemy() = default;
	~BaseEnemy() override = default;
	virtual void Initialize(Vector3 position) = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	
	void OnCollision() override = 0;

	bool GetIsAlive() { return isAlive_; };
	void SetTarget(std::unique_ptr<Player> player) { player_ = std::move(player); };
	Vector3 GetWorldPosition() override{ return { transform_.GetAffineMatrix().matrix[3][0],transform_.GetAffineMatrix().matrix[3][1],transform_.GetAffineMatrix().matrix[3][2] }; };

	Transform GetTransform()const { return transform_; };
protected:
	bool isAlive_ = false;

	std::unique_ptr<Player> player_ = nullptr;

	Transform transform_;
	Renderer::Model model_;
};

