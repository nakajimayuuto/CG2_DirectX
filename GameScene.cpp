#include "GameScene.h"
#include "Camera.h"
#include "ImGui.h"
#include "Math.h"
#include "DirectionalLight.h"
#include "SoundManager.h"
#include "InputManager.h"
#include "Environment.h"

void GameScene::Initialize() {
	ModelManager::GetInstance()->RegisterObj("test", "Resource", "multiMesh.obj");

	TextureManager::GetInstance()->RegisterTexture("monster_ball", "Resource/monsterBall.png");
	TextureManager::GetInstance()->RegisterTexture("checker", "Resource/uvChecker.png");

	testModel.Initialize(ModelManager::GetInstance()->GetModelInfo("test"));

	testSphere.Initialize(TextureManager::GetInstance()->GetTextureInfo("monster_ball"));

	testSprite.Initialize(TextureManager::GetInstance()->GetTextureInfo("monster_ball"));

	SoundManager::GetInstance()->RegisterSound("Alarm1","Resource/Alarm01.wav");
	SoundManager::GetInstance()->RegisterSound("Alarm2","Resource/Alarm02.wav");
	SoundManager::GetInstance()->RegisterSound("Alarm3","Resource/Alarm03.wav");

	testSphere.SetIsVisible(false);
	testSprite.SetIsVisible(false);
}

