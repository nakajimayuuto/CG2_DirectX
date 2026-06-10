#include "GameScene.h"
#include "../Satlib.h"

void GameScene::Initialize() {
	// 使用するテクスチャの読み込み.
	ModelManager::GetInstance()->RegisterObj("skydome", "Resource/skydome", "skydome.obj");
	ModelManager::GetInstance()->RegisterObj("player", "Resource/player", "player.obj");
	ModelManager::GetInstance()->RegisterObj("plane", "Resource", "plane.obj");
	ModelManager::GetInstance()->RegisterObj("multiMaterial", "Resource", "multiMaterial.obj");

	// 使用するテクスチャの読み込み.
	TextureManager::GetInstance()->RegisterTexture("uvChecker", "Resource/uvChecker.png");
	TextureManager::GetInstance()->RegisterTexture("monsterBall", "Resource/monsterBall.png");
	TextureManager::GetInstance()->RegisterTexture("effect_triangle", "Resource/effect_triangle.png");
	TextureManager::GetInstance()->RegisterTexture("effect_circle", "Resource/circle.png");

	// カメラ位置の調整
	Camera::GetInstance()->SetPosition({ 0.0f,0.0f,-10.0f });
	skydome_ = new Skydome();
	skydome_->Initialize();


	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("player"));
	transform_.Initialize();

	sprite_.Initialize(TextureManager::GetInstance()->GetTextureInfo("uvChecker"));
	transformSprite_.Initialize();
	transformSprite_.translate.x = (sprite_.GetSize().x / 2.0f);
	transformSprite_.translate.y = (sprite_.GetSize().y / 2.0f);

	transform_.rotate.y = Radian(-70.0f);
	DirectionalLight::GetInstance()->GetDirectionalLightData()->direction = { 0.0f,0.0f,1.0f };
	GetWindowRect(GameSystem::GetInstance()->GetHWND(), &windowRect);

	emitterTransform_.Initialize();

	emitterCount_ = 3;

	emitterFrequency_ = 0.5f;

	fieldAABB_.min = { -1.0f,-1.0f,-1.0f };
	fieldAABB_.max = { 1.0f,1.0f,1.0f };
	fieldAcceleration_ = { 15.0f,0.0f,0.0f };

	billboardType_ = 3;

	useField_ = false;

	//ParticleManager::GetInstance()->Initialize();
	//
	//ParticleManager::GetInstance()->CreateNewParticles("testParticle", TextureManager::GetInstance()->GetTextureInfo("effect_circle"));
	//ParticleManager::GetInstance()->CreateNewEmitter("testEmitter","testParticle", emitterTransform_, emitterCount_, emitterFrequency_);
	//ParticleManager::GetInstance()->SetEmitterShape("testEmitter",EmitterShape::kBox);
	//ParticleManager::GetInstance()->CreateNewField("testField",fieldAABB_,fieldAcceleration_);
	//ParticleManager::GetInstance()->SetBillboardType(static_cast<BillboardType>(billboardType_));

	color_ = { 1.0f,1.0f,1.0f,1.0f };
	isParticleUpdate_ = false;
}

