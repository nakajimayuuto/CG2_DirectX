#include "TitleScene.h"

void TitleScene::Initialize() {
	ModelManager::GetInstance()->RegisterObj("skydome", "Resource/skydome", "skydome.obj");
	ModelManager::GetInstance()->RegisterObj("player", "Resource/player_hovering_mode", "player.obj");
	CreateModel({0.0f,0.0f,0.0f});
	Camera::GetInstance()->SetPosition({ 0.0f,0.0f,-10.0f });

	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize();
}

void TitleScene::CreateModel(const Vector3& position) {
	std::unique_ptr<DrawModelData> newModelData;
	newModelData = std::make_unique<DrawModelData>();
	newModelData->model.Initialize(ModelManager::GetInstance()->GetModelInfo("block_template"));
	newModelData->transform.Initialize();
	newModelData->transform.translate = position;
	newModelData->lightingType[0] = { "None",false };
	newModelData->lightingType[1] = { "HalfLambert",true };
	newModelData->lightingType[2] = { "Lambert",false };

	newModelData->textureType[0] = { "uvChecker",true };
	newModelData->textureType[1] = { "monsterBall",false };
	newModelData->textureType[2] = { "effect_triangle",false };
	newModelData->number = modelIndex_;
	newModelData->isDelete = false;;
	modelDatas_.push_back(std::move(newModelData));

	modelIndex_++;
};

void TitleScene::Update() {
	if (ImGui::Button("CreateModel")) {
		CreateModel({ 0.0f,0.0f,0.0f });
	}

	for (auto& modelData : modelDatas_) {
		ImGui::PushID(modelData->number);
		if (ImGui::CollapsingHeader("Model")) {
			if (ImGui::Button("Delete")) {
				modelData->isDelete = true;
			}

			bool isVisible = modelData->model.GetIsVisible();
			ImGui::Checkbox("isVisible", &isVisible);
			modelData->model.SetIsVisible(isVisible);

			if (isVisible) {

				Vector4 color = modelData->model.GetColor();
				ImGui::ColorEdit4("color", reinterpret_cast<float*>(&color));
				modelData->model.SetColor(color);

				Vector3 imRotate = Degree(modelData->transform.rotate);
				ImGui::DragFloat3("scale", reinterpret_cast<float*>(&modelData->transform.scale), 0.1f, 0.0f, 10.0f);
				ImGui::DragFloat3("rotate", reinterpret_cast<float*>(&imRotate), 1.0f, -360.0f, 360.0f);
				ImGui::DragFloat3("translate", reinterpret_cast<float*>(&modelData->transform.translate), 0.1f, -10.0f, 10.0f);
				modelData->transform.rotate = Radian(imRotate);
				bool isSelect = false;
				//if (ImGui::BeginListBox("Texture")) {
				//	for (int i = 0; i < 3; i++) {
				//		isSelect = modelData->textureType[i].isSelect;
				//		ImGui::Selectable(modelData->textureType[i].name, &modelData->textureType[i].isSelect);
				//
				//		if (!modelData->textureType[i].isSelect) {
				//			modelData->textureType[i].isSelect = isSelect;
				//			continue;
				//		}
				//
				//		for (int j = 0; j < 3; j++) {
				//			if (i == j) {
				//				continue;
				//			}
				//
				//			modelData->textureType[j].isSelect = false;
				//		}
				//	}
				//	ImGui::EndListBox();
				//}

				//for (int i = 0; i < 3; i++) {
				//	if (modelData->textureType[i].isSelect) {
				//		modelData->model.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo(modelData->textureType[i].name));
				//	}
				//}

				if (ImGui::BeginListBox("LightingType")) {
					for (int i = 0; i < 3; i++) {
						isSelect = modelData->lightingType[i].isSelect;
						ImGui::Selectable(modelData->lightingType[i].name, &modelData->lightingType[i].isSelect);

						if (!modelData->lightingType[i].isSelect) {
							modelData->lightingType[i].isSelect = isSelect;
							continue;
						}

						for (int j = 0; j < 3; j++) {
							if (i == j) {
								continue;
							}

							modelData->lightingType[j].isSelect = false;
						}
					}
					ImGui::EndListBox();
				}

				for (int i = 0; i < 3; i++) {
					if (modelData->lightingType[i].isSelect) {
						modelData->model.SetLightingType(static_cast<LightingType>(i));
					}
				}

			}
		}
		ImGui::PopID();
	}

	Camera::GetInstance()->Update();
}

void TitleScene::Draw() {
	//skydome_->Draw();
	Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f, 1.0f, 0.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,100.0f }), {1280.0f,720.0f}, "white_template", { 0.1f,0.25f,0.5f,1.0f });
	
	for (auto& modelData : modelDatas_) {
		Renderer::GetInstance()->DrawModel(modelData->transform,&modelData->model,false);
	}
		//Renderer::GetInstance()->DrawModel(modelDatas_[0]->transform,&modelDatas_[0]->model,false);
}