#include "ParticleEmitterEntity.h"
#include <algorithm>
#include "../Math/Random.h"
#include "../SystemFile/DeltaTime.h"
#include "Renderer.h"
#include "../../Managers/ParticleManager.h"
#include "../../Managers/ImGuiManager.h"
#include "Camera.h"

#ifdef USE_IMGUI
#include "../../externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace {
nlohmann::json ToJsonArray(const Vector3& v) {
	return nlohmann::json::array({ v.x, v.y, v.z });
}

Vector3 FromJsonArray(const nlohmann::json& json, const Vector3& defaultValue) {
	if (!json.is_array() || json.size() != 3 || !json[0].is_number() || !json[1].is_number() || !json[2].is_number()) {
		return defaultValue;
	}
	return { json[0].get<float>(), json[1].get<float>(), json[2].get<float>() };
}
} // namespace

void ParticleEmitterEntity::Initialize() {
	transform_ = Transform::GetInitialValue();
	timer_ = 0.0f;
	isDeleteRequested_ = false;
}

void ParticleEmitterEntity::Update() {
	if (!isActive_) {
		return;
	}
	timer_ += DeltaTime::GetInstance()->GetDeltaTime();
	const float interval = (std::max)(frequency_, 0.01f);
	while (timer_ >= interval) {
		timer_ -= interval;
		Emit();
	}
}

void ParticleEmitterEntity::Draw() {
#ifdef _DEBUG
	const Vector4 color = { 0.3f, 0.9f, 1.0f, 1.0f };
	switch (shape_) {
	case Shape::kBox:
		Renderer::GetInstance()->DrawBoxWireFrame(transform_, { 1.0f, 1.0f, 1.0f }, color);
		break;
	case Shape::kSphere: {
		// scaleを直径として扱うので半分の大きさの球を描く.
		Transform sphere = transform_;
		sphere.scale = { transform_.scale.x * 0.5f, transform_.scale.y * 0.5f, transform_.scale.z * 0.5f };
		Renderer::GetInstance()->DrawSphereWireFrame(sphere, color);
		break;
	}
	}

	// 目標地点(範囲の中心から見た位置)を線で描く.
	if (targetPosition_.x != 0.0f || targetPosition_.y != 0.0f || targetPosition_.z != 0.0f) {
		Vector3 target = targetPosition_;
		if (isRotateWithEmitter_) {
			Matrix4x4 rotate = Matrix4x4::MakeRotateMatrix(transform_.rotate);
			target = rotate.TransformNomal(targetPosition_);
		}
		const Vector3 center = transform_.GetWorldPosition();
		const Vector3 end = { center.x + target.x, center.y + target.y, center.z + target.z };
		Renderer::GetInstance()->DrawLine(center, end, { 1.0f, 0.8f, 0.2f, 1.0f });
	}
#endif // _DEBUG
}

void ParticleEmitterEntity::Emit() {
	ParticleManager* particleManager = ParticleManager::GetInstance();
	if (!particleManager->HasParticles(particleName_)) {
		return;
	}
	// 初速 / 目標地点 はエミッターの向きに合わせて回す(設定されていれば).
	Vector3 velocity = startVelocity_;
	Vector3 target = targetPosition_;
	if (isRotateWithEmitter_) {
		Matrix4x4 rotate = Matrix4x4::MakeRotateMatrix(transform_.rotate);
		velocity = rotate.TransformNomal(startVelocity_);
		target = rotate.TransformNomal(targetPosition_);
	}
	for (uint32_t i = 0; i < count_; ++i) {
		particleManager->SpawnParticles(particleName_, GetRandomPosition(), velocity, lifeTime_, target);
	}
	emittedCount_ += count_;
}

Vector3 ParticleEmitterEntity::GetRandomPosition() const {
	// 大きさ1の範囲の中のランダムな点を、transform_で範囲の位置・向き・大きさに合わせる.
	Vector3 local{};
	switch (shape_) {
	case Shape::kBox:
		local = Random::GetInstance()->RandomVector3({ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f });
		break;
	case Shape::kSphere:
		local = Random::GetInstance()->RandomCircleVector3(0.5f);
		break;
	}
	Matrix4x4 world = transform_.GetAffineMatrix();
	const Vector3 offset = world.TransformNomal(local);
	return { offset.x + world.matrix[3][0], offset.y + world.matrix[3][1], offset.z + world.matrix[3][2] };
}

