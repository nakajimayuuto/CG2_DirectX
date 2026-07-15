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
	TextureManager::GetInstance()->RegisterTexture("window_back", "Resource/Window/window_back.png");
	TextureManager::GetInstance()->RegisterTexture("window_mask", "Resource/Window/window_mask.png");

	// カメラ位置の調整
	Camera::GetInstance()->SetPosition({ 0.0f,0.0f,-10.0f });
	skydome_ = new Skydome();
	skydome_->Initialize();


	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("player"));
	transform_.Initialize();

	sprite_.Initialize(TextureManager::GetInstance()->GetTextureInfo("window_back"));
	transformSprite_.Initialize();
	transformSprite_.translate.x = (sprite_.GetSize().x / 2.0f);
	transformSprite_.translate.y = (sprite_.GetSize().y / 2.0f);
	transformSprite_.translate.z = 200.0f;

	transform_.rotate.y = Radian(-30.0f);
	DirectionalLight::GetInstance()->GetDirectionalLightData()->direction = { 0.0f,0.0f,1.0f };
	GetWindowRect(GameSystem::GetInstance()->GetHWND(), &windowRect);
	windowFirstRect = windowRect;

	spriteTest2.Initialize(TextureManager::GetInstance()->GetTextureInfo("window_mask"));
	transformSpriteTest2_.Initialize();
	transformSpriteTest2_.translate.x = (spriteTest2.GetSize().x / 2.0f);
	transformSpriteTest2_.translate.y = (spriteTest2.GetSize().y / 2.0f);
	transformSpriteTest2_.translate.z = 0.0f;

	frameSpriteRight_.Initialize(TextureManager::GetInstance()->GetTextureInfo("white_template"));
	frameSpriteRight_.SetSize(Environment::GetInstance()->GetWindowSize());
	frameSpriteRight_.SetColor({0.0f,0.0f,0.0f,1.0f});

	backGroundSprite_.Initialize(TextureManager::GetInstance()->GetTextureInfo("white_template"));
	backGroundSprite_.SetSize(Environment::GetInstance()->GetWindowSize());
	backGroundSprite_.SetColor({ 0.1f,0.25f,0.5f,1.0f });

	model_.SetIsVisible(false);

	Environment::GetInstance()->SetWindowMode(kFullscreen);
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

	bool imBool = model_.GetIsVisible();
	ImGui::Checkbox("ModelVisible", &imBool);
	Vector3 imRotate = Degree(transform_.rotate);
	ImGui::DragFloat3("ModelScale", reinterpret_cast<float*>(&transform_.scale), 0.1f, 0.0f, 5.0f);
	ImGui::DragFloat3("ModelRotate", reinterpret_cast<float*>(&imRotate), 0.1f, -20.0f, 20.0f);
	ImGui::DragFloat3("ModelTranslate", reinterpret_cast<float*>(&transform_.translate), 0.1f, -20.0f, 20.0f);
	transform_.rotate = Radian(imRotate);
	model_.SetIsVisible(imBool);

	imBool = sprite_.GetIsVisible();
	float imRotateX = Degree(transformSprite_.rotate.x);
	ImGui::Checkbox("SpriteVisible", &imBool);
	ImGui::DragFloat2("SpriteScale", reinterpret_cast<float*>(&transformSprite_.scale), 0.1f, 0.0f, 10.0f);
	ImGui::DragFloat3("SpriteRotate", reinterpret_cast<float*>(&imRotateX), 0.1f, -20.0f, 20.0f);
	ImGui::DragFloat2("SpriteTranslate", reinterpret_cast<float*>(&transformSprite_.translate), 10.0f, 0.0f, 1280.0f);
	transformSprite_.rotate.x = Radian(imRotateX);
	sprite_.SetIsVisible(imBool);

	imBool = spriteTest2.GetIsVisible();
	imRotateX = Degree(transformSpriteTest2_.rotate.x);
	ImGui::Checkbox("SpriteVisible2", &imBool);
	ImGui::DragFloat2("SpriteScale2", reinterpret_cast<float*>(&transformSpriteTest2_.scale), 0.1f, 0.0f, 10.0f);
	ImGui::DragFloat3("SpriteRotate2", reinterpret_cast<float*>(&imRotateX), 0.1f, -20.0f, 20.0f);
	ImGui::DragFloat2("SpriteTranslate2", reinterpret_cast<float*>(&transformSpriteTest2_.translate), 10.0f, 0.0f, 1280.0f);
	transformSpriteTest2_.rotate.x = Radian(imRotateX);
	spriteTest2.SetIsVisible(imBool);

	ImGui::End();

	Camera::GetInstance()->Update();
}

void GameScene::Draw() {
	sprite_.SetBlendMode(BlendMode::kStencilNoneNormal);
	sprite_.Draw(transformSprite_);
	spriteTest2.SetBlendMode(BlendMode::kStencil);
	spriteTest2.Draw(transformSpriteTest2_);
	backGroundSprite_.Draw(Transform::GetInitialValue({ 100.0f,100.0f,100.0f }, { 0.0f ,0.0f,0.0f}, { -backGroundSprite_.GetSize().x / 2.0f * 50.0f,-backGroundSprite_.GetSize().y / 2.0f * 50.0f ,100.0f}));



	model_.Draw(transform_);

	//skydome_->Draw();
	//frameSpriteRight_.Draw({ {1.0f,1.0f},0.0f, {Environment::GetInstance()->GetWindowSize().width + (frameSpriteRight_.GetSize().x / 2.0f),frameSpriteRight_.GetSize().y / 2.0f } });
}
