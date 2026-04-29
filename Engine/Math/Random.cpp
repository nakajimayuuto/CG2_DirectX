#include "Random.h"
#include <algorithm>

Random* Random::GetInstance() {
	Random instance;
	return &instance;
}

void Random::Initialize() {
	std::random_device seed_gen;
	std::uint32_t seed = seed_gen();
	engine.seed(seed);
}

Vector3 Random::RandomVector3(Vector3 min, Vector3 max) {
	Vector3 random;

	random.x = RandomFloat(min.x, max.x);
	random.y = RandomFloat(min.y, max.y);
	random.z = RandomFloat(min.z, max.z);

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
