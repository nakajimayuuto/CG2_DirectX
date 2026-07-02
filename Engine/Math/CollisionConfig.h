#pragma once
#include <cstdint>

enum class CollisionTypeIdDef : uint32_t {
	kDefalut,
	kPlayer,
	kPlayerWeapon,
	kEnemy,
};

const uint32_t kCollisionAttributePlayer = 0b1;
const uint32_t kCollisionAttributeEnemy = 0b1 << 1;