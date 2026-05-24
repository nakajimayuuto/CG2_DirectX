#include "GameScene.h"
#include "../Satlib.h"

void GameScene::Initialize() {
	// 使用するテクスチャの読み込み.
	ModelManager::GetInstance()->RegisterObj("skydome", "Resource/skydome", "skydome.obj");

	// 使用するテクスチャの読み込み.
	TextureManager::GetInstance()->RegisterTexture("uvChecker", "Resource/uvChecker.png");
	TextureManager::GetInstance()->RegisterTexture("monsterBall", "Resource/monsterBall.png");
	TextureManager::GetInstance()->RegisterTexture("effect_triangle", "Resource/effect_triangle.png");

	// カメラ位置の調整
	Camera::GetInstance()->SetPosition({ 0.0f,0.0f,-10.0f });

	// デバッグ用の三角形の生成.
	CreateTriangle({ -1.0f,0.0f,0.0f });
	CreateTriangle({ 1.0f,0.0f,0.0f });

	// 演出用データの初期化.
	parentTransform_.Initialize();
	parentTransformMini_.Initialize();

	Vector3 vertexPosition[3] = { { 0.0f,kTriangleSize,0.0f }, { kTriangleSize * -1.04f,-kTriangleSize,kTriangleSize * -0.6f }, { kTriangleSize * 1.04f,-kTriangleSize,kTriangleSize * -0.6f } };

	for (uint32_t i = 0; i < kEffectTriangle; i++) {
		effectTriangleData_[i].transform.Initialize();
		effectTriangleData_[i].transform.SetParent(&parentTransform_);
		effectTriangleData_[i].model.Initialize(TextureManager::GetInstance()->GetTextureInfo("effect_triangle"));
		effectTriangleData_[i].model.SetVertexPosition(vertexPosition[0], vertexPosition[1], vertexPosition[2]);
		effectTriangleData_[i].transform.rotate.y = static_cast<float>(Radian(120.0f) * i);
		effectTriangleData_[i].model.SetBlendMode(BlendMode::kNormalCullNone);
	}

	for (uint32_t i = kEffectTriangle; i < kEffectTriangle + kEffectTriangleMini; i++) {
		effectTriangleData_[i].transform.Initialize();
		effectTriangleData_[i].transform.SetParent(&parentTransformMini_);
		effectTriangleData_[i].model.Initialize(TextureManager::GetInstance()->GetTextureInfo("effect_triangle"));
		effectTriangleData_[i].model.SetVertexPosition(vertexPosition[0] / 4.0f, vertexPosition[1] / 4.0f, vertexPosition[2] / 4.0f);
		effectTriangleData_[i].transform.rotate.y = static_cast<float>(Radian(120.0f) * i);
		effectTriangleData_[i].model.SetBlendMode(BlendMode::kNormalCullNone);
	}

	effectTriangleData_[kEffectTriangle + kEffectTriangleMini].transform.Initialize();
	effectTriangleData_[kEffectTriangle + kEffectTriangleMini].transform.SetParent(&parentTransform_);
	effectTriangleData_[kEffectTriangle + kEffectTriangleMini].model.Initialize(TextureManager::GetInstance()->GetTextureInfo("effect_triangle"));
	effectTriangleData_[kEffectTriangle + kEffectTriangleMini].model.SetVertexPosition({ 0.0f,-kTriangleSize,kTriangleSize * 1.2f }, { kTriangleSize * -1.05f,-kTriangleSize,kTriangleSize * -0.6f }, { kTriangleSize * 1.05f,-kTriangleSize,kTriangleSize * -0.6f });
	effectTriangleData_[kEffectTriangle + kEffectTriangleMini].model.SetBlendMode(BlendMode::kNormalCullNone);

	deltaTime_ = DeltaTime::GetInstance();

	skydome_ = new Skydome();
	skydome_->Initialize();

	DirectionalLight::GetInstance()->GetDirectionalLightData()->direction = { 0.0f,0.0f,-1.0f };
}

