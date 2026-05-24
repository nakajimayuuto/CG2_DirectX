#pragma once
//#include "../Engine/Renderer/Renderer.h"
//#include "../Managers/ModelManager.h"
#include "../Satlib.h"
#include "IScene.h"
#include <list>

class GameScene : public IScene {
public:
	enum class State {
		kTriangleDebug,
		kTriangleEffect,
		kTriangleEffectAnimation,
	};

	enum class AnimationPhase {
		kOpen,
		kStay,
		kClose,
	};

	~GameScene();
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
	void EffectInitialize();

	void EffectUpdate();
	
	void EffectAnimationInitialize();

	void EffectAnimationUpdate();

	void CreateTriangle(const Vector3& position);

	void DeleteTriangle();
private:
	// リストボックスに入れるアイテムの構造体.
	struct GuiItem {
		const char* name;
		bool isSelect;
	};

	// 三角形を複数生成しやすくするための構造体. 
	struct TriangleData {
		Renderer::ModelTriangle model;

		Transform transform;

		GuiItem lightingType[3];

		GuiItem textureType[3];

		uint32_t number;

		bool isDelete;
	};

	// デバッグ用三角形.
	std::list<TriangleData> triangleDatas_;

	// デバッグ用三角形の数.
	uint32_t triangleIndex_;

	// 演出用の変数.
	Transform parentTransform_;
	Transform parentTransformMini_;

	static inline const uint32_t kEffectTriangle = 3;
	static inline const uint32_t kEffectTriangleMini = 3;

	std::array<TriangleData, kEffectTriangle + kEffectTriangleMini + 1> effectTriangleData_;

	float animationTimer_;

	static inline const float kOpenAnimationMax = 0.7f;
	static inline const float kStayAnimationMax = 0.1f;
	static inline const float kCloseAnimationMax = 0.7f;

	static inline const float kRotateSpeed = 60.0f;
	static inline const float kRotateActionSpeed = 120.0f;

	static inline const float kTrianglePositionZ = 0.0f;
	static inline const float kTriangleActionPositionZ = -2.0f;

	State state_;

	AnimationPhase phase_;

	DeltaTime* deltaTime_;
};

