#pragma once
#include "Collider.h"
#include "Transform.h"
#include "nameof.hpp"
#include <string>

// inspectorName を自分のクラス名にする(Initializeなどで呼ぶ).
#define InspectorNameAutoSet inspectorName = static_cast<std::string>(NAMEOF_SHORT_TYPE_EXPR(*this))

class BaseEntity{
public:
	virtual ~BaseEntity() = default;

	virtual void Initialize() = 0;

	virtual void Update() = 0;

	virtual void Draw() = 0;

	void SetInspectorName(std::string name) { inspectorName = name; };
	const std::string& GetInspectorName() const { return inspectorName; };

	// Inspector(ImGuiManager::Begin(inspectorName)のタブ)に値を表示する.
	virtual void InspectorUpdate() = 0;
protected:
	std::string inspectorName = "noname";
};

class BaseEntityTransform : public BaseEntity {
public:
	void InspectorUpdate() override;

	Transform& GetEntityTransform() { return transform_; };
protected:
	Transform transform_ = Transform::GetInitialValue();
};

class BaseEntityCharactor : public BaseEntity , public Collider{
public:
	void InspectorUpdate() override;
protected:
	float currentHP_ = 100.0f;
	float maxHP_ = 100.0f;
};
