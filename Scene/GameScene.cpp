#include "GameScene.h"
#include "../Satlib.h"

void GameScene::Initialize() {
	// 使用するテクスチャの読み込み.
	ModelManager::GetInstance()->RegisterObj("skydome", "Resource/skydome", "skydome.obj");
	ModelManager::GetInstance()->RegisterObj("player", "Resource/player", "player.obj");
	ModelManager::GetInstance()->RegisterObj("plane", "Resource", "plane.obj");

	// 使用するテクスチャの読み込み.
	TextureManager::GetInstance()->RegisterTexture("uvChecker", "Resource/uvChecker.png");
	TextureManager::GetInstance()->RegisterTexture("monsterBall", "Resource/monsterBall.png");
	TextureManager::GetInstance()->RegisterTexture("effect_triangle", "Resource/effect_triangle.png");

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

	transform_.rotate.y = Radian(-30.0f);
	DirectionalLight::GetInstance()->GetDirectionalLightData()->direction = { 0.0f,0.0f,1.0f };
	GetWindowRect(GameSystem::GetInstance()->GetHWND(), &windowRect);
	windowFirstRect = windowRect;

	frameSpriteLeft_.Initialize(TextureManager::GetInstance()->GetTextureInfo("white_template"));
	frameSpriteLeft_.SetSize(Environment::GetInstance()->GetWindowSize());
	frameSpriteLeft_.SetColor({0.0f,0.0f,0.0f,1.0f});

	frameSpriteRight_.Initialize(TextureManager::GetInstance()->GetTextureInfo("white_template"));
	frameSpriteRight_.SetSize(Environment::GetInstance()->GetWindowSize());
	frameSpriteRight_.SetColor({0.0f,0.0f,0.0f,1.0f});

	backGroundSprite_.Initialize(TextureManager::GetInstance()->GetTextureInfo("white_template"));
	backGroundSprite_.SetSize(Environment::GetInstance()->GetWindowSize());
	backGroundSprite_.SetColor({ 0.1f,0.25f,0.5f,1.0f });
}

void GameScene::Update() {
	if (InputManager::GetInstance()->TriggerKey(DIK_R) && InputManager::GetInstance()->PressKey(DIK_LSHIFT)) {
		SetWindowLongW(GameSystem::GetInstance()->GetHWND(), GWL_STYLE, WS_OVERLAPPEDWINDOW);

		SetWindowPos(
			GameSystem::GetInstance()->GetHWND(),
			HWND_TOP,
			windowFirstRect.left,
			windowFirstRect.top,
			windowFirstRect.right - windowFirstRect.left,
			windowFirstRect.bottom - windowFirstRect.top,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW
		);

	}

	if (InputManager::GetInstance()->TriggerKey(DIK_R) && !InputManager::GetInstance()->PressKey(DIK_LSHIFT)) {
		SetWindowLongW(GameSystem::GetInstance()->GetHWND(), GWL_STYLE, WS_OVERLAPPEDWINDOW);

		SetWindowPos(
			GameSystem::GetInstance()->GetHWND(),
			HWND_TOP,
			windowRect.left,
			windowRect.top,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW
		);

	}

	if (InputManager::GetInstance()->TriggerKey(DIK_S)) {
		GetWindowRect(GameSystem::GetInstance()->GetHWND(), &windowRect);
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_F11)) {
		if (Environment::GetInstance()->GetWindowMode() == kFullscreen) {
			Environment::GetInstance()->SetWindowMode(kWindowed);
		} else {
			Environment::GetInstance()->SetWindowMode(kFullscreen);
		}
	}

	Environment* environment = Environment::GetInstance();

	ImGui::Begin("AspectMode");

	int imInt = static_cast<int>(environment->GetAspectMode());
	ImGui::SliderInt("mode",&imInt,0,kAspectCountMax - 1);
	environment->SetAspectMode(static_cast<AspectMode>(imInt));

	ImGui::End();

	ImGui::Begin("ObjectMove");

	Vector3 imRotate = Degree(transform_.rotate);
	ImGui::DragFloat3("ModelScale",reinterpret_cast<float*>(&transform_.scale),0.1f,0.0f,5.0f);
	ImGui::DragFloat3("ModelRotate",reinterpret_cast<float*>(&imRotate),0.1f,-20.0f,20.0f);
	ImGui::DragFloat3("ModelTranslate",reinterpret_cast<float*>(&transform_.translate),0.1f,-20.0f,20.0f);
	transform_.rotate = Radian(imRotate);

	float imRotateX = Degree(transformSprite_.rotate.x);
	ImGui::DragFloat2("SpriteScale",reinterpret_cast<float*>(&transformSprite_.scale),0.1f,0.0f,10.0f);
	ImGui::DragFloat("SpriteRotate",&imRotateX,0.1f,-20.0f,20.0f);
	ImGui::DragFloat2("SpriteTranslate",reinterpret_cast<float*>(&transformSprite_.translate),10.0f,0.0f,1280.0f);
	transformSprite_.rotate.x = Radian(imRotateX);

	ImGui::End();

	Camera::GetInstance()->Update();
}

void GameScene::Draw() {
	backGroundSprite_.Draw(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f ,0.0f,0.0f}, { backGroundSprite_.GetSize().x / 2.0f,backGroundSprite_.GetSize().y / 2.0f ,100.0f}));

	//skydome_->Draw();

	model_.Draw(transform_);

	sprite_.Draw(transformSprite_);

	frameSpriteLeft_.Draw({ {1.0f,1.0f},0.0f, {-frameSpriteLeft_.GetSize().x / 2.0f,frameSpriteLeft_.GetSize().y / 2.0f} });
	frameSpriteRight_.Draw({ {1.0f,1.0f},0.0f, {Environment::GetInstance()->GetWindowSize().width + (frameSpriteRight_.GetSize().x / 2.0f),frameSpriteRight_.GetSize().y / 2.0f } });
}
