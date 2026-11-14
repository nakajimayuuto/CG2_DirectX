#include "Particles.h"
#include "../SystemFile/DeltaTime.h"
#include "../../Managers/SrvManager.h"
#include "../Math/Random.h"
#include "../Renderer/Camera.h"
#include "../Math/Easing.h"
#include "Renderer.h"
#include "ParticleNodeMove.h"

Particles::~Particles() {
}

void Particles::Initialize(const ModelInfo& info) {
	if (info.modelData.size() > 1) {
#ifdef _DEBUG

		assert(false, "テスト用のやつなんでメッシュ1以上のやつはやらんといてください");

#endif // _DEBUG
	}

	const ModelData& modelData = info.modelData[0];

	// 頂点バッファはモデルのものをそのまま使う(コピーしない).
	mesh_.InitializeFromModelData(modelData);

	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = modelData.textureSrvHandlesGPU;
	if (modelData.materialData.textureFilePath == "") {
		textureHandle = TextureManager::GetInstance()->GetTextureInfo("white_template").textureSrvHandlesGPU;
	}

	InitializeCommon(textureHandle, modelData.materialData.matarial.color, modelData.materialData.matarial.uvTransform);
}

void Particles::Initialize(const TextureInfo& info) {
	// 板ポリ(effect_plane モデル)に指定したテクスチャを貼る.
	const ModelData& modelData = ModelManager::GetInstance()->GetModelInfo("effect_plane").modelData[0];

	mesh_.InitializeFromModelData(modelData);

	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = info.textureSrvHandlesGPU;
	if (modelData.materialData.textureFilePath == "") {
		textureHandle = TextureManager::GetInstance()->GetTextureInfo("white_template").textureSrvHandlesGPU;
	}

	InitializeCommon(textureHandle, modelData.materialData.matarial.color, modelData.materialData.matarial.uvTransform);
}

void Particles::Initialize(PrimitiveType type, const TextureInfo& info, const PrimitiveParam& param) {
	// Primitive図形(Rendererと同じ形状)をパーティクルとして使う.
	if (type == PrimitiveType::kBox) {
		// 箱は Renderer::DrawBox と同じく block_template モデルの頂点を使う.
		mesh_.InitializeFromModelData(ModelManager::GetInstance()->GetModelInfo("block_template").modelData[0]);
	} else {
		mesh_.Initialize(type, param);
	}

	InitializeCommon(info.textureSrvHandlesGPU, { 1.0f,1.0f,1.0f,1.0f }, Matrix4x4::Identity());

	// 大きさを設定しなくても見えるように、等倍にしておく.
	size_ = { 1.0f,1.0f,1.0f };
}

void Particles::InitializeCommon(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, const Vector4& color, const Matrix4x4& uvTransform) {
	isVisible_ = true;
	textureHandle_ = textureHandle;
	// 作り直した時は出ていたパーティクルを消す.
	particleData_.clear();

	// 【MaterialResourceを生成する】(2回目以降のInitializeでは作ってあるものを使い回す)
	if (!materialResource_) {
		materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
		materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	}
	materialData_->color = color;
	materialData_->lightingType = 0;
	materialData_->uvTransform = uvTransform;

	Transform uvTransformValue = materialData_->uvTransform.GetMatrixToTransform();
	uvTransform_.Initialize();
	uvTransform_.scale = uvTransformValue.scale;
	uvTransform_.rotate = uvTransformValue.rotate;
	uvTransform_.translate = uvTransformValue.translate;

	// 【インスタンシング用のResourceとSRVを生成する】
	// (2回目以降のInitializeでは作ってあるものを使い回す. SRVは解放できないので作り直すと減っていくため)
	if (!instancingResource_) {
		instancingResource_ = GameSystem::GetInstance()->CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(ParticleForGPU) * kNumMaxInstance);
		instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
		instancingSrvHandleGPU = SrvManager::GetInstance()->CreateSRVforStructureBuffer(instancingResource_.Get(), kNumMaxInstance, sizeof(ParticleForGPU));
	}

	for (uint32_t index = 0; index < kNumMaxInstance; index++) {
		instancingData_[index].WVP = Matrix4x4::Identity();
		instancingData_[index].World = Matrix4x4::Identity();
	}

	billboardMatrix_ = Matrix4x4::Identity();
	blendMode_ = BlendMode::kAdd;
	billboardType_ = BillboardType::kNone;
	moveType_ = Move::kNormal;

	colorNoAlpha_ = { 1.0f,1.0f,1.0f };
}

void Particles::MakeNewParticle(const Vector3& position) {
	MakeNewParticle(position, {0.0f,0.0f,0.0f});
}

