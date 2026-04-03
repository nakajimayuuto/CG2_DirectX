#include "Camera.h"
#include "GameSystem.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	GameSystem system;

	system.Initialize();

	/*=============================================================
	ここから下がゲームの変数.
	=============================================================*/



	bool isTriangleAutoMove = false;

	//bool useMonsterBall = true;
	uint32_t textureNumber = 2;

	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Transform uvTransformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };


	Camera::GetInstance()->Initialize(system.GetWindowSize().width, system.GetWindowSize().height);

	// ウィンドウのxボタンが押されるまでループ.
	while (system.ProcessMessage()) {
		if (system.BeginFrame()) {
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

			Vector4 imColor = system.materialData->color;

			ImGui::ColorEdit4("color", reinterpret_cast<float*>(&imColor));

			system.materialData->color = imColor;

			//ImGui::Checkbox("useMonsterBall",&useMonsterBall);
			//ImGui::SliderInt("texture", reinterpret_cast<int*>(textureNumber),0,2);

			ImGui::Checkbox("enableLighting", reinterpret_cast<bool*>(&system.materialData->enableLighting));

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
			imColor = system.directionalLightData->color;

			ImGui::ColorEdit4("color", reinterpret_cast<float*>(&imColor));

			system.directionalLightData->color = imColor;

			Vector3 imDirection = system.directionalLightData->direction;

			ImGui::SliderFloat3("direction", reinterpret_cast<float*>(&imDirection), -1.0f, 1.0f);

			system.directionalLightData->direction = imDirection.Normalize();

			ImGui::SliderFloat("intensity", &system.directionalLightData->intensity, 0.0f, 1.0f);

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

			system.wvpData->World = worldMatrix;
			system.wvpData->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);

			worldMatrix = Matrix4x4::MakeAffineMatrix(transformSprite);

			system.transformationMatrixDataSprite->World = worldMatrix;
			system.transformationMatrixDataSprite->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrixSprite(worldMatrix);

			system.materialDataSprite->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransformSprite);

			/*=============================================================
			以下にゲームの描画処理を記述.
			=============================================================*/
			system.DrawSetup();

			// ここをTextureManagerに変える
			system.TextureManagerProgram();

			system.Endframe();
		}
	}

	system.Finalize();

	return 0;
}
