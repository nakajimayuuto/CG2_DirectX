#include "TitleScene.h"

void TitleScene::Initialize() {
	TextureManager::GetInstance()->RegisterTexture("uvChecker", "Resource/uvChecker.png");
	ModelManager::GetInstance()->RegisterObj("plane", "Resource/Evaluation", "plane.obj");
	ModelManager::GetInstance()->RegisterObj("teapot", "Resource/Evaluation", "teapot.obj");
	ModelManager::GetInstance()->RegisterObj("bunny", "Resource/Evaluation", "bunny.obj");
	ModelManager::GetInstance()->RegisterObj("multiMesh", "Resource/Evaluation", "multiMesh.obj");
	ModelManager::GetInstance()->RegisterObj("multiMaterial", "Resource/Evaluation", "multiMaterial.obj");
	ModelManager::GetInstance()->RegisterObj("suzanne", "Resource/Evaluation", "suzanne.obj");
	CreateModelData({ -390.0f,-110.0f,0.0f }, DrawModelType::Sprite);
	CreateModelData({ 0.0f,0.0f,0.0f }, DrawModelType::Plane);
	Camera::GetInstance()->SetPosition({ 0.0f,0.0f,-10.0f });
	Camera::GetInstance()->ChangeCameraMode();
	currentNewModelType_ = DrawModelType::Plane;
}

void TitleScene::CreateModelData(const Vector3& position, DrawModelType type) {
	std::unique_ptr<DrawModelData> newModelData;
	newModelData = std::make_unique<DrawModelData>();
	newModelData->type = type;
	newModelData->isMultiMesh = false;
	CreateModel(newModelData.get());
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
}

void TitleScene::CreateModel(DrawModelData* data) {

	switch (data->type) {
	case DrawModelType::Plane:
		data->model.Initialize(ModelManager::GetInstance()->GetModelInfo("plane"));
		break;
	case DrawModelType::Sphere:
		data->model.Initialize(ModelManager::GetInstance()->GetModelInfo("plane"));
		data->sphereInfo = TextureManager::GetInstance()->GetTextureInfo("uvChecker");
		break;
	case DrawModelType::UtahTeapot:
		data->model.Initialize(ModelManager::GetInstance()->GetModelInfo("teapot"));
		break;
	case DrawModelType::StanfordBunny:
		data->model.Initialize(ModelManager::GetInstance()->GetModelInfo("bunny"));
		break;
	case DrawModelType::MultiMesh:
		data->model.Initialize(ModelManager::GetInstance()->GetModelInfo("multiMesh"));
		data->isMultiMesh = true;
		break;
	case DrawModelType::MultiMaterial:
		data->model.Initialize(ModelManager::GetInstance()->GetModelInfo("multiMaterial"));
		data->isMultiMesh = true;
		break;
	case DrawModelType::Suzzanne:
		data->model.Initialize(ModelManager::GetInstance()->GetModelInfo("suzanne"));
		break;
	case DrawModelType::Sprite:
		data->sprite.Initialize(TextureManager::GetInstance()->GetTextureInfo("uvChecker"));
		break;
	}
}

