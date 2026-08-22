#include "MirrorHalberd.h"

void MirrorHalberd::Initialize() {
	halberdModel_.Initialize("halberd");
	halberdModel_.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("halberd_soul"));
	transform_.Initialize();
	transform_.translate = { 0.0f,0.0f,0.0f };
	SetSize({ 0.6f,2.8f,0.8f });
	SetRadius(3.5f);
	SetColliderType(ColliderType::kBox);
	SetDimensionType(ColliderDimensionType::k3D);
	SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemyAttack));
	SetCollisionMask(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayer));
	SetDebugColor({ 1.0f,1.0f,1.0f,1.0f });
	SetDamage(10.0f);
	SetDamageCoolTime(3.0f);
	isColliderActive_ = false;

	isActive_ = false;
}

void MirrorHalberd::Update() {
	SetActive(isActive_);
	if (!isActive_) {
		return;
	}
	
	CollisionManager::GetInstance()->AddColliderList(this);
}

void MirrorHalberd::Draw() {
	if (!isActive_) {
		return;
	}
	DrawCollider();

	Renderer* renderer = Renderer::GetInstance();
	renderer->DrawModel(transform_, &halberdModel_, true);
	renderer->DrawShadow(transform_, &halberdModel_, { 0.0f,0.0f,0.0f,1.0f });
}