void GameScene::Update() {
	if (InputManager::GetInstance()->TriggerKey(DIK_R)) {
		//SetWindowLongW(GameSystem::GetInstance()->GetHWND(), GWL_STYLE, WS_OVERLAPPEDWINDOW);
		//
		//SetWindowPos(
		//	GameSystem::GetInstance()->GetHWND(),
		//	HWND_TOP,
		//	windowRect.left,
		//	windowRect.top,
		//	windowRect.right - windowRect.left,
		//	windowRect.bottom - windowRect.top,
		//	SWP_FRAMECHANGED | SWP_SHOWWINDOW
		//);
		SceneManager::GetInstance()->ReloadScene();

	}

	if (InputManager::GetInstance()->TriggerKey(DIK_F3)) {
		Camera::GetInstance()->ChangeCameraMode();
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_F11)) {
		if (Environment::GetInstance()->GetWindowMode() == kFullscreen) {
			Environment::GetInstance()->SetWindowMode(kWindowed);
		} else {
			Environment::GetInstance()->SetWindowMode(kFullscreen);
		}
	}

	Environment* environment = Environment::GetInstance();

	/*
	ImGui::Begin("Particles");

	ImGui::Text(std::format("DeltaTime : {}",DeltaTime::GetInstance()->GetDeltaTime()).c_str());

	ImGui::Checkbox("Update",&isParticleUpdate_);

	ImGui::SliderInt("Billboard",&billboardType_,0,4);

	ParticleManager::GetInstance()->SetBillboardType(static_cast<BillboardType>(billboardType_));

	ImGui::End();

	ImGui::Begin("Emitter");

	Vector3 imRotate = Degree(emitterTransform_.rotate);
	ImGui::SliderFloat3("scale",reinterpret_cast<float*>(&emitterTransform_.scale),0.0f,3.0f);
	ImGui::SliderFloat3("roate",reinterpret_cast<float*>(&imRotate),-360.0f,360.0f);
	ImGui::SliderFloat3("translate",reinterpret_cast<float*>(&emitterTransform_.translate),-10.0f,10.0f);
	emitterTransform_.rotate = Radian(imRotate);

	ImGui::SliderInt("count",reinterpret_cast<int*>(&emitterCount_),1,10);
	ImGui::SliderFloat("frequency", &emitterFrequency_, 0.0f, 2.0f);

	ParticleManager::GetInstance()->SetEmitterTransform("testEmitter", emitterTransform_);
	ParticleManager::GetInstance()->SetEmitterCount("testEmitter", emitterCount_);
	ParticleManager::GetInstance()->SetEmitterFrequency("testEmitter", emitterFrequency_);

	ImGui::End();

	ImGui::Begin("Field");

	ImGui::SliderFloat3("min",reinterpret_cast<float*>(&fieldAABB_.min), -10.0f, 10.0f);
	ImGui::SliderFloat3("max",reinterpret_cast<float*>(&fieldAABB_.max), -10.0f, 10.0f);
	ImGui::SliderFloat3("acceleration",reinterpret_cast<float*>(&fieldAcceleration_),-30.0f,30.0f);



	ParticleManager::GetInstance()->SetFieldAcceleration("testField",fieldAcceleration_);
	ParticleManager::GetInstance()->SetFieldArea("testField", fieldAABB_);

	ImGui::End();*/



	ImGui::Begin("ObjectMove");

	Vector3 imRotate = Degree(transform_.rotate);
	ImGui::DragFloat3("ModelScale", reinterpret_cast<float*>(&transform_.scale), 0.1f, 0.0f, 5.0f);
	ImGui::DragFloat3("ModelRotate", reinterpret_cast<float*>(&imRotate), 0.1f, -360.0f, 360.0f);
	ImGui::DragFloat3("ModelTranslate", reinterpret_cast<float*>(&transform_.translate), 0.1f, -20.0f, 20.0f);
	ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&color_));
	transform_.rotate = Radian(imRotate);

	ImGui::End();

	ImGui::Begin("DirectionalLight");
	Vector3 imDirection = DirectionalLight::GetInstance()->GetDirectionalLightData()->direction;
	ImGui::DragFloat3("LightDirection", reinterpret_cast<float*>(&imDirection), 0.1f, -1.0f, 1.0f);
	ImGui::DragFloat("LightIntensity", &DirectionalLight::GetInstance()->GetDirectionalLightData()->intensity, 0.1f, 0.0f, 1.0f);
	DirectionalLight::GetInstance()->GetDirectionalLightData()->direction = imDirection.Normalize();
	ImGui::End();

	ImGui::Begin("PointLight");
	ImGui::DragFloat3("LightPosition", reinterpret_cast<float*>(&PointLight::GetInstance()->GetPointLightData()->position), 0.1f, -10.0f, 10.0f);
	ImGui::DragFloat("LightIntensity", &PointLight::GetInstance()->GetPointLightData()->intensity, 0.1f, 0.0f, 1.0f);
	ImGui::End();

	ImGui::Begin("SpotLight");
	ImGui::DragFloat3("LightPosition", reinterpret_cast<float*>(&SpotLight::GetInstance()->GetSpotLightData()->position), 0.1f, -10.0f, 10.0f);
	ImGui::DragFloat("LightIntensity", &SpotLight::GetInstance()->GetSpotLightData()->intensity, 0.1f, 0.0f, 1.0f);
	ImGui::End();

	ImGui::Begin("Camera");
	Vector3 imPosition = Camera::GetInstance()->GetPosition();
	imRotate = Degree(Camera::GetInstance()->GetTransform().rotate);
	ImGui::DragFloat3("CameraRotate", reinterpret_cast<float*>(&imRotate), 0.1f, -360.0f, 360.0f);
	ImGui::DragFloat3("CameraPosition", reinterpret_cast<float*>(&imPosition), 0.1f, -5.0f, 5.0f);
	Camera::GetInstance()->SetPosition(imPosition);
	Camera::GetInstance()->SetRotate(Radian(imRotate));
	ImGui::End();

	if (isParticleUpdate_) {

		ParticleManager::GetInstance()->Update();
		//emitter_.Update();
		//particle.Update();
	}

	Camera::GetInstance()->Update();
}

