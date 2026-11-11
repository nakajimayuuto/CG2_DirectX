#include "ParticleManager.h"
#include "../Engine/Renderer/ParticleNodeMove.h"
#include "ImGuiManager.h"
#include <algorithm>
#include <cfloat>
#include <format>
#ifdef USE_IMGUI
#include "../externals/imgui/imgui.h"
#endif // USE_IMGUI

ParticleManager* ParticleManager::GetInstance() {
	static ParticleManager instance;
	return &instance;
}

void ParticleManager::Initialize() {
	ParticleClear();

	TextureManager::GetInstance()->RegisterTexture("numbers", "Resources/UI/numbers.png");
	for (uint32_t i = 0; i < 10; i++) {
		CreateNewParticles(std::format("number_{}", i), TextureManager::GetInstance()->GetTextureInfo("numbers"), BillboardType::kAllAxis, Particles::Move::kNumber);
		SetParticleSize(std::format("number_{}", i), { 0.2f,0.2f,0.2f });
		SetParticleUVTransform(std::format("number_{}", i), Transform::GetInitialValue({ 0.1f ,1.0f,0.0f }, { 0.0f,0.0f,0.0f }, { 0.1f * static_cast<float>(i),0.0f,0.0f }));
	}
}

void ParticleManager::ParticleClear() {
	particles_.clear();
	emitters_.clear();
	fields_.clear();
}

void ParticleManager::Update() {

	for (auto& emitter : emitters_) {
		emitter.second->Update();
	}

	CheckCollision();

	for (auto& particles : particles_) {
		particles.second->Update();
	}

}

void ParticleManager::Draw() {
	for (auto& emitter : emitters_) {
		emitter.second->DebugDraw();
	}

	for (auto& field : fields_) {
		field.second->DebugDraw();
	}

	for (auto& particles : particles_) {
		particles.second->Draw();
	}
}

void ParticleManager::CheckCollision() {
	for (auto& field : fields_) {
		for (auto& particles : particles_) {
			particles.second->CheckCollision(*field.second);
		}
	}
}


void ParticleManager::SpawnParticles(const std::string& name, const Transform transform) {

	particles_[name]->MakeNewParticle(transform);

}

void ParticleManager::SpawnParticles(const std::string& name, const Vector3 vector3) {

	particles_[name]->MakeNewParticle(vector3);
}

void ParticleManager::SpawnParticles(const std::string& name, const Vector3& position, const Vector3& velocity, float lifeTime, const Vector3& targetPosition) {
	particles_[name]->MakeNewParticle(position, velocity, lifeTime, targetPosition, { 0.0f,0.0f,0.0f });
}

void ParticleManager::SpawnNumbers(float number, const Transform transform, const Vector3& color) {
	if (number >= 10000.0f) {
		return;
	}

	if (number < 0.0f) {
		return;
	}
	unsigned int absX = static_cast<int>(std::abs(number));
	unsigned int numDigit = 0;

	if (absX == 0) {
		numDigit = 1;
	} else {
		numDigit = static_cast<int>(std::log10(absX)) + 1;
	}

	float numDigitBlank = static_cast<float>(numDigit) - 1.0f;
	float numberTemp = number;
	float temp = 0.0f;

	Vector3 move = { 0.0f,0.0f,1.0f };
	if (numDigit == 1) {
		temp = number;
		particles_[std::format("number_{}", static_cast<int>(temp))]->SetColor(color);
		particles_[std::format("number_{}", static_cast<int>(temp))]->MakeNewParticle(transform.GetWorldPosition());
	}else{
		for (uint32_t i = 0; i < numDigit; i++) {
			numberTemp = numberTemp - temp;
			temp = numberTemp / std::pow(10.0f, numDigitBlank - static_cast<float>(i));
			move = { (0.25f * static_cast<float>(i)) - (0.25f * numDigitBlank),0.0f,0.0f };
			//move = rotateMatrix.TransformNomal(move);
			particles_[std::format("number_{}", static_cast<int>(temp))]->SetColor(color);
			particles_[std::format("number_{}", static_cast<int>(temp))]->MakeNewParticle(transform.GetWorldPosition(), move);
			temp = static_cast<float>(static_cast<int>(temp)) * std::pow(10.0f, numDigitBlank - static_cast<float>(i));
		}
	}
}

void ParticleManager::CreateNewParticles(const std::string& name, const TextureInfo& info) {
	if (particles_.find(name) != particles_.end()) {
		return;
	}

	particles_[name] = std::make_unique<Particles>();
	particles_[name]->Initialize(info);
}

