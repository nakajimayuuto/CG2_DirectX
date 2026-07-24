#include "TitleScene.h"

void TitleScene::Initialize() {
}

void TitleScene::CreateModel(const Vector3& position) {

};

void TitleScene::Update() {
	if (ImGui::Button("CreateTriangle")) {
		CreateModel({ 0.0f,0.0f,0.0f });
	}

	for (auto& modelData : modelDatas_) {
		ImGui::PushID(modelData->number_);
		if (ImGui::CollapsingHeader("Triangle")) {
			if (ImGui::Button("Delete")) {
				mmodelData->isDelete = true;
			}

			bool isVisible = triangleData.model.GetIsVisible();
			ImGui::Checkbox("isVisible", &isVisible);
			triangleData.model.SetIsVisible(isVisible);

			if (isVisible) {

				Vector4 color = triangleData.model.GetColor();
				ImGui::ColorEdit4("color", reinterpret_cast<float*>(&color));
				triangleData.model.SetColor(color);

				Vector3 imRotate = Degree(triangleData.transform.rotate);
				ImGui::DragFloat3("scale", reinterpret_cast<float*>(&triangleData.transform.scale), 0.1f, 0.0f, 10.0f);
				ImGui::DragFloat3("rotate", reinterpret_cast<float*>(&imRotate), 1.0f, -360.0f, 360.0f);
				ImGui::DragFloat3("translate", reinterpret_cast<float*>(&triangleData.transform.translate), 0.1f, -10.0f, 10.0f);
				triangleData.transform.rotate = Radian(imRotate);
				bool isSelect = false;
				if (ImGui::BeginListBox("Texture")) {
					for (int i = 0; i < 3; i++) {
						isSelect = triangleData.textureType[i].isSelect;
						ImGui::Selectable(triangleData.textureType[i].name, &triangleData.textureType[i].isSelect);

						if (!triangleData.textureType[i].isSelect) {
							triangleData.textureType[i].isSelect = isSelect;
							continue;
						}

						for (int j = 0; j < 3; j++) {
							if (i == j) {
								continue;
							}

							triangleData.textureType[j].isSelect = false;
						}
					}
					ImGui::EndListBox();
				}

				for (int i = 0; i < 3; i++) {
					if (triangleData.textureType[i].isSelect) {
						triangleData.model.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo(triangleData.textureType[i].name));
					}
				}

				if (ImGui::BeginListBox("LightingType")) {
					for (int i = 0; i < 3; i++) {
						isSelect = triangleData.lightingType[i].isSelect;
						ImGui::Selectable(triangleData.lightingType[i].name, &triangleData.lightingType[i].isSelect);

						if (!triangleData.lightingType[i].isSelect) {
							triangleData.lightingType[i].isSelect = isSelect;
							continue;
						}

						for (int j = 0; j < 3; j++) {
							if (i == j) {
								continue;
							}

							triangleData.lightingType[j].isSelect = false;
						}
					}
					ImGui::EndListBox();
				}

				for (int i = 0; i < 3; i++) {
					if (triangleData.lightingType[i].isSelect) {
						triangleData.model.SetLightingType(static_cast<Renderer::LightingType>(i));
					}
				}

			}
		}
		ImGui::PopID();
	}
}

void TitleScene::Draw() {
	Renderer::GetInstance()->DrawSprite(Transform2D({ 1.1f,1.1f }, 0.0f, { 1.0f,1.0f }).GetTransformValue(), { 1280.0f,720.0f }, "white_template", { 0.1f,0.25f,0.5f,1.0f });
}