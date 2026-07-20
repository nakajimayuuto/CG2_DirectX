#include "GameScene.h"
#include "../Satlib.h"

void GameScene::Initialize() {

	CreateFakeWindow();
}

void GameScene::Update(){

	if (InputManager::GetInstance()->PressKey(DIK_LSHIFT) && InputManager::GetInstance()->TriggerKey(DIK_T)) {
		SceneManager::GetInstance()->ChengeScene(SceneName::kTitleScene);
	}

	for (auto it = fakeWindows_.begin(); it != fakeWindows_.end(); ) {
		if (!(*it)->GetIsActive()) {
			it = fakeWindows_.erase(it);
		} else {
			++it;
		}
	}
}

void GameScene::Draw() {
	for (auto& window : fakeWindows_) {
		window->DrawBack();
	}
	for (auto& window : fakeWindows_) {
		window->DrawMask();
	}
}

void GameScene::CreateFakeWindow(){
	std::unique_ptr<FakeWindow> newWindow;
	newWindow = std::make_unique<FakeWindow>();
	newWindow->Initialize();
	newWindow->SetType(WindowType::kWindowTypeSquareS);
	fakeWindows_.push_back(std::move(newWindow));
}