void ParticleManager::CreateNewParticles(const std::string& name, const TextureInfo& info, BillboardType billType, Particles::Move moveType) {
	if (particles_.find(name) != particles_.end()) {
		return;
	}

	particles_[name] = std::make_unique<Particles>();
	particles_[name]->Initialize(info);
	particles_[name]->SetBillboardType(billType);
	particles_[name]->SetMoveType(moveType);
}

void ParticleManager::CreateNewParticles(const std::string& name, const ModelInfo& info) {
	if (particles_.find(name) != particles_.end()) {
		return;
	}

	particles_[name] = std::make_unique<Particles>();
	particles_[name]->Initialize(info);
}

void ParticleManager::CreateNewParticles(const std::string& name, Primitive primitive, const TextureInfo& info, const PrimitiveParam& param) {
	if (particles_.find(name) != particles_.end()) {
		return;
	}

	particles_[name] = std::make_unique<Particles>();
	particles_[name]->Initialize(primitive, info, param);
}

void ParticleManager::CreateNewParticles(const std::string& name, Primitive primitive, const TextureInfo& info, BillboardType billType, Particles::Move moveType, const PrimitiveParam& param) {
	if (particles_.find(name) != particles_.end()) {
		return;
	}

	particles_[name] = std::make_unique<Particles>();
	particles_[name]->Initialize(primitive, info, param);
	particles_[name]->SetBillboardType(billType);
	particles_[name]->SetMoveType(moveType);
}

void ParticleManager::CreateNewParticles(const std::string& name, const TextureInfo& info, BillboardType billType, const std::string& moveName) {
	if (particles_.find(name) != particles_.end()) {
		return;
	}

	particles_[name] = std::make_unique<Particles>();
	particles_[name]->Initialize(info);
	particles_[name]->SetBillboardType(billType);
	SetNodeMove(*particles_[name], moveName);
}

void ParticleManager::CreateNewParticles(const std::string& name, Primitive primitive, const TextureInfo& info, BillboardType billType, const std::string& moveName, const PrimitiveParam& param) {
	if (particles_.find(name) != particles_.end()) {
		return;
	}

	particles_[name] = std::make_unique<Particles>();
	particles_[name]->Initialize(primitive, info, param);
	particles_[name]->SetBillboardType(billType);
	SetNodeMove(*particles_[name], moveName);
}

void ParticleManager::SetNodeMove(Particles& particles, const std::string& moveName) {
	std::shared_ptr<ParticleNodeMove> nodeMove = ParticleNodeMove::Get(moveName);
	if (!nodeMove) {
		// 読み込めなかった時は今まで通りの動き(kNormal)にする.
#ifdef _DEBUG
		OutputDebugStringA(std::format("[ParticleManager] {} が読み込めませんでした\n", ParticleNodeMove::GetFilePath(moveName)).c_str());
		assert(false && "ノードの動き(Resources/Node/Particle/名前.json)が見つかりませんでした");
#endif // _DEBUG
		particles.SetMoveType(Particles::Move::kNormal);
		return;
	}
	particles.SetNodeMove(nodeMove);
	particles.SetMoveType(Particles::Move::kNodeMove);
}

bool ParticleManager::HasParticles(const std::string& name) const {
	return particles_.find(name) != particles_.end();
}

std::vector<std::string> ParticleManager::GetParticleNames() const {
	std::vector<std::string> names;
	for (const auto& [name, particles] : particles_) {
		if (name.rfind("__", 0) != 0) {
			names.push_back(name);
		}
	}
	return names;
}

void ParticleManager::CreateNewEmitter(const std::string& name, const std::string& setParticleName, const Transform& transform, uint32_t count, float frequency) {
	if (emitters_.find(name) != emitters_.end()) {
		return;
	}

	emitters_[name] = std::make_unique<Emitter>();
	emitters_[name]->Initialize(transform, count, frequency);
	emitters_[name]->SetParticle(GetParticles(setParticleName));
}

void ParticleManager::CreateNewEmitter(const std::string& name, const std::string& setParticleName) {
	if (emitters_.find(name) != emitters_.end()) {
		return;
	}

	emitters_[name] = std::make_unique<Emitter>();
	emitters_[name]->Initialize(Transform::GetInitialValue(), 3, 0.3f);
	emitters_[name]->SetParticle(GetParticles(setParticleName));
}

