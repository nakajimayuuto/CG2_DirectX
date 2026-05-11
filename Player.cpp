#include "Player.h"

Player::~Player() {

	for (PlayerBullet* bullet : bullets_) {
		delete bullet;
	}
	bullets_.clear();
}

void Player::Initialize() {
	transform_.Initialize();
	model_.Initialize(TextureManager::GetInstance()->GetTextureInfo("player_texture"));
}

void Player::Update() {
	MoveUpdate();
	RotateUpdate();
	AttackUpdate();

	for(PlayerBullet* bullet : bullets_){
		bullet->Update();
	}

	ImGui::Begin("Player");
	ImGui::DragFloat3("transform", reinterpret_cast<float*>(&transform_.translate.x), 0.1f, -3.0f, 3.0f);
	ImGui::End();
}

void Player::MoveUpdate() {
	InputManager* input = InputManager::GetInstance();
	Vector3 move = { 0.0f,0.0f,0.0f };

	if (input->PressKey(DIK_LEFT)) {
		move.x -= kCharacterSpeed;
	} else if (input->PressKey(DIK_RIGHT)) {
		move.x += kCharacterSpeed;
	}

	if (input->PressKey(DIK_UP)) {
		move.y += kCharacterSpeed;
	} else if (input->PressKey(DIK_DOWN)) {
		move.y -= kCharacterSpeed;
	}

	transform_.translate += move;

	transform_.translate.x = std::max(transform_.translate.x, -kMoveLimitX);
	transform_.translate.x = std::min(transform_.translate.x, kMoveLimitX);
	transform_.translate.y = std::max(transform_.translate.y, -kMoveLimitY);
	transform_.translate.y = std::min(transform_.translate.y, kMoveLimitY);
}

void Player::RotateUpdate() {
	InputManager* input = InputManager::GetInstance();

	if (input->PressKey(DIK_A)) {
		transform_.rotate.y -= kRotSpeed;
	} else if (input->PressKey(DIK_D)) {
		transform_.rotate.y += kRotSpeed;
	}

	transform_.rotate.y = std::max(transform_.rotate.y, -kRotateLimitY);
	transform_.rotate.y = std::min(transform_.rotate.y, kRotateLimitY);
}

void Player::AttackUpdate() {
	if (InputManager::GetInstance()->TriggerKey(DIK_SPACE)) {
		PlayerBullet* newBullet = new PlayerBullet;
		newBullet->Initialize("bullet_texture", transform_.translate);

		bullets_.push_back(newBullet);
	}

}

void Player::Draw() {
	for (PlayerBullet* bullet : bullets_) {
		bullet->Draw();
	}

	model_.Draw(transform_);
}
