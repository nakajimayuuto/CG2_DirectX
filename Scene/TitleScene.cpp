#include "TitleScene.h"

void TitleScene::Initialize() {
	fakeWindows_.clear();
	modelDatas_.clear();
	CreateModelData({ -390.0f,-110.0f,0.0f }, DrawModelType::Sprite);
	CreateModelData({ 0.0f,0.0f,0.0f }, DrawModelType::Plane);
	CreateModelData({ -2.0f,0.0f,2.0f }, DrawModelType::Sphere);
	CreateModelData({ -6.0f,0.0f,4.0f }, DrawModelType::MultiMesh);
	CreateModelData({ 0.0f,0.0f,8.0f }, DrawModelType::MultiMaterial);
	CreateModelData({ 2.0f,0.0f,2.0f }, DrawModelType::UtahTeapot);
	CreateModelData({ 4.0f,0.0f,4.0f }, DrawModelType::StanfordBunny);
	CreateModelData({ 2.0f,0.0f,6.0f }, DrawModelType::Suzzanne);
	Camera::GetInstance()->SetPosition({ 0.0f,10.0f,-13.0f });
	Camera::GetInstance()->SetRotate({ Radian(30.0f),0.0f,0.0f });
	Camera::GetInstance()->DebugInitialize();
	//Camera::GetInstance()->ChangeCameraMode();
	currentNewModelType_ = DrawModelType::Plane;

	data = SoundManager::GetInstance()->GetSoundData("test");

	stencilMask_.Initialize();
	stencilMask_.SetBlendMode(BlendMode::kStencil);

	CreateFakeWindow();
	CreateFakeWindow();
	fakeWindows_[1]->SetTransform({ { 1.0f,1.0f },0.0f,{ 640.0f,360.0f } });
}

void TitleScene::CreateModelData(const Vector3& position, DrawModelType type) {
	std::unique_ptr<DrawModelData> newModelData;
	newModelData = std::make_unique<DrawModelData>();
	newModelData->type = type;
	newModelData->isMultiMesh = false;
	CreateModel(newModelData.get());
	newModelData->transform.Initialize();
	if (type != DrawModelType::Sphere && type != DrawModelType::Sprite) {
		newModelData->transform.rotate.y = Radian(180.0f);
	}
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
		data->model.Initialize(ModelManager::GetInstance()->GetModelInfo("plane"));
		data->sprite.Initialize(TextureManager::GetInstance()->GetTextureInfo("uvChecker"));
		break;
	}
}

