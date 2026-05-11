#pragma once
#include "Satlib.h"
#include "PlayerBullet.h"
#include <list>

/// <summary>
/// 自キャラ
/// </summary>
class Player{
public:
	~Player();
	void Initialize();

	void Update();

	void Draw();
private:
	void MoveUpdate();

	void RotateUpdate();

	void AttackUpdate();
private:
	static inline float kCharacterSpeed = 0.2f;
	static inline float kRotSpeed = Radian(1.0f);

	static inline float kMoveLimitX = 20.0f;
	static inline float kMoveLimitY = 11.0f;

	static inline float kRotateLimitY = Radian(45.0f);
	
	Transform transform_;

	Renderer::ModelBox model_;

	std::list<PlayerBullet*> bullets_;
};

