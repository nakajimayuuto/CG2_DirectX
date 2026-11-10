#pragma once
#include "Collider.h"
#include <string>

#define InspectorNameAutoSet inspectorName = static_cast<std::string>(NAMEOF_SHORT_TYPE_EXPR(*this))

class BaseEntityTransform {
public:
	void SetInspectorName(std::string name) { inspectorName = name; };
protected:
	void TransformInspectorUpdate();

	virtual void Initialize() = 0;

	virtual void Update() = 0;

	virtual void Draw() = 0;
protected:
	std::string inspectorName = "noname";
};

class BaseEntityTransform : public BaseEntity{
public:
	void InspectorUpdate() override;
protected:
	float currentHP_ = 100.0f;
	float maxHP_ = 100.0f;
};

class BaseEntity{
public:
	void SetInspectorName(std::string name) { inspectorName = name; };
	virtual void InspectorUpdate() = 0;
protected:
	std::string inspectorName = "noname";
};