void GameScene::Update() {
	if (InputManager::GetInstance()->TriggerKey(DIK_ESCAPE)) {
		Environment::GetInstance()->GameFinished();
	}

	// 球のもろもろ.
#ifdef USE_IMGUI
	bool isVisible;
	Vector3 rotate;

	/*=============================================================
	Modelのデバッグ.
	=============================================================*/
	ImGui::Begin("Model");

	//isVisible = testModel.GetIsVisible();

	ImGui::Checkbox("testModelVisible", reinterpret_cast<bool*>(&isVisible));

	//testModel.SetIsVisible(isVisible);

	rotate = Degree(transformModel.rotate);
	ImGui::SliderFloat3("scale", reinterpret_cast<float*>(&transformModel.scale), 0.0f, 2.0f);
	ImGui::SliderFloat3("rotate", reinterpret_cast<float*>(&rotate), -360.0f, 360.0f);
	ImGui::SliderFloat3("translate", reinterpret_cast<float*>(&transformModel.translate), -5.0f, 5.0f);
	transformModel.rotate = Radian(rotate);

	Vector4 imColor;
		//= testModel.GetColor();

	//ImGui::ColorEdit4("color", reinterpret_cast<float*>(&imColor));

	//testModel.SetColor(imColor);

	int imSlider;
		//= static_cast<int>(testModel.GetLightingType());

	//ImGui::SliderInt("ModelLightingType",&imSlider,0,2);

	//testModel.SetLightingType(static_cast<Renderer::LightingType>(imSlider));

	if (ImGui::Button("AutoMove")) {
		if (isModelAutoMove) {
			isModelAutoMove = false;
		}
		else {
			isModelAutoMove = true;
		}

		transformModel.rotate = { 0.0f,0.0f,0.0f };
	}

	ImGui::Text("AutoMove : %s", isModelAutoMove ? "true" : "false");
	ImGui::End();


	/*=============================================================
	スフィアのデバッグ.
	=============================================================*/
	ImGui::Begin("Sphere");

	isVisible = testSphere.GetIsVisible();

	ImGui::Checkbox("testSphereVisible", reinterpret_cast<bool*>(&isVisible));

	testSphere.SetIsVisible(isVisible);

	rotate = Degree(transformSphere.rotate);
	ImGui::SliderFloat3("scale", reinterpret_cast<float*>(&transformSphere.scale), 0.0f, 2.0f);
	ImGui::SliderFloat3("rotate", reinterpret_cast<float*>(&rotate), -360.0f, 360.0f);
	ImGui::SliderFloat3("translate", reinterpret_cast<float*>(&transformSphere.translate), -5.0f, 5.0f);
	transformSphere.rotate = Radian(rotate);

	imColor = testSphere.GetColor();

	ImGui::ColorEdit4("color", reinterpret_cast<float*>(&imColor));

	testSphere.SetColor(imColor);

	imSlider = static_cast<int>(testSphere.GetLightingType());

	ImGui::SliderInt("ModelLightingType", &imSlider, 0, 2);

	testSphere.SetLightingType(static_cast<Renderer::LightingType>(imSlider));

	int numberTemp = textureNumber_;
	ImGui::SliderInt("texture", &textureNumber_, 0, 1);

	if (InputManager::GetInstance()->TriggerKey(DIK_F3)) {
		Camera::GetInstance()->ChangeCameraMode();
	}

	if (InputManager::GetInstance()->PressKey(DIK_D)) {
		transformSphere.translate.x += 0.02f;
	}
	
	if (InputManager::GetInstance()->PressKey(DIK_A)) {
		transformSphere.translate.x -= 0.02f;
	}
	
	if (InputManager::GetInstance()->PressKey(DIK_W)) {
		transformSphere.translate.z += 0.02f;
	}
	
	if (InputManager::GetInstance()->PressKey(DIK_S)) {
		transformSphere.translate.z -= 0.02f;
	}


	if (numberTemp != textureNumber_) {
		switch (textureNumber_) {
		case 0:
			testSphere.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("monster_ball"));
			break;
		case 1:
			testSphere.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("checker"));
			break;
		}
	}

	if (ImGui::Button("AutoMove")) {
		if (isSphereAutoMove) {
			isSphereAutoMove = false;
		}
		else {
			isSphereAutoMove = true;
		}

		transformSphere.rotate = { 0.0f,0.0f,0.0f };
	}

	ImGui::Text("AutoMove : %s", isSphereAutoMove ? "true" : "false");
	ImGui::End();



	/*=============================================================
	スプライトのデバッグ.
	=============================================================*/

	ImGui::Begin("Sprite");

	isVisible = testSprite.GetIsVisible();
	ImGui::Checkbox("testSpriteVisible", reinterpret_cast<bool*>(&isVisible));
	testSprite.SetIsVisible(isVisible);

	rotate.z = Degree(transformSprite.rotate.z);
	ImGui::SliderFloat2("scale", reinterpret_cast<float*>(&transformSprite.scale), 0.0f, 2.0f);
	ImGui::SliderFloat("rotate", reinterpret_cast<float*>(&rotate.z), -360.0f, 360.0f);
	ImGui::SliderFloat2("translate", reinterpret_cast<float*>(&transformSprite.translate), -640.0f, 1280.0f);
	transformSprite.rotate.z = Radian(rotate.z);

	rotate.z = Degree(uvTransformSprite.rotate.z);
	ImGui::SliderFloat2("UVScale", reinterpret_cast<float*>(&uvTransformSprite.scale), 0.0f, 2.0f);
	ImGui::SliderFloat("UVRotate", reinterpret_cast<float*>(&rotate.z), -360.0f, 360.0f);
	ImGui::SliderFloat2("UVTranslate", reinterpret_cast<float*>(&uvTransformSprite.translate), -640.0f, 1280.0f);
	uvTransformSprite.rotate.z = Radian(rotate.z);

	Vector2 imSize = testSprite.GetSize();
	ImGui::SliderFloat2("Size", reinterpret_cast<float*>(&imSize), 0.0f, 1280.0f);
	testSprite.SetSize(imSize);

	imColor = testSprite.GetColor();
	ImGui::ColorEdit4("color", reinterpret_cast<float*>(&imColor));
	testSprite.SetColor(imColor);

	ImGui::End();


	/*=============================================================
	光源のデバッグ.
	=============================================================*/

	ImGui::Begin("DirectionalLight");
	imColor = DirectionalLight::GetInstance()->GetDirectionalLightData()->color;
	Vector3 imDirection = DirectionalLight::GetInstance()->GetDirectionalLightData()->direction;

	ImGui::ColorEdit4("color", reinterpret_cast<float*>(&imColor));
	ImGui::SliderFloat3("direction", reinterpret_cast<float*>(&imDirection), -1.0f, 1.0f);
	ImGui::SliderFloat("intensity", &DirectionalLight::GetInstance()->GetDirectionalLightData()->intensity, 0.0f, 1.0f);
	DirectionalLight::GetInstance()->GetDirectionalLightData()->color = imColor;
	DirectionalLight::GetInstance()->GetDirectionalLightData()->direction = imDirection.Normalize();
	ImGui::End();


	/*=============================================================
	サウンドのデバッグ.
	=============================================================*/

	ImGui::Begin("Sound");

	ImGui::SliderInt("Sounds",&soundNumber_,0,2);

	if (ImGui::Button("Play")) {
		switch (soundNumber_){
		case 0:
			SoundManager::GetInstance()->SoundPlayWave(SoundManager::GetInstance()->GetSoundData("Alarm1"));
			break;
		case 1:
			SoundManager::GetInstance()->SoundPlayWave(SoundManager::GetInstance()->GetSoundData("Alarm2"));
			break;
		case 2:
			SoundManager::GetInstance()->SoundPlayWave(SoundManager::GetInstance()->GetSoundData("Alarm3"));
			break;
		}
	}

	ImGui::End();

#endif // USE_IMGUI
	Camera::GetInstance()->Update();

	if (isModelAutoMove) {
		transformModel.rotate.y += 0.03f;

		if (transformModel.rotate.y >= Radian(360.0f)) {
			transformModel.rotate.y -= Radian(360.0f);
		}
	}

	if (isSphereAutoMove) {
		transformSphere.rotate.y += 0.03f;

		if (transformSphere.rotate.y >= Radian(360.0f)) {
			transformSphere.rotate.y -= Radian(360.0f);
		}
	}

	testSprite.SetUvTransform(uvTransformSprite);
}

void GameScene::Draw() {
	testModel.Draw(transformModel);
	testSphere.Draw(transformSphere);
	testSprite.Draw(transformSprite);
}