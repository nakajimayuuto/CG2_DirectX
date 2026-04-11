#include "GameScene.h"
#include "Camera.h"
#include "ImGui.h"
#include "Math.h"
#include "DirectionalLight.h"
#include "SoundManager.h"
#include "InputManager.h"

void GameScene::Initialize() {
	ModelManager::GetInstance()->RegisterObj("test", "Resource", "axis.obj");

	TextureManager::GetInstance()->RegisterTexture("monster_ball", "Resource/monsterBall.png");
	TextureManager::GetInstance()->RegisterTexture("checker", "Resource/uvChecker.png");

	testModel.Initialize(ModelManager::GetInstance()->GetModelInfo("test"));

	testSphere.Initialize(TextureManager::GetInstance()->GetTextureInfo("monster_ball"));

	testSprite.Initialize(TextureManager::GetInstance()->GetTextureInfo("monster_ball"));

	SoundManager::GetInstance()->RegisterSound("Alarm1","Resource/Alarm01.wav");
	SoundManager::GetInstance()->RegisterSound("Alarm2","Resource/Alarm02.wav");
	SoundManager::GetInstance()->RegisterSound("Alarm3","Resource/Alarm03.wav");
}

void GameScene::Update() {
	// 球のもろもろ.
#ifdef USE_IMGUI
	bool isVisible;
	Vector3 rotate;

	/*=============================================================
	Modelのデバッグ.
	=============================================================*/
	ImGui::Begin("Model");

	isVisible = testModel.GetIsVisible();

	ImGui::Checkbox("testModelVisible", reinterpret_cast<bool*>(&isVisible));

	testModel.SetIsVisible(isVisible);

	rotate = Degree(transformModel.rotate);
	ImGui::SliderFloat3("scale", reinterpret_cast<float*>(&transformModel.scale), 0.0f, 2.0f);
	ImGui::SliderFloat3("rotate", reinterpret_cast<float*>(&rotate), -360.0f, 360.0f);
	ImGui::SliderFloat3("translate", reinterpret_cast<float*>(&transformModel.translate), -5.0f, 5.0f);
	transformModel.rotate = Radian(rotate);

	Vector4 imColor = testModel.materialData_->color;

	ImGui::ColorEdit4("color", reinterpret_cast<float*>(&imColor));

	testModel.materialData_->color = imColor;

	ImGui::Checkbox("testEnableLighting", reinterpret_cast<bool*>(&testModel.materialData_->enableLighting));

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

	imColor = testSphere.materialData_->color;

	ImGui::ColorEdit4("color", reinterpret_cast<float*>(&imColor));

	testSphere.materialData_->color = imColor;

	int numberTemp = textureNumber_;
	ImGui::SliderInt("texture", &textureNumber_, 0, 1);

	if (InputManager::GetInstance()->PressKey(DIK_RIGHT)) {
		transformSphere.translate.x += 0.02f;
	}

	if (InputManager::GetInstance()->PressKey(DIK_LEFT)) {
		transformSphere.translate.x -= 0.02f;
	}

	if (InputManager::GetInstance()->PressKey(DIK_UP)) {
		transformSphere.translate.z += 0.02f;
	}

	if (InputManager::GetInstance()->PressKey(DIK_DOWN)) {
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

	ImGui::Checkbox("testEnableLighting", reinterpret_cast<bool*>(&testSphere.materialData_->enableLighting));

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

	imColor = testSprite.materialData_->color;
	ImGui::ColorEdit4("color", reinterpret_cast<float*>(&imColor));
	testSprite.materialData_->color = imColor;

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

	testSprite.materialData_->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransformSprite);
}

void GameScene::Draw() {
	testModel.Draw(transformModel);
	testSphere.Draw(transformSphere);
	testSprite.Draw(transformSprite);
}