void GameScene::Draw() {
	//	backGroundSprite_.Draw(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f ,0.0f,0.0f}, { backGroundSprite_.GetSize().x / 2.0f,backGroundSprite_.GetSize().y / 2.0f ,100.0f}));

		//skydome_->Draw();

	//model_.Draw(transform_);
	//Renderer::GetInstance()->DrawSphere(transform_, "monsterBall", color_);
	//Renderer::GetInstance()->DrawBox(transform_, "monsterBall", color_);
	//Renderer::GetInstance()->DrawModel(transform_,ModelManager::GetInstance()->GetModelInfo("multiMaterial"), {1.0f,1.0f,1.0f,1.0f});
	//Renderer::GetInstance()->DrawSphere(transform_, TextureManager::GetInstance()->GetTextureInfo("uvChecker"), {1.0f,1.0f,1.0f,1.0f});
	//model_.Draw(transform_);
	Renderer::GetInstance()->DrawSphere(transform_, "uvChecker", { 1.0f,1.0f,1.0f,1.0f });

	Renderer::GetInstance()->DrawLine({ 0.0f,0.0f,0.0f }, { 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->DrawLine({ 1.0f,1.0f,1.0f }, { 1.0f,-1.0f,1.0f }, { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->DrawLine({ 1.0f,-1.0f,1.0f }, { 0.0f,-1.0f,0.0f }, { 1.0f,1.0f,1.0f,1.0f });

	//Renderer::GetInstance()->DrawBox(transform_, TextureManager::GetInstance()->GetTextureInfo("uvChecker"), {1.0f,1.0f,1.0f,1.0f});
	//Renderer::GetInstance()->DrawModel(transform_,ModelManager::GetInstance()->GetModelInfo("player"), {1.0f,1.0f,1.0f,1.0f});
	//or (uint32_t i = 0; i < 10; i++) {
	//	transform_.translate.x = i;
	//	transformSprite_.translate.x = (sprite_.GetSize().x / 2.0f) + (i * 10.0f);
	//	transformSprite_.translate.y = (sprite_.GetSize().y / 2.0f);
	//	Renderer::GetInstance()->DrawModel(transform_, &model_);
	//	Renderer::GetInstance()->DrawSprite(transformSprite_, sprite_);
	//
	//
	//ransform_.Initialize();
	//ransformSprite_.Initialize();

	//particle.Draw();
	//
	//emitter_.DebugDraw();

	//ParticleManager::GetInstance()->Draw();

	//sprite_.Draw(transformSprite_);

	//frameSpriteLeft_.Draw({ {1.0f,1.0f},0.0f, {-frameSpriteLeft_.GetSize().x / 2.0f,frameSpriteLeft_.GetSize().y / 2.0f} });
	//frameSpriteRight_.Draw({ {1.0f,1.0f},0.0f, {Environment::GetInstance()->GetWindowSize().width + (frameSpriteRight_.GetSize().x / 2.0f),frameSpriteRight_.GetSize().y / 2.0f } });
}