void TitleScene::Update() {
	bool enumSelect;
	if (ImGui::TreeNode("ModelType")) {
		if (ImGui::BeginListBox("LightingType")) {
			for (DrawModelType type : magic_enum::enum_values<DrawModelType>()) {
				enumSelect = (currentNewModelType_ == type);
				ImGui::Selectable(magic_enum::enum_name(type).data(), &enumSelect);

				if (enumSelect) {
					if (type != currentNewModelType_) {
						currentNewModelType_ = type;
					}
				}
			}
			ImGui::EndListBox();
		}
		ImGui::TreePop();
	}

	ImGui::Text(magic_enum::enum_name(currentNewModelType_).data());

	if (ImGui::Button("CreateModel")) {
		CreateModelData({ 0.0f,0.0f,0.0f }, currentNewModelType_);
	}

	for (auto& modelData : modelDatas_) {
		ImGui::PushID(modelData->number);
		if (ImGui::CollapsingHeader(magic_enum::enum_name(modelData->type).data())) {
			if (ImGui::Button("Delete")) {
				modelData->isDelete = true;
			}

			bool isVisible = modelData->model.GetIsVisible();
			ImGui::Checkbox("isVisible", &isVisible);
			modelData->model.SetIsVisible(isVisible);
			Vector3 imRotate;
			bool isSelect = false;
			Vector4 color;
			Transform uvTransform;

			if (isVisible) {
				if (modelData->type == DrawModelType::Sprite) {
					imRotate.z = Degree(modelData->transform.rotate.z);
					ImGui::DragFloat2("scale", reinterpret_cast<float*>(&modelData->transform.scale),0.1f, 0.0f, 100.0f);
					ImGui::DragFloat("rotate", &imRotate.z, 1.0f, -360.0f, 360.0f);
					ImGui::DragFloat2("translate", reinterpret_cast<float*>(&modelData->transform.translate), 10.0f, -1280.0f, 1280.0f);
					modelData->transform.rotate.z = Radian(imRotate.z);
					uvTransform.Initialize();
					uvTransform = modelData->sprite.GetUvTransform();
					imRotate.z = Degree(uvTransform.rotate.z);

					ImGui::DragFloat2("uvScale", reinterpret_cast<float*>(&uvTransform.scale), 0.1f, 0.0f, 10.0f);
					ImGui::DragFloat("uvRotate", &imRotate.z, 1.0f, -360.0f, 360.0f);
					ImGui::DragFloat2("uvTranslate", reinterpret_cast<float*>(&uvTransform.translate), 0.1f, -10.0f, 10.0f);
					uvTransform.rotate.z = Radian(imRotate.z);
					modelData->sprite.SetUvTransform(uvTransform);
				} else {
					imRotate = Degree(modelData->transform.rotate);
					ImGui::DragFloat3("scale", reinterpret_cast<float*>(&modelData->transform.scale), 0.1f, 0.0f, 10.0f);
					ImGui::DragFloat3("rotate", reinterpret_cast<float*>(&imRotate), 1.0f, -360.0f, 360.0f);
					ImGui::DragFloat3("translate", reinterpret_cast<float*>(&modelData->transform.translate), 0.1f, -10.0f, 10.0f);
					modelData->transform.rotate = Radian(imRotate);
					uvTransform.Initialize();
					if (modelData->isMultiMesh) {
						for (uint32_t i = 0; i < modelData->model.GetModelCountMax(); i++) {
							ImGui::PushID(i);
							color = modelData->model.GetColor(i);
							ImGui::ColorEdit4("color", reinterpret_cast<float*>(&color));
							modelData->model.SetColor(color, i);


							if (ImGui::BeginListBox("LightingType")) {
								for (LightingType lighting : magic_enum::enum_values<LightingType>()) {
									enumSelect = (lighting == modelData->model.GetLightingType(i));
									ImGui::Selectable(magic_enum::enum_name(lighting).data(), &enumSelect);

									if (enumSelect) {
										if (lighting != modelData->model.GetLightingType(i)) {
											modelData->model.SetLightingType(lighting, i);
										}
									}
								}
								ImGui::EndListBox();
							}

							uvTransform = modelData->model.GetUvTransform(i);
							imRotate.z = Degree(uvTransform.rotate.z);

							ImGui::DragFloat2("uvScale", reinterpret_cast<float*>(&uvTransform.scale), 0.1f, 0.0f, 10.0f);
							ImGui::DragFloat("uvRotate", &imRotate.z, 1.0f, -360.0f, 360.0f);
							ImGui::DragFloat2("uvTranslate", reinterpret_cast<float*>(&uvTransform.translate), 0.1f, -10.0f, 10.0f);
							uvTransform.rotate.z = Radian(imRotate.z);
							modelData->model.SetUvTransform(uvTransform, i);
							ImGui::PopID();
						}
					} else {
						color = modelData->model.GetColor();
						ImGui::ColorEdit4("color", reinterpret_cast<float*>(&color));
						modelData->model.SetColor(color);


						if (ImGui::BeginListBox("LightingType")) {
							for (LightingType lighting : magic_enum::enum_values<LightingType>()) {
								enumSelect = (lighting == modelData->model.GetLightingType());
								ImGui::Selectable(magic_enum::enum_name(lighting).data(), &enumSelect);

								if (enumSelect) {
									if (lighting != modelData->model.GetLightingType()) {
										modelData->model.SetLightingType(lighting);
									}
								}
							}
							ImGui::EndListBox();
						}

						uvTransform = modelData->model.GetUvTransform();
						imRotate.z = Degree(uvTransform.rotate.z);

						ImGui::DragFloat2("uvScale", reinterpret_cast<float*>(&uvTransform.scale), 0.1f, 0.0f, 10.0f);
						ImGui::DragFloat("uvRotate", &imRotate.z, 1.0f, -360.0f, 360.0f);
						ImGui::DragFloat2("uvTranslate", reinterpret_cast<float*>(&uvTransform.translate), 0.1f, -10.0f, 10.0f);
						uvTransform.rotate.z = Radian(imRotate.z);
						modelData->model.SetUvTransform(uvTransform);
					}
				}
			}
		}
		ImGui::PopID();
	}

	for (auto it = modelDatas_.begin(); it != modelDatas_.end(); ) {
		if ((*it)->isDelete) {
			it = modelDatas_.erase(it);
		} else {
			++it;
		}
	}

	Camera::GetInstance()->Update();
}

void TitleScene::Draw() {
	Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f, 1.0f, 0.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,100.0f }), { 1280.0f,720.0f }, "white_template", { 0.1f,0.25f,0.5f,1.0f });

	for (auto& modelData : modelDatas_) {
		if (modelData->type == DrawModelType::Sphere) {
			if (!modelData->model.GetIsVisible()) {
				continue;
			}
			Renderer::GetInstance()->SetLightingType(modelData->model.GetLightingType());
			Renderer::GetInstance()->DrawSphere(modelData->transform, modelData->sphereInfo, modelData->model.GetColor(),modelData->model.GetUvTransform());
			Renderer::GetInstance()->SetLightingType(LightingType::kHalfLambert);
		} else if(modelData->type == DrawModelType::Sprite){
			modelData->sprite.Draw(modelData->transform);
		} else {
			modelData->model.Draw(modelData->transform);
		}
	}
}