void ParticleManager::CreateNewField(const std::string& name, const AABB& aabb, const Vector3& acceleration) {
	if (fields_.find(name) != fields_.end()) {
		return;
	}

	fields_[name] = std::make_unique<Field>();
	fields_[name]->Initialize(acceleration, aabb);
}

void ParticleManager::SetBillboardType(BillboardType type) {
	for (auto& particles : particles_) {
		if (particles.second->GetBillboardType() == type) {
			continue;
		}

		particles.second->SetBillboardType(type);
	}
}

void ParticleManager::SetBillboardType(const std::string& name, BillboardType type) {
	auto it = particles_.find(name);

	//assert(it != particles_.end(), std::format("name : {}と一致するParticlesが見つかりませんでした", name));

	if (particles_[name]->GetBillboardType() == type) {
		return;
	}

	particles_[name]->SetBillboardType(type);
}

void ParticleManager::SetParticleSize(const std::string& name, const Vector3 size) {
	auto it = particles_.find(name);

	particles_[name]->SetSize(size);
}

void ParticleManager::SetParticleUVTransform(const std::string& name, const Transform& transform) {
	auto it = particles_.find(name);

	particles_[name]->SetUvTransform(transform);
}

void ParticleManager::SetEmitterTransform(const std::string& name, const Transform& transform) {
	auto it = emitters_.find(name);

#ifdef _DEBUG

	assert(it != emitters_.end(), std::format("name : {}と一致するEmitterが見つかりませんでした", name));

#endif // _DEBUG

	emitters_[name]->SetTransform(transform);
}

void ParticleManager::SetEmitterCount(const std::string& name, uint32_t count) {
	auto it = emitters_.find(name);

#ifdef _DEBUG
	assert(it != emitters_.end(), std::format("name : {}と一致するEmitterが見つかりませんでした", name));
#endif //_DEBUG

	emitters_[name]->SetCount(count);
}

void ParticleManager::SetEmitterFrequency(const std::string& name, float frequency) {
	auto it = emitters_.find(name);

#ifdef _DEBUG

	assert(it != emitters_.end(), std::format("name : {}と一致するEmitterが見つかりませんでした", name));

#endif // _DEBUG

	emitters_[name]->SetFrequency(frequency);
}

void ParticleManager::SetEmitterShape(const std::string& name, EmitterShape shape) {
	auto it = emitters_.find(name);
#ifdef _DEBUG
	assert(it != emitters_.end(), std::format("name : {}と一致するEmitterが見つかりませんでした", name));
#endif // _DEBUG
	emitters_[name]->SetShape(shape);
}

void ParticleManager::SetFieldArea(const std::string& name, const AABB& area) {
	auto it = fields_.find(name);
#ifdef _DEBUG
	assert(it != fields_.end(), std::format("name : {}と一致するFieldが見つかりませんでした", name));
#endif // _DEBUG
	fields_[name]->SetArea(area);

}

void ParticleManager::SetFieldAcceleration(const std::string& name, const Vector3& acceleration) {
	auto it = fields_.find(name);
#ifdef _DEBUG
	assert(it != fields_.end(), std::format("name : {}と一致するFieldが見つかりませんでした", name));
#endif // _DEBUG
	fields_[name]->SetAcceleration(acceleration);
}

void ParticleManager::SetFieldIsActive(const std::string& name, bool isActive) {
	auto it = fields_.find(name);
#ifdef _DEBUG
	assert(it != fields_.end(), std::format("name : {}と一致するFieldが見つかりませんでした", name));
#endif // _DEBUG
	fields_[name]->SetIsActive(isActive);
}

Particles* ParticleManager::GetParticles(const std::string& name) {
	auto it = particles_.find(name);
#ifdef _DEBUG
	assert(it != particles_.end(), std::format("name : {}と一致するParticlesが見つかりませんでした", name));
#endif // _DEBUG
	return it->second.get();
}

void ParticleManager::ClearParticles() {
	for (auto& particles : particles_) {
		particles.second->ClearParticle();
	}
}

