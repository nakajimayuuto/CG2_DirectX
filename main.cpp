#include "Camera.h"
#include "GameSystem.h"
#include "Renderer.h"
#include "ModelManager.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	GameSystem* system = GameSystem::GetInstance();

	system->Initialize();

	/*=============================================================
	ここから下がゲームの変数.
	=============================================================*/



	bool isTriangleAutoMove = false;

	bool useMonsterBall = true;
	int textureNumber = 1;

	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Transform uvTransformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Camera::GetInstance()->Initialize(system->GetWindowSize().width, system->GetWindowSize().height);

	

	Renderer::Model testModel;

	ModelManager::GetInstance()->RegisterObj("test", "Resource", "axis.obj");
	TextureManager::GetInstance()->RegisterTexture("monster_ball", "Resource/monsterBall.png");
	testModel.Initialize(ModelManager::GetInstance()->GetModelInfo("test"));

	Renderer::Sphere testSphere;

	testSphere.Initialize(TextureManager::GetInstance()->GetTextureInfo("monster_ball"));

	Renderer::Sprite testSprite;

	testSprite.Initialize(TextureManager::GetInstance()->GetTextureInfo("monster_ball"));

	// ウィンドウのxボタンが押されるまでループ.
	while (system->ProcessMessage()) {
		if (system->BeginFrame()) {
//
			/*=============================================================
			以下にゲームの更新処理を記述.
			=============================================================*/

			// 球のもろもろ.
#ifdef USE_IMGUI
			ImGui::Begin("Triangle");
			ImGui::SliderFloat3("scale", reinterpret_cast<float*>(&transform.scale), 0.0f, 2.0f);
			ImGui::SliderFloat3("rotate", reinterpret_cast<float*>(&transform.rotate), 0.0f, Radian(360.0f));
			ImGui::SliderFloat3("translate", reinterpret_cast<float*>(&transform.translate), -5.0f, 5.0f);

			Vector4 imColor = testModel.materialData_->color;

			ImGui::ColorEdit4("color", reinterpret_cast<float*>(&imColor));

			system->materialData->color = imColor;

			testModel.materialData_->color = imColor;

			//ImGui::Checkbox("useMonsterBall",&useMonsterBall);
			ImGui::SliderInt("texture",&textureNumber,0,2);

			ImGui::Checkbox("enableLighting", reinterpret_cast<bool*>(&system->materialData->enableLighting));
			ImGui::Checkbox("testEnableLighting", reinterpret_cast<bool*>(&testModel.materialData_->enableLighting));

			if (ImGui::Button("AutoMove")) {
				if (isTriangleAutoMove) {
					isTriangleAutoMove = false;
				}
				else {
					isTriangleAutoMove = true;
				}

				transform.rotate = { 0.0f,0.0f,0.0f };
			}

			ImGui::Text("AutoMove : %s", isTriangleAutoMove ? "true" : "false");

			ImGui::End();

			ImGui::Begin("Sprite");
			ImGui::SliderFloat2("scale", reinterpret_cast<float*>(&transformSprite.scale), 0.0f, 2.0f);
			ImGui::SliderFloat("rotate", reinterpret_cast<float*>(&transformSprite.rotate.z), 0.0f, Radian(360.0f));
			ImGui::SliderFloat2("translate", reinterpret_cast<float*>(&transformSprite.translate), -640.0f, 1280.0f);

			ImGui::SliderFloat2("UVScale", reinterpret_cast<float*>(&uvTransformSprite.scale), 0.0f, 2.0f);
			ImGui::SliderFloat("UVRotate", reinterpret_cast<float*>(&uvTransformSprite.rotate.z), 0.0f, Radian(360.0f));
			ImGui::SliderFloat2("UVTranslate", reinterpret_cast<float*>(&uvTransformSprite.translate), -640.0f, 1280.0f);

			ImGui::End();


			ImGui::Begin("DirectionalLight");
			imColor = DirectionalLight::GetInstance()->GetDirectionalLightData()->color;

			ImGui::ColorEdit4("color", reinterpret_cast<float*>(&imColor));

			DirectionalLight::GetInstance()->GetDirectionalLightData()->color = imColor;

			Vector3 imDirection = DirectionalLight::GetInstance()->GetDirectionalLightData()->direction;

			ImGui::SliderFloat3("direction", reinterpret_cast<float*>(&imDirection), -1.0f, 1.0f);

			DirectionalLight::GetInstance()->GetDirectionalLightData()->direction = imDirection.Normalize();

			ImGui::SliderFloat("intensity", &DirectionalLight::GetInstance()->GetDirectionalLightData()->intensity, 0.0f, 1.0f);

			ImGui::End();

#endif // USE_IMGUI

			Camera::GetInstance()->Update();

			if (isTriangleAutoMove) {
				transform.rotate.y += 0.03f;

				if (transform.rotate.y >= Radian(360.0f)) {
					transform.rotate.y -= Radian(360.0f);
				}
			}

			Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform);

			//system->wvpData->World = worldMatrix;
			//system->wvpData->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);

			testModel.wvpData_->World = worldMatrix;
			testModel.wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);

			testSphere.wvpData_->World = worldMatrix;
			testSphere.wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);

			worldMatrix = Matrix4x4::MakeAffineMatrix(transformSprite);

			//system->transformationMatrixDataSprite->World = worldMatrix;
			//system->transformationMatrixDataSprite->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrixSprite(worldMatrix);


			//system->materialDataSprite->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransformSprite);

			testSprite.transformationMatrixData_->World = worldMatrix;
			testSprite.transformationMatrixData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrixSprite(worldMatrix);


			testSprite.materialData_->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransformSprite);

			/*=============================================================
			以下にゲームの描画処理を記述.
			=============================================================*/
			system->DrawSetup();

			// testModel.Draw();
			// testSphere.Draw();
			testSprite.Draw();

			system->Endframe();
		}
	}

	system->Finalize();

	return 0;
}