void ParticleEmitterEntity::InspectorUpdate() {
	// scale / rotate / translate はBaseEntityTransformと同じタブに出す(同じ名前のBeginは続きに追加される).
	BaseEntityTransform::InspectorUpdate();

	ImGuiManager* imguiManager = ImGuiManager::GetInstance();
	const bool isVisible = imguiManager->Begin(inspectorName);
#ifdef USE_IMGUI
	ImGui::Separator();

	// 出すパーティクル(ParticleManagerに作ってあるものから選ぶ).
	ParticleManager* particleManager = ParticleManager::GetInstance();
	const bool hasParticles = particleManager->HasParticles(particleName_);
	if (ImGui::BeginCombo("パーティクル", particleName_.empty() ? "(選択してください)" : particleName_.c_str())) {
		for (const std::string& name : particleManager->GetParticleNames()) {
			if (ImGui::Selectable(name.c_str(), name == particleName_)) {
				particleName_ = name;
			}
		}
		ImGui::EndCombo();
	}
	if (!particleName_.empty() && !hasParticles) {
		ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "\"%s\" のパーティクルがありません", particleName_.c_str());
	}

	int shape = static_cast<int>(shape_);
	if (ImGui::Combo("範囲の形", &shape, "箱\0球\0")) {
		shape_ = static_cast<Shape>(shape);
	}
	int count = static_cast<int>(count_);
	if (ImGui::DragInt("1回の個数", &count, 0.2f, 0, 1000)) {
		count_ = static_cast<uint32_t>((std::max)(count, 0));
	}
	ImGui::DragFloat("間隔(秒)", &frequency_, 0.005f, 0.01f, 60.0f, "%.3f");
	frequency_ = (std::max)(frequency_, 0.01f);
	ImGui::Checkbox("自動で出す", &isActive_);

	// パーティクルに渡す値.
	ImGui::SeparatorText("パーティクルに渡す値");
	ImGui::DragFloat3("初速", &startVelocity_.x, 0.05f);
	ImGui::DragFloat("寿命(秒)", &lifeTime_, 0.01f, 0.01f, 600.0f, "%.2f");
	lifeTime_ = (std::max)(lifeTime_, 0.01f);
	ImGui::DragFloat3("目標地点", &targetPosition_.x, 0.05f);
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("生成位置から見た位置(ローカル座標)\nGame画面に範囲の中心からの線で表示されます");
	}
	ImGui::Checkbox("エミッターの向きに合わせる", &isRotateWithEmitter_);
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("初速と目標地点をエミッターの rotate で回します");
	}
	if (hasParticles && !particleManager->GetParticles(particleName_)->GetNodeMove()) {
		ImGui::TextDisabled("(ノードで作った動きのパーティクルで使われます)");
	}
	ImGui::Separator();

	ImGui::BeginDisabled(!imguiManager->IsPlaying());
	if (ImGui::Button("今すぐ出す")) {
		Emit();
	}
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
		ImGui::SetTooltip("再生中に1回分出します");
	}
	ImGui::SameLine();
	if (ImGui::Button("カメラの前へ")) {
		// 今のカメラ(再生中ならゲームのカメラ / デバッグカメラならデバッグカメラ)の10先に移動する.
		Matrix4x4 cameraMatrix = Camera::GetInstance()->GetMatrix();
		transform_.translate = {
			cameraMatrix.matrix[3][0] + cameraMatrix.matrix[2][0] * 10.0f,
			cameraMatrix.matrix[3][1] + cameraMatrix.matrix[2][1] * 10.0f,
			cameraMatrix.matrix[3][2] + cameraMatrix.matrix[2][2] * 10.0f,
		};
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("今見えているカメラの前に移動します\n(停止中のカメラは再生中のカメラと位置が違うことがあります)");
	}

	// 出ているか確認する用.
	if (hasParticles) {
		ImGui::TextDisabled("出した数: %u  /  \"%s\" が今出ている数: %zu", emittedCount_, particleName_.c_str(),
			particleManager->GetParticles(particleName_)->GetParticleCount());
	}
	if (imguiManager->IsPlaying() && !Camera::GetInstance()->GetUseDebugCamera()) {
		const Vector3 position = transform_.GetWorldPosition();
		const float radius = (std::max)({ transform_.scale.x, transform_.scale.y, transform_.scale.z, 0.5f });
		if (!Camera::GetInstance()->IsInCameraFrustum(position, radius)) {
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "ゲームのカメラに映っていません(「カメラの前へ」で移動できます)");
		}
	}

	ImGui::Separator();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.65f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.25f, 0.25f, 1.0f));
	if (ImGui::Button("削除")) {
		isDeleteRequested_ = true;
	}
	ImGui::PopStyleColor(2);

	// タブが表示されている時だけギズモを出す.
	if (isVisible) {
		imguiManager->Gizmo(inspectorName, transform_);
	}
#else
	(void)isVisible;
#endif // USE_IMGUI
	imguiManager->End();
}

nlohmann::json ParticleEmitterEntity::ToJson() const {
	return {
		{ "name", inspectorName },
		{ "particle", particleName_ },
		{ "translate", ToJsonArray(transform_.translate) },
		{ "rotate", ToJsonArray(transform_.rotate) },
		{ "scale", ToJsonArray(transform_.scale) },
		{ "shape", shape_ == Shape::kBox ? "box" : "sphere" },
		{ "count", count_ },
		{ "frequency", frequency_ },
		{ "active", isActive_ },
		{ "startVelocity", ToJsonArray(startVelocity_) },
		{ "lifeTime", lifeTime_ },
		{ "targetPosition", ToJsonArray(targetPosition_) },
		{ "rotateWithEmitter", isRotateWithEmitter_ },
	};
}

void ParticleEmitterEntity::FromJson(const nlohmann::json& json) {
	if (!json.is_object()) {
		return;
	}
	inspectorName = json.value("name", inspectorName);
	particleName_ = json.value("particle", std::string{});
	if (json.contains("translate")) { transform_.translate = FromJsonArray(json["translate"], transform_.translate); }
	if (json.contains("rotate")) { transform_.rotate = FromJsonArray(json["rotate"], transform_.rotate); }
	if (json.contains("scale")) { transform_.scale = FromJsonArray(json["scale"], transform_.scale); }
	shape_ = json.value("shape", std::string("box")) == "sphere" ? Shape::kSphere : Shape::kBox;
	count_ = json.value("count", 3u);
	frequency_ = (std::max)(json.value("frequency", 0.1f), 0.01f);
	isActive_ = json.value("active", true);
	if (json.contains("startVelocity")) { startVelocity_ = FromJsonArray(json["startVelocity"], startVelocity_); }
	lifeTime_ = (std::max)(json.value("lifeTime", 1.0f), 0.01f);
	if (json.contains("targetPosition")) { targetPosition_ = FromJsonArray(json["targetPosition"], targetPosition_); }
	isRotateWithEmitter_ = json.value("rotateWithEmitter", true);
}