void Particles::MakeNewParticle(const Vector3& position, const Vector3& blank){
	MakeNewParticle(position, Random::GetInstance()->RandomVector3({ -1.0f,-1.0f,-1.0f }, { 1.0f,1.0f,1.0f }), Random::GetInstance()->RandomFloat(1.0f, 3.0f),Random::GetInstance()->RandomVector3({ -1.0f,-1.0f,-1.0f }, { 1.0f,1.0f,1.0f }),blank);
}

void Particles::MakeNewParticle(const Vector3& position, const Vector3& velocity, float lifeTime, const Vector3& targetPosition, const Vector3& blank){
	ParticleData newParticleData;
	newParticleData.transform.Initialize();
	newParticleData.targetTransform.Initialize();
	newParticleData.transform.translate = position;
	newParticleData.transform.scale = size_;
	newParticleData.currentTime = 0;
	newParticleData.posBlank = blank;
	newParticleData.spawnColor = spawnColor_;

	// ノードで作った動きを使う時は、初期値もノードで決める.
	// 渡された 速度 / 寿命 / 目標地点(生成位置から見たローカル座標) はノードの"パーティクル"ノードの 初速 / 寿命 / 目標地点 になる.
	if (nodeMove_) {
		newParticleData.velocity = velocity;
		newParticleData.lifeTime = lifeTime;
		newParticleData.targetTransform.translate = targetPosition;
		nodeMove_->Spawn(newParticleData);
		particleData_.push_back(newParticleData);
		return;
	}

	switch (moveType_) {
	case Particles::Move::kFire:
		newParticleData.velocity = { 0.0f,Random::GetInstance()->RandomFloat(0.01f,0.5f),0.0f };
		newParticleData.color.InitializeColor();
		newParticleData.lifeTime = Random::GetInstance()->RandomFloat(1.0f, 3.0f);
		break;
	case Particles::Move::kSlash:
		newParticleData.color.InitializeColor();
		newParticleData.velocity = { 0.0f,0.0f,0.0f };
		newParticleData.lifeTime = 0.5f;
		break;
	case Particles::Move::kExplode:
		newParticleData.velocity = Random::GetInstance()->RandomVector3({ -1.0f,-1.0f,-1.0f }, { 1.0f,1.0f,1.0f });
		newParticleData.velocity = newParticleData.velocity.Normalize() * 3.0f;
		newParticleData.color = { 0.7f,0.9f,1.0f,1.0f };
		newParticleData.lifeTime = Random::GetInstance()->RandomFloat(1.0f, 3.0f);
		break;
	case Particles::Move::kExplodeMonochrome:
		newParticleData.velocity = Random::GetInstance()->RandomVector3({ -1.0f,-1.0f,-1.0f }, { 1.0f,1.0f,1.0f });
		newParticleData.velocity = newParticleData.velocity.Normalize() * 3.0f;
		newParticleData.color.InitializeColor();
		newParticleData.lifeTime = Random::GetInstance()->RandomFloat(1.0f, 3.0f);
		break;
	case Particles::Move::kCharge:
		newParticleData.velocity = Random::GetInstance()->RandomVector3({ -1.0f,-1.0f,-1.0f }, { 1.0f,1.0f,1.0f });
		newParticleData.velocity = newParticleData.transform.translate + (newParticleData.velocity.Normalize() * 10.0f);
		newParticleData.color.InitializeColor();
		newParticleData.lifeTime = Random::GetInstance()->RandomFloat(1.0f, 3.0f);
		newParticleData.targetTransform = newParticleData.transform;
		break;
	case Particles::Move::kNumber:
		newParticleData.velocity = newParticleData.transform.translate;
		newParticleData.color.InitializeColor();
		newParticleData.color.SetColorWithoutAlpha(colorNoAlpha_, 1.0f);
		newParticleData.lifeTime = 2.0f;
		newParticleData.targetTransform = newParticleData.transform;
		newParticleData.targetTransform.translate.y = newParticleData.transform.translate.y + 1.0f;
		break;
	case Particles::Move::kNodeMove:
		newParticleData.velocity = velocity;
		newParticleData.lifeTime = lifeTime;
		newParticleData.targetTransform.translate = targetPosition;
		newParticleData.color.InitializeColor();
		newParticleData.color.SetColorWithoutAlpha(colorNoAlpha_, 1.0f);
		break;
	case Particles::Move::kNormal:
	default:
		newParticleData.velocity = Random::GetInstance()->RandomVector3({ -1.0f,-1.0f,-1.0f }, { 1.0f,1.0f,1.0f });
		newParticleData.color.SetColorWithoutAlpha(Random::GetInstance()->RandomVector3({ 0.0f,0.0f,0.0f }, { 1.0f,1.0f,1.0f }), 1.0f);
		newParticleData.lifeTime = Random::GetInstance()->RandomFloat(1.0f, 3.0f);
		break;
	}

	particleData_.push_back(newParticleData);
}

