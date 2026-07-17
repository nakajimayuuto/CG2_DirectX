#pragma once
#include "Collision.h"
#include "Shape.h"
#include "CollisionConfig.h"
#include "Transform.h"
#include "../../ContactRecord.h"

enum class ColliderType {
	kSphere,
	kBox,
	kTorus,
};

class Collider {
public:
	Collider() = default;
	virtual ~Collider() = default;
	virtual void OnCollision([[maybe_unused]] Collider* other);

	void SetRadius(float radius) { colliderRadius_ = radius; };
	float GetRadius() const { return colliderRadius_; };
	void SetSize(const Vector3& size) { colliderSize_ = size; };

	void SetTransform(Transform transform) { transform_ = transform; };
	Transform GetTransform() { return transform_; };
	void SetParent(Transform* transform) { transform_.SetParent(transform); };

	void SetCollisionAttribute(uint32_t collisionAttribute) { collisionAttribute_ = collisionAttribute; };
	void SetCollisionMask(uint32_t collisionMask) { collisionMask_ = collisionMask; };

	uint32_t GetCollisionAttribute() { return collisionAttribute_; };
	uint32_t GetCollisionMask() { return collisionMask_; };

	void SetOnCollisionFunc(void (*func)([[maybe_unused]] Collider* other));

	virtual Vector3 GetWorldPosition() { return transform_.GetAffineMatrix().GetMatrixToTranslate(); };
	
	ColliderType GetColliderType() { return colliderType_; };

	OBB GetOBB() { CreateObbCollider(); return colliderObb_; };

	void SetColliderType(ColliderType type) { colliderType_ = type; };

	void DrawCollider();

	void SetDebugColor(const Vector4& color) { colliderColor_ = color; };

	float GetMinorRadius() const { return colliderMinorRadius_; };
	void SetMinorRadius(float radius) { colliderMinorRadius_ = radius; };

	void SetDamage(float damage) { damage_ = damage; };

	float GetDamage() { return damage_; };

	void SetDamageCoolTime(float damageCoolTime) { damageCoolTime_ = damageCoolTime; };

	float GetDamageCoolTime() { return damageCoolTime_; };

	bool GetActive() { return isColliderActive_; };
	void SetActive(bool isActive) {isColliderActive_ = isActive; };
protected:
	void CreateObbCollider();
protected:
	void (*pOnCollision_)([[maybe_unused]] Collider* other) = nullptr;

	Transform transform_ = Transform::GetInitialValue();

	float colliderRadius_ = 1.0f;
	float colliderMinorRadius_ = 1.0f;

	Vector3 colliderSize_ = {2.0f,2.0f,2.0f};

	OBB colliderObb_;

	Vector4 colliderColor_ = {1.0f,1.0f,1.0f,1.0f};

	ColliderType colliderType_ = ColliderType::kSphere;

	// 自分の属性(後々ここはstd::vectorにする)
	uint32_t collisionAttribute_ = 0xFFFFFFFF;

	// どの属性と当たるか(後々ここはstd::vectorにする)
	uint32_t collisionMask_ = 0xFFFFFFFF;

	float damage_ = 10.0f;

	float damageCoolTime_ = -1.0f;


	bool isColliderActive_ = true;
};