void TitleScene::Update() {
	bool enumSelect;

	/**/
#ifdef _DEBUG

	ImGui::Begin("Window");
	if (ImGui::Button("Reset")) {
		SceneManager::GetInstance()->ReloadScene();
	}

	if (ImGui::Button("Finish")) {
		Environment::GetInstance()->GameFinished();
	}

	if (ImGui::CollapsingHeader("Models")) {
		if (ImGui::TreeNode("ModelType")) {
			if (ImGui::BeginListBox("ModelType")) {
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
				modelData->sprite.SetIsVisible(isVisible);
				Vector3 imRotate;
				bool isSelect = false;
				Vector4 color;
				Transform uvTransform;

				if (isVisible) {
					if (modelData->type == DrawModelType::Sprite) {
						imRotate.z = Degree(modelData->transform.rotate.z);
						ImGui::DragFloat2("scale", reinterpret_cast<float*>(&modelData->transform.scale), 0.1f, 0.0f, 100.0f);
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
	}

	if (ImGui::CollapsingHeader("Light")) {
		ImGui::ColorEdit4("LightColor", reinterpret_cast<float*>(&LightManager::GetInstance()->GetDirectionalLightData()->color));
		ImGui::SliderFloat3("LightDirection", reinterpret_cast<float*>(&LightManager::GetInstance()->GetDirectionalLightData()->direction), -1.0f, 1.0f);
		ImGui::DragFloat("LightIntensity", &LightManager::GetInstance()->GetDirectionalLightData()->intensity, 0.01f, 0.0f, 1.0f);
		LightManager::GetInstance()->GetDirectionalLightData()->direction = LightManager::GetInstance()->GetDirectionalLightData()->direction.Normalize();

	}

	if (ImGui::CollapsingHeader("Sound")) {
		if (ImGui::Button("start")) {
			SoundManager::GetInstance()->SoundPlay(data, 1.0f, 1.0f, kBGM, true, "test");
		}
		if (ImGui::Button("stop")) {
			SoundManager::GetInstance()->SoundStop("test");
		}
		if (ImGui::Button("se")) {
			SoundManager::GetInstance()->SoundPlay(data, 1.0f, 1.0f, kSoundEffect);
		}
	}


	for (auto it = fakeWindows_.begin(); it != fakeWindows_.end(); ) {
		if (!(*it)->GetIsActive()) {
			it = fakeWindows_.erase(it);
		} else {
			++it;
		}
	}

	if (ImGui::CollapsingHeader("FakeWindow")) {
		ImGui::Text("useFakeWindow");
		if (ImGui::Button(useFakeWindow_ ? "true" : "false")) {
			if (useFakeWindow_) {
				useFakeWindow_ = false;
			} else {
				useFakeWindow_ = true;
			}
		}


		if (useFakeWindow_) {
			if (ImGui::Button("Create")) {
				CreateFakeWindow();
			}

			Vector2 imVector2;
			Vector2 imScale2;
			float imRotateZ;
			uint32_t index = 0;;
			for (auto& window : fakeWindows_) {
				ImGui::PushID(index);
				if (ImGui::CollapsingHeader("Window")) {
					imVector2 = window->GetTransform().translate;
					imScale2 = window->GetTransform().scale;
					imRotateZ = Degree(window->GetTransform().rotate);
					ImGui::DragFloat2("scale", reinterpret_cast<float*>(&imScale2), 0.1f, 0.0f, 100.0f);
					ImGui::DragFloat("rotate", &imRotateZ, 1.0f, -360.0f, 360.0f);
					ImGui::DragFloat2("translate", reinterpret_cast<float*>(&imVector2), 10.0f, -(1920.0f / 2.0f) - (window->GetWindowSize().x / 2.0f), (1920.0f / 2.0f) + (window->GetWindowSize().x / 2.0f));
					window->SetTransform({ imScale2,Radian(imRotateZ),imVector2 });
					int type = static_cast<int>(window->GetType());
					//ImGui::SliderInt("Type", &type, 0, kWindowTypeCount - 1);
					//if (type != static_cast<int>(window->GetType())) {
					//	window->SetType(static_cast<WindowType>(type));
					//}

					if (ImGui::Button("Delete")) {
						window->SetIsActive(false);
					}
				}
				ImGui::PopID();
				index++;
			}
		}
	}


	ImGui::End();

#endif // _DEBUG

	for (auto it = modelDatas_.begin(); it != modelDatas_.end(); ) {
		if ((*it)->isDelete) {
			it = modelDatas_.erase(it);
		} else {
			++it;
		}
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_F3)) {
		Camera::GetInstance()->ChangeCameraMode();
	}

	Camera::GetInstance()->Update();
}

void TitleScene::Draw() {
	if (useFakeWindow_) {
		for (auto& window : fakeWindows_) {
			window->DrawBack();
		}
		for (auto& window : fakeWindows_) {
			window->DrawMask();
		}
	} else {
		stencilMask_.Draw(Transform::GetInitialValue({ 2000.0f, 1200.0f, 0.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,100.0f }));

	}

	Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 100.0f, 100.0f, 0.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,100.0f }), { 1920.0f,1080.0f }, "white_template", { 0.1f,0.25f,0.5f,1.0f });

	for (auto& modelData : modelDatas_) {
		if (modelData->type == DrawModelType::Sphere) {
			if (!modelData->model.GetIsVisible()) {
				continue;
			}
			Renderer::GetInstance()->SetLightingType(modelData->model.GetLightingType());
			Renderer::GetInstance()->DrawSphere(modelData->transform, modelData->sphereInfo, modelData->model.GetColor(), modelData->model.GetUvTransform());
			Renderer::GetInstance()->SetLightingType(LightingType::kHalfLambert);
		} else if (modelData->type == DrawModelType::Sprite) {
			modelData->sprite.Draw(modelData->transform);
		} else {
			modelData->model.Draw(modelData->transform);
		}
	}
}

void TitleScene::CreateFakeWindow() {
	std::unique_ptr<FakeWindow> newWindow;
	newWindow = std::make_unique<FakeWindow>();
	newWindow->Initialize();
	fakeWindows_.push_back(std::move(newWindow));
}