void Particles::MakeNewParticle(const Transform& transform) {
	MakeNewParticle(transform.GetAffineMatrix().GetMatrixToTranslate());
}

void Particles::Update() {
	if (nodeMove_) {
		const float deltaTime = DeltaTime::GetInstance()->GetDeltaTime();
		for (ParticleData& particle : particleData_) {
			nodeMove_->Update(particle, deltaTime);
		}
		return;
	}

	switch (moveType_){
	case Particles::Move::kNormal:
	case Particles::Move::kFire:
	case Particles::Move::kSlash:
		MoveNormal();
		break;
	case Particles::Move::kExplode:
	case Particles::Move::kExplodeMonochrome:
		MoveExplode();
		break;
	case Particles::Move::kCharge:
		MoveCharge();
		break;
	case Particles::Move::kNumber:
		MoveNumber();
		break;
	}	
}

void Particles::MoveNormal(){
	for (ParticleData& particle : particleData_) {
		particle.transform.translate += particle.velocity * DeltaTime::GetInstance()->GetDeltaTime();
		particle.currentTime += DeltaTime::GetInstance()->GetDeltaTime();
		particle.color.w = Easing(1.0f, 0.0f, particle.currentTime, particle.lifeTime, EaseType::kConstant);
	}
}

void Particles::MoveExplode(){
	for (ParticleData& particle : particleData_) {
		particle.transform.translate += Easing(particle.velocity, {0.0f,0.0f,0.0f}, particle.currentTime, particle.lifeTime, EaseType::kConstant) * DeltaTime::GetInstance()->GetDeltaTime();
		particle.currentTime += DeltaTime::GetInstance()->GetDeltaTime();
		particle.color.w = Easing(1.0f, 0.0f, particle.currentTime, particle.lifeTime, EaseType::kConstant);
	}
}

void Particles::MoveCharge(){
	for (ParticleData& particle : particleData_) {
		particle.currentTime += DeltaTime::GetInstance()->GetDeltaTime();
		particle.transform.translate = Easing(particle.velocity, particle.targetTransform.translate, particle.currentTime, particle.lifeTime, EaseType::kEaseIn);
		particle.color.w = Easing(1.0f, 0.0f, particle.currentTime, particle.lifeTime, EaseType::kConstant);
	}
}

void Particles::MoveNumber(){
	for (ParticleData& particle : particleData_) {
		particle.currentTime += DeltaTime::GetInstance()->GetDeltaTime();
		particle.transform.translate = Easing(particle.velocity, particle.targetTransform.translate, particle.currentTime, particle.lifeTime, EaseType::kEaseOut);
		particle.color.w = Easing(1.0f, 0.0f, particle.currentTime, particle.lifeTime, EaseType::kEaseIn);
	}
}

void Particles::CheckCollision(const Field& field) {
	for (std::list<ParticleData>::iterator particleIterator = particleData_.begin(); particleIterator != particleData_.end(); ++particleIterator) {
		if (Collision::AABBToPoint(field.GetArea(), (*particleIterator).transform.translate)) {
			(*particleIterator).velocity += field.GetAcceleration() * DeltaTime::GetInstance()->GetDeltaTime();
		}
	}
}


