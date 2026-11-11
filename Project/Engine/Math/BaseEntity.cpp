#include "BaseEntity.h"
#include "ImGuiManager.h"
#include "MyMath.h"
#ifdef USE_IMGUI
#include "../../externals/imgui/imgui.h"
#endif // USE_IMGUI

void BaseEntityTransform::InspectorUpdate(){
	ImGuiManager::GetInstance()->Begin(inspectorName);

#ifdef USE_IMGUI
	ImGui::DragFloat3("scale", reinterpret_cast<float*>(&transform_.scale), 0.1f, 0.0f, 100000.0f);
	Vector3 rotate = Degree(transform_.rotate);
	ImGui::DragFloat3("rotate", reinterpret_cast<float*>(&rotate), 1.0f, -360000.0f, 360000.0f);
	transform_.rotate = Radian(rotate);
	ImGui::DragFloat3("translate", reinterpret_cast<float*>(&transform_.translate), 0.1f, -100000.0f, 100000.0f);
#endif // USE_IMGUI

	ImGuiManager::GetInstance()->End();
}

void BaseEntityCharactor::InspectorUpdate() {
	ImGuiManager::GetInstance()->Begin(inspectorName);

#ifdef USE_IMGUI
	ImGui::DragFloat3("scale", reinterpret_cast<float*>(&transform_.scale), 0.1f, 0.0f, 100000.0f);
	Vector3 rotate = Degree(transform_.rotate);
	ImGui::DragFloat3("rotate", reinterpret_cast<float*>(&rotate), 1.0f, -360000.0f, 360000.0f);
	transform_.rotate = Radian(rotate);
	ImGui::DragFloat3("translate", reinterpret_cast<float*>(&transform_.translate), 0.1f, -100000.0f, 100000.0f);
	ImGui::DragFloat("HP", &currentHP_, 1.0f, 0.0f, maxHP_);
	ImGui::DragFloat("MaxHP", &maxHP_, 1.0f, 0.0f, 10000000.0f);
#endif // USE_IMGUI

	ImGuiManager::GetInstance()->End();

}
