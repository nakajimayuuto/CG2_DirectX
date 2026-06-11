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
	LightManager::GetInstance()->GetDirectionalLightData()->direction = { 0.0f,0.0f,1.0f };
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

	LightManager::GetInstance()->CreatePointLight("testPoint");
	LightManager::GetInstance()->CreatePointLight("testPoint2");
	LightManager::GetInstance()->CreateSpotLight("testSpot");

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

	ImGui::Begin("ObjectMove");

	Vector3 imRotate = Degree(transform_.rotate);
	ImGui::DragFloat3("ModelScale", reinterpret_cast<float*>(&transform_.scale), 0.1f, 0.0f, 5.0f);
	ImGui::DragFloat3("ModelRotate", reinterpret_cast<float*>(&imRotate), 0.1f, -360.0f, 360.0f);
	ImGui::DragFloat3("ModelTranslate", reinterpret_cast<float*>(&transform_.translate), 0.1f, -20.0f, 20.0f);
	ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&color_));
	transform_.rotate = Radian(imRotate);

	ImGui::End();

	ImGui::Begin("DirectionalLight");
	Vector3 imDirection = LightManager::GetInstance()->GetDirectionalLightData()->direction;
	ImGui::DragFloat3("LightDirection", reinterpret_cast<float*>(&imDirection), 0.1f, -1.0f, 1.0f);
	ImGui::DragFloat("LightIntensity", &LightManager::GetInstance()->GetDirectionalLightData()->intensity, 0.1f, 0.0f, 1.0f);
	LightManager::GetInstance()->GetDirectionalLightData()->direction = imDirection.Normalize();
	ImGui::End();

	ImGui::Begin("PointLight");
	ImGui::DragFloat3("LightPosition", reinterpret_cast<float*>(&LightManager::GetInstance()->GetLightData("testPoint")->position), 0.1f, -10.0f, 10.0f);
	ImGui::DragFloat("LightIntensity", &LightManager::GetInstance()->GetLightData("testPoint")->intensity, 0.1f, 0.0f, 1.0f);
	ImGui::End();

	ImGui::Begin("PointLight2");
	ImGui::DragFloat3("LightPosition", reinterpret_cast<float*>(&LightManager::GetInstance()->GetLightData("testPoint2")->position), 0.1f, -10.0f, 10.0f);
	ImGui::DragFloat("LightIntensity", &LightManager::GetInstance()->GetLightData("testPoint2")->intensity, 0.1f, 0.0f, 1.0f);
	ImGui::End();

	ImGui::Begin("SpotLight");
	ImGui::DragFloat3("LightPosition", reinterpret_cast<float*>(&LightManager::GetInstance()->GetLightData("testSpot")->position), 0.1f, -10.0f, 10.0f);
	ImGui::DragFloat("LightIntensity", &LightManager::GetInstance()->GetLightData("testSpot")->intensity, 0.1f, 0.0f, 1.0f);
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
	}

	Camera::GetInstance()->Update();
}

void GameScene::Draw() {
	//backGroundSprite_.Draw(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f ,0.0f,0.0f}, { backGroundSprite_.GetSize().x / 2.0f,backGroundSprite_.GetSize().y / 2.0f ,100.0f}));

	Renderer::GetInstance()->DrawSphere(transform_, "uvChecker", { 1.0f,1.0f,1.0f,1.0f });

	Renderer::GetInstance()->DrawLine({ 0.0f,0.0f,0.0f }, { 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->DrawLine({ 1.0f,1.0f,1.0f }, { 1.0f,-1.0f,1.0f }, { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->DrawLine({ 1.0f,-1.0f,1.0f }, { 0.0f,-1.0f,0.0f }, { 1.0f,1.0f,1.0f,1.0f });
}
