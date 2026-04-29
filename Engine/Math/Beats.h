#pragma once
class Beats{
private:
	float deltaTimeBeats_;

	float beat1_;
	bool beat1Judge_;
	float beat1per16_;
	bool beat1per16Judge_;
	float beat1Timer_;
	float beat1per16Timer_;
public:
	void Initialize();

	void Update();

	void SetDeltaTimePer1Beat(float beat) { deltaTimeBeats_ = beat; };

	float GetBeat1() { return beat1_; };
	float GetBeat1per16() { return beat1per16_; };
	bool GetBeat1Judge() { return beat1Judge_; }
	bool GetBeat1per16Judge() { return beat1per16Judge_; }
};

