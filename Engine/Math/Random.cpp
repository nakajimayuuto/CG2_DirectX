#include "Random.h"
#include <algorithm>

Random* Random::GetInstance() {
	static Random instance;
	return &instance;
}

void Random::Initialize() {
	std::random_device seedGen;
	engine.seed(seedGen());
}

Vector3 Random::RandomVector3(Vector3 min, Vector3 max) {
	Vector3 random;

	random.x = RandomFloat(min.x, max.x);
	random.y = RandomFloat(min.y, max.y);
	random.z = RandomFloat(min.z, max.z);

	return random;
}

Vector3 Random::RandomCircleVector3(Vector3 radius){
	Vector3 random;

	random = RandomVector3(-(radius / 2.0f), radius / 2.0f);

	while (random.Normalize().x > radius.x || random.Normalize().y > radius.y || random.Normalize().z > radius.z) {
		random = RandomVector3(-(radius / 2.0f), radius / 2.0f);
	}

	return random;
}

bool Random::Probability(float percent) {
	float probability = RandomFloat(0.0f, 100.0f);
	if (probability < percent) {
		return true;
	}

	return false;
}

float Random::RandomFloat(float min, float max) {
	if (min > max) {
		std::swap(min, max);
	}

	std::uniform_real_distribution<float> dist(min, max);
	float random = dist(engine);
	return random;
}