void ParticleManager::DrawEmitterGui() {
#ifdef USE_IMGUI
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();
	// タブが選ばれていない時はギズモも出さない(Begin / End は必ず対にする).
	const bool isVisible = imguiManager->Begin("Emitter");
	if (!isVisible) {
		imguiManager->End();
		return;
	}

	if (emitters_.empty()) {
		ImGui::TextDisabled("Emitterがありません(CreateNewEmitterで作られると表示されます)");
		imguiManager->End();
		return;
	}

	// 選んでいるEmitterが無くなっていたら先頭を選び直す.
	if (emitters_.find(selectedEmitterName_) == emitters_.end()) {
		selectedEmitterName_ = emitters_.begin()->first;
	}
	ImGui::SetNextItemWidth(-FLT_MIN);
	if (ImGui::BeginCombo("##EmitterSelect", selectedEmitterName_.c_str())) {
		for (const auto& [name, emitter] : emitters_) {
			if (ImGui::Selectable(name.c_str(), name == selectedEmitterName_)) {
				selectedEmitterName_ = name;
			}
		}
		ImGui::EndCombo();
	}

	Emitter& emitter = *emitters_[selectedEmitterName_];
	Transform& transform = emitter.GetTransform();

	// 使っているパーティクルの名前.
	const char* particleName = "(なし)";
	for (const auto& [name, particles] : particles_) {
		if (particles.get() == emitter.GetParticles()) {
			particleName = name.c_str();
			break;
		}
	}
	ImGui::TextDisabled("パーティクル: %s", particleName);
	ImGui::Separator();

	ImGui::DragFloat3("位置", &transform.translate.x, 0.05f);
	ImGui::DragFloat3("範囲", &transform.scale.x, 0.05f, 0.0f, 1000.0f);

	int shape = static_cast<int>(emitter.GetShape());
	if (ImGui::Combo("形", &shape, "箱\0球\0")) {
		emitter.SetShape(static_cast<EmitterShape>(shape));
	}

	int count = static_cast<int>(emitter.GetCount());
	if (ImGui::DragInt("1回の個数", &count, 0.2f, 0, 1000)) {
		emitter.SetCount(static_cast<uint32_t>((std::max)(count, 0)));
	}

	bool isAuto = emitter.GetFrequency() >= 0.0f;
	if (ImGui::Checkbox("自動で生成", &isAuto)) {
		emitter.SetFrequency(isAuto ? 0.1f : -1.0f);
	}
	if (isAuto) {
		float frequency = emitter.GetFrequency();
		if (ImGui::DragFloat("間隔(秒)", &frequency, 0.005f, 0.01f, 60.0f, "%.3f")) {
			emitter.SetFrequency((std::max)(frequency, 0.01f));
		}
	}

	ImGui::BeginDisabled(!imguiManager->IsPlaying());
	if (ImGui::Button("今すぐ生成")) {
		emitter.CreateParticle();
	}
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
		ImGui::SetTooltip("再生中に1回分生成します");
	}
	ImGui::SameLine();
	// 調整した値は終了ボタンでシーンを作り直すと元に戻るので、コードに貼れる形でコピーできるようにする.
	if (ImGui::Button("設定をコピー")) {
		const std::string code = std::format(
			"ParticleManager::GetInstance()->SetEmitterTransform(\"{0}\", Transform::GetInitialValue({{ {1:.3f}f,{2:.3f}f,{3:.3f}f }}, {{ 0.0f,0.0f,0.0f }}, {{ {4:.3f}f,{5:.3f}f,{6:.3f}f }}));\n"
			"ParticleManager::GetInstance()->SetEmitterCount(\"{0}\", {7});\n"
			"ParticleManager::GetInstance()->SetEmitterFrequency(\"{0}\", {8:.3f}f);\n"
			"ParticleManager::GetInstance()->SetEmitterShape(\"{0}\", EmitterShape::{9});\n",
			selectedEmitterName_,
			transform.scale.x, transform.scale.y, transform.scale.z,
			transform.translate.x, transform.translate.y, transform.translate.z,
			emitter.GetCount(), emitter.GetFrequency(),
			emitter.GetShape() == EmitterShape::kBox ? "kBox" : "kSphere");
		ImGui::SetClipboardText(code.c_str());
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("今の値を設定するコードをクリップボードにコピーします\n(終了ボタンを押すとシーンが作り直されて値が元に戻ります)");
	}

	ImGui::Checkbox("ギズモを表示", &isEmitterGizmoVisible_);
	if (isEmitterGizmoVisible_) {
		// Emitterは回転を使わないので 移動 / 拡縮 だけ.
		imguiManager->Gizmo("Emitter:" + selectedEmitterName_, transform, ImGuiManager::kGizmoTranslate | ImGuiManager::kGizmoScale);
	}

	imguiManager->End();
#endif // USE_IMGUI
}