void GameScene::Update() {
	DeleteTriangle();

	ImGui::Begin("Triangles");
	ImGui::Text("AnimationMode : %s", state_ != State::kTriangleDebug ? "true" : "false");
	if (ImGui::Button("Change")) {
		switch (state_) {
		case GameScene::State::kTriangleDebug:
			EffectInitialize();
			state_ = State::kTriangleEffect;
			break;
		case GameScene::State::kTriangleEffect:
		case GameScene::State::kTriangleEffectAnimation:
			state_ = State::kTriangleDebug;
			break;
		default:
			break;
		}
	}

	if (state_ == State::kTriangleDebug) {
		if (ImGui::Button("CreateTriangle")) {
			CreateTriangle({ 0.0f,0.0f,0.0f });
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
	} else {
		if (ImGui::Button("PlayAnimation")) {
			if (state_ == State::kTriangleEffect){ 
			state_ = State::kTriangleEffectAnimation;
			EffectAnimationInitialize();
			}
		}
	}
	ImGui::End();

	switch (state_) {
	case GameScene::State::kTriangleEffect:
		EffectUpdate();
		break;
	case GameScene::State::kTriangleEffectAnimation:
		EffectAnimationUpdate();
		break;
	}

	Camera::GetInstance()->Update();
}

void GameScene::EffectUpdate() {
	if (InputManager::GetInstance()->TriggerKey(DIK_SPACE)) {
		state_ = State::kTriangleEffectAnimation;
		EffectAnimationInitialize();
	}

	parentTransform_.rotate.y += Radian(kRotateSpeed) * deltaTime_->GetDeltaTime();
	parentTransformMini_.rotate.y -= Radian(kRotateSpeed) * deltaTime_->GetDeltaTime();

	if (parentTransform_.rotate.y > Radian(360.0f)) {
		parentTransform_.rotate.y -= Radian(360.0f);
	}

	if (parentTransformMini_.rotate.y < Radian(0.0f)) {
		parentTransformMini_.rotate.y += Radian(360.0f);
	}
}

void GameScene::EffectAnimationInitialize() {
	phase_ = AnimationPhase::kOpen;
	animationTimer_ = 0.0f;
}

void GameScene::EffectAnimationUpdate() {
	animationTimer_ += deltaTime_->GetDeltaTime();

	float rotateSpeed = 0.0f;
	float trianglePositionZ = 0.0f;
	Matrix4x4 triangleMatrix = Matrix4x4::Identity();

	switch (phase_) {
	case GameScene::AnimationPhase::kOpen:
		rotateSpeed = Easing(kRotateSpeed, kRotateActionSpeed, animationTimer_, kOpenAnimationMax, EaseType::kEaseOut);
		trianglePositionZ = Easing(kTrianglePositionZ, kTriangleActionPositionZ, animationTimer_, kOpenAnimationMax, EaseType::kEaseOut);
		parentTransform_.rotate.y += Radian(rotateSpeed) * deltaTime_->GetDeltaTime();
		parentTransformMini_.rotate.y -= Radian(rotateSpeed) * deltaTime_->GetDeltaTime();


		for (uint32_t i = 0; i < kEffectTriangle; i++) {
			triangleMatrix = Matrix4x4::MakeRotateYMatrix(Radian(static_cast<float>(120.0f * i)));
			triangleMatrix = Matrix4x4::MakeTranslateMatrix({ 0.0f,0.0f,trianglePositionZ }) * triangleMatrix;
			effectTriangleData_[i].transform.translate = triangleMatrix.GetMatrixToTranslate();
		}

		if (animationTimer_ >= kOpenAnimationMax) {
			animationTimer_ = 0.0f;
			phase_ = AnimationPhase::kStay;
		}
		break;
	case GameScene::AnimationPhase::kStay:
		parentTransform_.rotate.y += Radian(kRotateActionSpeed) * deltaTime_->GetDeltaTime();
		parentTransformMini_.rotate.y -= Radian(kRotateActionSpeed) * deltaTime_->GetDeltaTime();

		if (animationTimer_ >= kStayAnimationMax) {
			animationTimer_ = 0.0f;
			phase_ = AnimationPhase::kClose;
		}
		break;
	case GameScene::AnimationPhase::kClose:
		rotateSpeed = Easing(kRotateActionSpeed, kRotateSpeed, animationTimer_, kCloseAnimationMax, EaseType::kEaseIn);
		trianglePositionZ = Easing(kTriangleActionPositionZ, kTrianglePositionZ, animationTimer_, kCloseAnimationMax, EaseType::kEaseIn);
		parentTransform_.rotate.y += Radian(rotateSpeed) * deltaTime_->GetDeltaTime();
		parentTransformMini_.rotate.y -= Radian(rotateSpeed) * deltaTime_->GetDeltaTime();

		for (uint32_t i = 0; i < kEffectTriangle; i++) {
			triangleMatrix = Matrix4x4::MakeRotateYMatrix(Radian(static_cast<float>(120.0f * i)));
			triangleMatrix = Matrix4x4::MakeTranslateMatrix({ 0.0f,0.0f,trianglePositionZ }) * triangleMatrix;
			effectTriangleData_[i].transform.translate = triangleMatrix.GetMatrixToTranslate();
		}

		if (animationTimer_ >= kCloseAnimationMax) {
			animationTimer_ = 0.0f;
			state_ = State::kTriangleEffect;
		}
		break;
	}
}


void GameScene::Draw() {

	skydome_->Draw();

	switch (state_) {
	case GameScene::State::kTriangleDebug:
		for (TriangleData& triangleData : triangleDatas_) {
			triangleData.model.Draw(triangleData.transform);
		}
		break;
	case GameScene::State::kTriangleEffect:
	case GameScene::State::kTriangleEffectAnimation:
		for (uint32_t i = 0; i < kEffectTriangle + kEffectTriangleMini + 1; i++) {
			effectTriangleData_[i].model.Draw(effectTriangleData_[i].transform);
		}
		break;
	}
}

void GameScene::EffectInitialize() {
	parentTransform_.Initialize();

	for (uint32_t i = 0; i < kEffectTriangle; i++) {
		effectTriangleData_[i].transform.translate.z = kTrianglePositionZ;
	}
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
	newTriangleData.textureType[2] = { "effect_triangle",false };
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