void Particles::Draw() {
	if (!isVisible_) {
		return;
	}

	numInstance_ = 0;

	billboardMatrix_ = Camera::GetInstance()->GetMatrix();
	billboardMatrix_.matrix[3][0] = 0.0f;
	billboardMatrix_.matrix[3][1] = 0.0f;
	billboardMatrix_.matrix[3][2] = 0.0f;
	Transform transformTemp;
	//float cameraRotate_ = Camera::GetInstance()->GetRotate().y - Radian(180.0f);
	Vector3 temp = Camera::GetInstance()->GetPosition();
	float cameraRotate_ = 0.0f;
	Vector3 a;

	for (std::list<ParticleData>::iterator particleIterator = particleData_.begin(); particleIterator != particleData_.end();) {
		if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
			particleIterator = particleData_.erase(particleIterator);
			continue;
		}
		cameraRotate_ = std::atan2((*particleIterator).transform.translate.x - temp.x,(*particleIterator).transform.translate.z - temp.z);

		if (moveType_ == Move::kNumber) {
			numInstance_ = numInstance_;
		}

		if (numInstance_ < kNumMaxInstance) {

			Matrix4x4 worldMatrix;
			Matrix4x4 bill;
			bill = Matrix4x4::Identity();
			bill *= Matrix4x4::MakeRotateYMatrix(Radian(180.0f));
			worldMatrix.Identity();
			transformTemp = (*particleIterator).transform;
			a = Matrix4x4::MakeRotateYMatrix(cameraRotate_).TransformNomal((*particleIterator).posBlank);
			transformTemp.translate = (*particleIterator).transform.translate + a;

			switch (billboardType_) {
			case BillboardType::kAllAxis:
			case BillboardType::kOnlyX:
			case BillboardType::kOnlyY:
			case BillboardType::kOnlyZ:
				worldMatrix = (*particleIterator).transform.GetScaleMatrix() * billboardMatrix_ * transformTemp.GetTranslateMatrix();
				//worldMatrix *= Matrix4x4::MakeTranslateMatrix((*particleIterator).posBlank);
				//worldMatrix *= bill;
				break;
			default:
				worldMatrix = (*particleIterator).transform.GetAffineMatrix();
				break;
			}


			instancingData_[numInstance_].World = worldMatrix;
			instancingData_[numInstance_].WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);
			// 動きで決まる色に、出現時の色を掛ける.
			const Vector4& color = (*particleIterator).color;
			const Vector4& spawnColor = (*particleIterator).spawnColor;
			instancingData_[numInstance_].color = { color.x * spawnColor.x, color.y * spawnColor.y, color.z * spawnColor.z, color.w * spawnColor.w };

			++numInstance_;
		}

		++particleIterator;
	}

	if (numInstance_ <= 0) {
		return;
	}

	materialData_->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform_);

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = GameSystem::GetInstance()->GetCommandList();

	/*=============================================================
	三角形の描画のコマンド.
	=============================================================*/
	GameSystem::GetInstance()->SetParticlePipeline(blendMode_);

	commandList->IASetVertexBuffers(0, 1, &mesh_.GetVertexBufferView()); // VBVを設定.
	// Primitive図形などインデックスを使うメッシュはIBVも設定する.
	if (mesh_.UseIndex()) {
		commandList->IASetIndexBuffer(mesh_.GetIndexBufferView());
	}
	// 形状を設定。PS0に設定しているものとはまた別。同じものを設定すると考えておけば良い.
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// CBufferの場所を設定.
	// マテリアル用のCBufferの場所.
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// WVP用のCBufferの場所.
	//commandList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(1, instancingSrvHandleGPU);
	// SRVのDescriptorTableの先頭の設定。2はrootParameter[2]である.
	commandList->SetGraphicsRootDescriptorTable(2, textureHandle_);
	// 描画！(DrawCall/ドローコール)。numInstance_個のインスタンスをまとめて描画する.
	if (mesh_.UseIndex()) {
		commandList->DrawIndexedInstanced(mesh_.GetDrawCount(), numInstance_, 0, 0, 0);
	} else {
		commandList->DrawInstanced(mesh_.GetDrawCount(), numInstance_, 0, 0);
	}
}

void Particles::SetBillboardType(BillboardType billboardType) {
	if (billboardType == billboardType_) {
		return;
	}

	billboardType_ = billboardType;
}

void Particles::SetMoveType(Move moveType){
	// ノード以外のMoveを指定したらノードの動きはやめる(kNodeMoveならノードの動きはそのまま).
	if (moveType != Move::kNodeMove) {
		nodeMove_.reset();
	}
	if (moveType == moveType_) {
		return;
	}

	moveType_ = moveType;
}

void Emitter::Initialize(const Transform& transform, uint32_t count, float frequency) {
	shape_ = EmitterShape::kBox;
	transform_ = transform;
	count_ = count;
	frequency_ = frequency;
	frequencyTime_ = 0.0f;

	if (frequency < 0.0f) {
		useTimer_ = false;
	} else {
		useTimer_ = true;
	}
}

void Emitter::CreateParticle() {
	// 出すパーティクルが設定されていない / 消された.
	if (!particles_) {
		return;
	}
	for (uint32_t count = 0; count < count_; count++) {
		switch (shape_) {
		case EmitterShape::kBox:
			particles_->MakeNewParticle(transform_.translate + Random::GetInstance()->RandomVector3(-(transform_.scale / 2.0f), (transform_.scale / 2.0f)));
			break;
		case EmitterShape::kSphere:
			particles_->MakeNewParticle(transform_.translate + Random::GetInstance()->RandomCircleVector3(transform_.scale / 2.0f));
			break;
		}
	}
}

void Emitter::Update() {
	if (!useTimer_) {
		return;
	}

	frequencyTime_ += DeltaTime::GetInstance()->GetDeltaTime();

	if (frequency_ <= frequencyTime_) {
			CreateParticle();
		frequencyTime_ -= frequency_;
	}
}

void Emitter::DebugDraw() {
#ifdef _DEBUG

	switch (shape_) {
	case EmitterShape::kBox:
		Renderer::GetInstance()->DrawBoxWireFrame(transform_, { 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f,1.0f });
		break;
	case EmitterShape::kSphere:
		Renderer::GetInstance()->DrawSphereWireFrame(transform_, { 1.0f,1.0f,1.0f,1.0f });
		break;
	}

#endif // _DEBUG
}
