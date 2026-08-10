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
	CreateModelData({ -2.0f,0.0f,2.0f }, DrawModelType::Sphere);
	CreateModelData({ -6.0f,0.0f,4.0f }, DrawModelType::MultiMesh);
	CreateModelData({ 0.0f,0.0f,8.0f }, DrawModelType::MultiMaterial);
	CreateModelData({ 2.0f,0.0f,2.0f }, DrawModelType::UtahTeapot);
	CreateModelData({ 4.0f,0.0f,4.0f }, DrawModelType::StanfordBunny);
	CreateModelData({ 2.0f,0.0f,6.0f }, DrawModelType::Suzzanne);
	Camera::GetInstance()->SetPosition({ 0.0f,10.0f,-13.0f });
	Camera::GetInstance()->SetRotate({ Radian(30.0f),0.0f,0.0f });
	Camera::GetInstance()->ChangeCameraMode();
	currentNewModelType_ = DrawModelType::Plane;

	data = SoundManager::GetInstance()->GetSoundData("test");
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

	if (InputManager::GetInstance()->TriggerKey(DIK_F3)) {
		Camera::GetInstance()->ChangeCameraMode();
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
			Renderer::GetInstance()->DrawSphere(modelData->transform, modelData->sphereInfo, modelData->model.GetColor(), modelData->model.GetUvTransform());
			Renderer::GetInstance()->SetLightingType(LightingType::kHalfLambert);
		} else if (modelData->type == DrawModelType::Sprite) {
			modelData->sprite.Draw(modelData->transform);
		} else {
			modelData->model.Draw(modelData->transform);
		}
	}
}