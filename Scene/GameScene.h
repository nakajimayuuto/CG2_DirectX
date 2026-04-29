#pragma once
#include "../Satlib.h"
#include "IScene.h"

#include "../Player.h"
#include "../Skydome.h"
#include <vector>

class GameScene : public IScene{
public:
	~GameScene();
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
	// 要素数.
	const uint32_t kNumBlockVirtical = 20;
	const uint32_t kNumBlockHorizontal = 10;
	// ブロック1個分の横幅.
	const float kBlockWidth = 1.0f;
	const float kBlockHeight = 1.0f;

	std::vector<std::vector<Transform*>> transformBlocks_;
	std::vector<std::vector<Renderer::ModelBox*>> modelBlocks_;

	Player* player_ = nullptr;

	Skydome* skydome_ = nullptr;
};

