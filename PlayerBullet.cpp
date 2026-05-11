#include "PlayerBullet.h"

void PlayerBullet::Initialize(const std::string& modelName, const Vector3& position){
	model_.Initialize(TextureManager::GetInstance()->GetTextureInfo(modelName));
	transform_.Initialize();
	transform_.translate = position;
}

void PlayerBullet::Update(){
}

void PlayerBullet::Draw(){
	model_.Draw(transform_);
}
