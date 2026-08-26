#pragma once
#include "../Satlib.h"
#include "IScene.h"
#include "../Ground.h"
#include "../Fade.h"

enum class TitlePhase {
	kTitle,
	kMenuSelect,
	kDifficultySelect,
	kSetting,
	kCount
};

class TitleScene : public IScene {
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
	bool TriggerUp();
	bool TriggerDown();

	bool TriggerSubmit();

	bool TriggerChancel();
private:
	bool preStickUpUse_;
	bool preStickDownUse_;
private:
	void EaseTimerUpdate(TitlePhase incrimentTimer);

	void PhaseSelect();

	void TitleUpdate();
	void MenuUpdate();
	void DiffucltyUpdate();
	void SettingUpdate();
private:
	std::unique_ptr<Ground> ground_;
	std::unique_ptr<Fade> fade_;
	Transform center_;
	Transform cameraTransform_;
	std::unique_ptr<Emitter> worldFrameEmitter_;
	std::unique_ptr<Emitter> worldBigFrameEmitter_;

	static inline float kRotateAngleSpeed = Radian(15.0f);

	bool isSubmit_;
	TitlePhase currentPhase_;

	int32_t currentMenuSelectNum_;
	int32_t currentDifficultySelectNum_;
	
	//static inline Vector3 kUIAnimScaleNormal_ = { 1.0f,1.0f,1.0f };
	//static inline Vector3 kUIAnimScaleMag_ = { 1.5f,1.5f,1.0f };
	static inline float kUIAnimEaseTimerMax_ = 0.5f;
	std::array<float, static_cast<size_t>(TitlePhase::kCount)> easeTimer_;

	static inline float kPressAPosY = 200.0f;
	static inline float kMenuPosX = -400.0f;
};