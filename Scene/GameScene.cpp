#include "GameScene.h"
#include "../Satlib.h"

void GameScene::Initialize() {
	ModelManager::GetInstance()->RegisterObj("player", "Resource/player", "player.obj");
	ModelManager::GetInstance()->RegisterObj("block", "Resource/block", "block.obj");
	ModelManager::GetInstance()->RegisterObj("plane", "Resource", "plane.obj");
	ModelManager::GetInstance()->RegisterObj("fence", "Resource/fence", "fence.obj");
	//testModel_.Initialize(ModelManager::GetInstance()->GetModelInfo("fence"));
	//testTransform_.Initialize();
	//testTransform_.translate = { 0.0f,0.0f,0.0f };
	//testTransform_.rotate.y = Radian(-180.0f);
	//testTransform_.rotate.x = Radian(30.0f);

	TextureManager::GetInstance()->RegisterTexture("uvChecker", "Resource/uvChecker.png");
	TextureManager::GetInstance()->RegisterTexture("monsterBall", "Resource/monsterBall.png");

	model_.Initialize(TextureManager::GetInstance()->GetTextureInfo("uvChecker"));
	transform_.Initialize();
	CreateTriangle({ -1.0f,0.0f,0.0f });
	CreateTriangle({ 1.0f,0.0f,0.0f });

	Camera::GetInstance()->SetPosition({ 0.0f,0.0f,-10.0f });
}

void GameScene::Update() {
	DeleteTriangle();

	ImGui::ShowDemoWindow();

	ImGui::Begin("Triangles");
	if (ImGui::Button("CreateTriangle")) {
		CreateTriangle({0.0f,0.0f,0.0f});
	}

	for (TriangleData& triangleData : triangleDatas_) {
		ImGui::PushID(triangleData.number);
		if (ImGui::CollapsingHeader("Triangle")) {
			if (ImGui::Button("Delete")) {
				triangleData.isDelete = true;
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
	ImGui::End();

	Camera::GetInstance()->Update();
}

void GameScene::Draw() {

	//	testModel_.Draw(testTransform_);

	for (TriangleData& triangleData : triangleDatas_) {
		triangleData.model.Draw(triangleData.transform);
	}

	//model_.Draw(transform_);
}

void GameScene::CreateTriangle(const Vector3& position) {
	TriangleData newTriangleData;
	newTriangleData.model.Initialize(TextureManager::GetInstance()->GetTextureInfo("uvChecker"));
	newTriangleData.transform.Initialize();
	newTriangleData.transform.translate = position;
	newTriangleData.lightingType[0] = { "None",false };
	newTriangleData.lightingType[1] = { "HalfLambert",true };
	newTriangleData.lightingType[2] = { "Lambert",false };

	newTriangleData.textureType[0] = { "uvChecker",true };
	newTriangleData.textureType[1] = { "monsterBall",false };
	newTriangleData.textureType[2] = { "white_template",false };
	newTriangleData.number = triangleIndex_;
	newTriangleData.isDelete = false;;
	triangleDatas_.push_back(newTriangleData);

	triangleIndex_++;
}

void GameScene::DeleteTriangle() {
	triangleDatas_.remove_if([](TriangleData triangle) {
		if (triangle.isDelete) {
			return true;
		}
		return false;
		});
}
