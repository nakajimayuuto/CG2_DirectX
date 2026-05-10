#pragma once
#include <random>
#include "Vector3.h"

class Random{
public:
	static Random* GetInstance();

	void Initialize();
	
	float RandomFloat(float min, float max);
	
	Vector3 RandomVector3(Vector3 min, Vector3 max);

	bool Probability(float percent);
private:
	std::mt19937_64 engine;
};
	