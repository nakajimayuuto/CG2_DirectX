#include "TitleScene.h"
#include "../Satlib.h"

void TitleScene::Initialize() {

	// カメラ位置の調整
	Camera::GetInstance()->SetPosition({ 0.0f,0.0f,-10.0f });
	skydome_ = new Skydome();
	skydome_->Initialize();


	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("player"));
	transform_.Initialize();

	sphereModel_.Initialize(TextureManager::GetInstance()->GetTextureInfo("monsterBall"));
	sphereTransform_.Initialize();
	sphereTransform_.translate.z = 3.0f;

	boxModel_.Initialize(TextureManager::GetInstance()->GetTextureInfo("uvChecker"));
	boxTransform_.Initialize();
	boxTransform_.translate.z = -3.0f;

	transform_.rotate.y = Radian(-30.0f);
	DirectionalLight::GetInstance()->GetDirectionalLightData()->direction = { 0.0f,0.0f,1.0f };
	backGroundSprite_.Initialize(TextureManager::GetInstance()->GetTextureInfo("white_template"));
	backGroundSprite_.SetSize(Environment::GetInstance()->GetWindowSize());
	backGroundSprite_.SetColor({ 0.1f,0.25f,0.5f,1.0f });

	CreateFakeWindow();

	cameraRotateCenter_.Initialize();
	newCameraTransform_.Initialize();
	newCameraTransform_.SetParent(&cameraRotateCenter_);
	newCameraTransform_.translate.z = -30.0f;
}

void TitleScene::Update() {

	if (InputManager::GetInstance()->TriggerKey(DIK_C)) {
		CreateFakeWindow();
	}

	if (InputManager::GetInstance()->PressKey(DIK_LSHIFT) && InputManager::GetInstance()->TriggerKey(DIK_T)) {
		SceneManager::GetInstance()->ChengeScene(SceneName::kGameScene);
	}

	Environment* environment = Environment::GetInstance();

	ImGui::Begin("ObjectMove");
	std::string buttonName;
	if (isRotate_) {
		buttonName = "RotateStop";
	} else {
		buttonName = "RotatePlay";
	}

	if (ImGui::Button(buttonName.c_str())) {
		if (isRotate_) {
			isRotate_ = false;
		} else {
			isRotate_ = true;
		}
	}

	bool imBool = model_.GetIsVisible();
	ImGui::Checkbox("ModelVisible", &imBool);
	Vector3 imRotate = Degree(transform_.rotate);
	ImGui::DragFloat3("ModelScale", reinterpret_cast<float*>(&transform_.scale), 0.1f, 0.0f, 5.0f);
	ImGui::DragFloat3("ModelRotate", reinterpret_cast<float*>(&imRotate), 0.1f, -20.0f, 20.0f);
	ImGui::DragFloat3("ModelTranslate", reinterpret_cast<float*>(&transform_.translate), 0.1f, -20.0f, 20.0f);
	transform_.rotate = Radian(imRotate);
	model_.SetIsVisible(imBool);

	imBool = sphereModel_.GetIsVisible();
	ImGui::Checkbox("SphereVisible", &imBool);
	imRotate = Degree(sphereTransform_.rotate);
	ImGui::DragFloat3("SphereScale", reinterpret_cast<float*>(&sphereTransform_.scale), 0.1f, 0.0f, 5.0f);
	ImGui::DragFloat3("SphereRotate", reinterpret_cast<float*>(&imRotate), 0.1f, -20.0f, 20.0f);
	ImGui::DragFloat3("SphereTranslate", reinterpret_cast<float*>(&sphereTransform_.translate), 0.1f, -20.0f, 20.0f);
	sphereTransform_.rotate = Radian(imRotate);
	sphereModel_.SetIsVisible(imBool);

	imBool = boxModel_.GetIsVisible();
	ImGui::Checkbox("BoxVisible", &imBool);
	imRotate = Degree(boxTransform_.rotate);
	ImGui::DragFloat3("BoxScale", reinterpret_cast<float*>(&boxTransform_.scale), 0.1f, 0.0f, 5.0f);
	ImGui::DragFloat3("BoxRotate", reinterpret_cast<float*>(&imRotate), 0.1f, -20.0f, 20.0f);
	ImGui::DragFloat3("BoxTranslate", reinterpret_cast<float*>(&boxTransform_.translate), 0.1f, -20.0f, 20.0f);
	boxTransform_.rotate = Radian(imRotate);
	boxModel_.SetIsVisible(imBool);


	for (auto it = fakeWindows_.begin(); it != fakeWindows_.end(); ) {
		if (!(*it)->GetIsActive()) {
			it = fakeWindows_.erase(it);
		} else {
			++it;
		}
	}

	if (ImGui::Button("Create")) {
		CreateFakeWindow();
	}

	Vector2 imVector2;
	Vector2 imScale2;
	float imRotateZ;
	uint32_t index = 0;;
	for (auto& window : fakeWindows_) {
		ImGui::PushID(index);
		imVector2 = window->GetTransform().translate;
		imScale2 = window->GetTransform().scale;
		imRotateZ = Degree(window->GetTransform().rotate);
		ImGui::DragFloat2("scale", reinterpret_cast<float*>(&imScale2), 0.1f, 0.0f, 100.0f);
		ImGui::DragFloat("rotate", &imRotateZ, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat2("translate", reinterpret_cast<float*>(&imVector2), 10.0f, -(1920.0f / 2.0f) -(window->GetWindowSize().x / 2.0f), (1920.0f / 2.0f) + (window->GetWindowSize().x / 2.0f));
		window->SetTransform({ imScale2,Radian(imRotateZ),imVector2 });
		int type = static_cast<int>(window->GetType());
		ImGui::SliderInt("Type", &type, 0, kWindowTypeCount - 1);
		if (type != static_cast<int>(window->GetType())) {
			window->SetType(static_cast<WindowType>(type));
		}

		if (ImGui::Button("Delete")) {
			window->SetIsActive(false);
		}
		ImGui::PopID();
		index++;
	}

	ImGui::End();

	cameraRotateCenter_.rotate.y += Radian(1.0f);

	Camera::GetInstance()->SetPosition(newCameraTransform_.GetAffineMatrix().GetMatrixToTranslate());
	Camera::GetInstance()->SetRotate(cameraRotateCenter_.rotate);
	Camera::GetInstance()->Update();
}

void TitleScene::Draw() {
	for (auto& window : fakeWindows_) {
		window->DrawBack();
	}
	for (auto& window : fakeWindows_) {
		window->DrawMask();
	}

	backGroundSprite_.Draw(Transform::GetInitialValue({ 100.0f,100.0f,100.0f }, { 0.0f ,0.0f,0.0f }, { -backGroundSprite_.GetSize().x / 2.0f * 50.0f,-backGroundSprite_.GetSize().y / 2.0f * 50.0f ,100.0f }));
	model_.Draw(transform_);
	sphereModel_.Draw(sphereTransform_);
	boxModel_.Draw(boxTransform_);
}

void TitleScene::CreateFakeWindow() {
	std::unique_ptr<FakeWindow> newWindow;
	newWindow = std::make_unique<FakeWindow>();
	newWindow->Initialize();
	fakeWindows_.push_back(std::move(newWindow));
}