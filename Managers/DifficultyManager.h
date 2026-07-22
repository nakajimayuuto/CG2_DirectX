#pragma once
enum Difficulty{
	kDifficultyEasy,
	kDifficultyNormal,
	kDifficultyHard,
	kDifficultyHell,
	kDifficultyCount,
};

class DifficultyManager{
public:
	static DifficultyManager* GetInstance();
	
	void Initialize();

	void Update();

	float GetDopamineSpeed() { return dopamineSpeed_; };

	void SetPlayerHPData(float* hp, float hpMax) { playerHP = hp; playerHPMax = hpMax; };
	void SetBossHPData(float* hp, float hpMax) { bossHP = hp; bossHPMax = hpMax; };

	void SetCurrentDifficulty(Difficulty newDifficulty) { currentDifficulty_ = newDifficulty; };

	Difficulty GetCurrentDifficulty() { return currentDifficulty_; };

	float GetSpeedMagnification() { return speedMagnification_; };
	float GetDamageMagnification() { return damageMagnification_; };
private:
	void DebugUpdate();
private:
	Difficulty currentDifficulty_;

	float* playerHP;
	float playerHPMax;
	float* bossHP;
	float bossHPMax;

	float dopamineSpeed_;

	float speedMagnification_;
	float damageMagnification_;
};

