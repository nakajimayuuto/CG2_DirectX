#include "BaseEntity.h"
#include "ImGuiManager.h"
#include "MyMath.h"

void BaseEntityTransform::TransformInspectorUpdate(){
	ImGuiManager::GetInstance()->Begin(inspectorName);

	ImGui::DragFloat3("scale",reinterpret_cast<float*>(&transform_.scale),0.1f,0.0f,100000.0f);
	Vector3 rotate = Degree(transform_.rotate);
	ImGui::DragFloat3("rotate",reinterpret_cast<float*>(&rotate),1.0f,-360000.0f,360000.0f);
	transform_.rotate = Radian(rotate);
	ImGui::DragFloat3("translate",reinterpret_cast<float*>(&transform_.translate),0.1f,-100000.0f,100000.0f);

	ImGuiManager::GetInstance()->End();
}

void BaseEntity::InspectorUpdate(){
	TransformInspectorUpdate();
	ImGuiManager::GetInstance()->Begin(inspectorName);

	ImGui::DragFloat("HP", &currentHP_, 1.0f, 0.0f,maxHP_);
	ImGui::DragFloat("MaxHP", &maxHP_, 1.0f, 0.0f, 10000000.0f);

	ImGuiManager::GetInstance()->End();

}
