#include "Particles.h"
#include "../SystemFile/DeltaTime.h"
#include "../Math/Random.h"
#include "../Renderer/Camera.h"
#include "../Math/Easing.h"
#include "Renderer.h"

Particles::~Particles() {
	delete vertexData;
	delete materialData_;
	delete instancingData_;
}

void Particles::Initialize(const ModelInfo& info) {
	// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)
	modelMax_ = static_cast<uint32_t>(info.modelData.size());

	if (modelMax_ > 1) {
#ifdef _DEBUG

		assert(false, "テスト用のやつなんでメッシュ1以上のやつはやらんといてください");

#endif // _DEBUG
	}

	isVisible_ = true;

	modelData_ = info.modelData[0];

	if (modelData_.materialData.textureFilePath == "") {
		modelData_.textureSrvHandlesGPU = TextureManager::GetInstance()->GetTextureInfo("white_template").textureSrvHandlesGPU;
		modelData_.textureSrvHandlesCPU = TextureManager::GetInstance()->GetTextureInfo("white_template").textureSrvHandlesCPU;
	}

	vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());

	// 【MaterialResourceを生成する】
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResource 
	materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む.
	//Material* materialData = nullptr;
	// 書き込むためのアドレスを取得.
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// 今回は赤を書き込んでみる
	materialData_->color = modelData_.materialData.matarial.color;
	materialData_->lightingType = 0;
	materialData_->uvTransform = modelData_.materialData.matarial.uvTransform;

	// 【TransformationMatrix】
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する.
	//Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource 
	//wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む.
	//TransformationMatrix* wvpData = nullptr;
	// 書き込むためのアドレスを取得.
	//wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	// 単位行列を書き込んでおく.
	//wvpData_->WVP = Matrix4x4::Identity();
	//wvpData_->World = Matrix4x4::Identity();
	uvTransform_.Initialize();
	uvTransform_.scale = materialData_->uvTransform.GetMatrixToTransform().scale;
	uvTransform_.rotate = materialData_->uvTransform.GetMatrixToTransform().rotate;
	uvTransform_.translate = materialData_->uvTransform.GetMatrixToTransform().translate;

	// 【VertexBufferViewを作成する】

	// 頂点バッファビューを作成する.
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// リソースの先頭のアドレスから使う.
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ.(多分ここは他の場所でも変えられる。Rendererから頂点数取ってきて代入とかできそう)
	//vertexBufferView.SizeInBytes = sizeof(VertexData) * kSubdivision * kSubdivision * 4;
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	// 1頂点あたりのサイズ.
	vertexBufferView_.StrideInBytes = sizeof(VertexData);


	// 【Resourceにデータを書き込む】

	// 頂点リソースにデータを書き込む.
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得.
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());


	instancingResource_ = GameSystem::GetInstance()->CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(ParticleForGPU) * kNumMaxInstance);

	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));

	for (uint32_t index = 0; index < kNumMaxInstance; index++) {
		instancingData_[index].WVP = Matrix4x4::Identity();
		instancingData_[index].World = Matrix4x4::Identity();
	}

	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = kNumMaxInstance;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);

	instancingSrvHandleCPU = GameSystem::GetInstance()->GetCPUDescriptorHandle(GameSystem::GetInstance()->GetSrvDescriptorHeap(), GameSystem::GetInstance()->GetDescriptorSizeSRV(), 50);
	instancingSrvHandleGPU = GameSystem::GetInstance()->GetGPUDescriptorHandle(GameSystem::GetInstance()->GetSrvDescriptorHeap(), GameSystem::GetInstance()->GetDescriptorSizeSRV(), 50);

	GameSystem::GetInstance()->SrvDescriptorHeapNumIncrement();

	GameSystem::GetInstance()->GetDevice()->CreateShaderResourceView(instancingResource_.Get(), &instancingSrvDesc, instancingSrvHandleCPU);

	blendMode_ = BlendMode::kAdd;
}

void Particles::Initialize(const TextureInfo& info) {
	// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)
	ModelInfo modelInfo = ModelManager::GetInstance()->GetModelInfo("effect_plane");
	modelMax_ = static_cast<uint32_t>(modelInfo.modelData.size());

	if (modelMax_ > 1) {
#ifdef _DEBUG

		assert(false, "テスト用のやつなんでメッシュ1以上のやつはやらんといてください");

#endif // _DEBUG
	}

	isVisible_ = true;

	modelData_ = modelInfo.modelData[0];
	modelData_.textureSrvHandlesGPU = info.textureSrvHandlesGPU;

	if (modelData_.materialData.textureFilePath == "") {
		modelData_.textureSrvHandlesGPU = TextureManager::GetInstance()->GetTextureInfo("white_template").textureSrvHandlesGPU;
		modelData_.textureSrvHandlesCPU = TextureManager::GetInstance()->GetTextureInfo("white_template").textureSrvHandlesCPU;
	}

	vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());

	// 【MaterialResourceを生成する】
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResource 
	materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む.
	//Material* materialData = nullptr;
	// 書き込むためのアドレスを取得.
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// 今回は赤を書き込んでみる
	materialData_->color = modelData_.materialData.matarial.color;
	materialData_->lightingType = 0;
	materialData_->uvTransform = modelData_.materialData.matarial.uvTransform;

	// 【TransformationMatrix】
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する.
	//Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource 
	//wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む.
	//TransformationMatrix* wvpData = nullptr;
	// 書き込むためのアドレスを取得.
	//wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	// 単位行列を書き込んでおく.
	//wvpData_->WVP = Matrix4x4::Identity();
	//wvpData_->World = Matrix4x4::Identity();
	uvTransform_.Initialize();
	uvTransform_.scale = materialData_->uvTransform.GetMatrixToTransform().scale;
	uvTransform_.rotate = materialData_->uvTransform.GetMatrixToTransform().rotate;
	uvTransform_.translate = materialData_->uvTransform.GetMatrixToTransform().translate;

	// 【VertexBufferViewを作成する】

	// 頂点バッファビューを作成する.
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// リソースの先頭のアドレスから使う.
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ.(多分ここは他の場所でも変えられる。Rendererから頂点数取ってきて代入とかできそう)
	//vertexBufferView.SizeInBytes = sizeof(VertexData) * kSubdivision * kSubdivision * 4;
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	// 1頂点あたりのサイズ.
	vertexBufferView_.StrideInBytes = sizeof(VertexData);


	// 【Resourceにデータを書き込む】

	// 頂点リソースにデータを書き込む.
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得.
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());


	instancingResource_ = GameSystem::GetInstance()->CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(ParticleForGPU) * kNumMaxInstance);

	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));

	for (uint32_t index = 0; index < kNumMaxInstance; index++) {
		instancingData_[index].WVP = Matrix4x4::Identity();
		instancingData_[index].World = Matrix4x4::Identity();
	}

	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = kNumMaxInstance;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);

	instancingSrvHandleCPU = GameSystem::GetInstance()->GetCPUDescriptorHandle(GameSystem::GetInstance()->GetSrvDescriptorHeap(), GameSystem::GetInstance()->GetDescriptorSizeSRV(), GameSystem::GetInstance()->GetSrvDescriptorHeapNum());
	instancingSrvHandleGPU = GameSystem::GetInstance()->GetGPUDescriptorHandle(GameSystem::GetInstance()->GetSrvDescriptorHeap(), GameSystem::GetInstance()->GetDescriptorSizeSRV(), GameSystem::GetInstance()->GetSrvDescriptorHeapNum());

	GameSystem::GetInstance()->GetDevice()->CreateShaderResourceView(instancingResource_.Get(), &instancingSrvDesc, instancingSrvHandleCPU);

	GameSystem::GetInstance()->SrvDescriptorHeapNumIncrement();

	//for (uint32_t index = 0; index < 10; index++) {
	//	MakeNewParticle();
	//}

	billboardMatrix_.Identity();
	blendMode_ = BlendMode::kAdd;
	billboardType_ = BillboardType::kNone;

	colorNoAlpha_ = { 1.0f,1.0f,1.0f };
}

void Particles::MakeNewParticle(const Vector3& position) {
	MakeNewParticle(position, {0.0f,0.0f,0.0f});
}

void Particles::MakeNewParticle(const Vector3& position, const Vector3& blank){
	ParticleData newParticleData;
	newParticleData.transform.Initialize();
	newParticleData.targetTransform.Initialize();
	newParticleData.transform.translate = position;
	newParticleData.transform.scale = size_;
	newParticleData.currentTime = 0;
	newParticleData.posBlank = blank;
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
			instancingData_[numInstance_].color = (*particleIterator).color;

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

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定.
	// 形状を設定。PS0に設定しているものとはまた別。同じものを設定すると考えておけば良い.
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// CBufferの場所を設定.
	// マテリアル用のCBufferの場所.
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// WVP用のCBufferの場所.
	//commandList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(1, instancingSrvHandleGPU);
	// SRVのDescriptorTableの先頭の設定。2はrootParameter[2]である.
	commandList->SetGraphicsRootDescriptorTable(2, modelData_.textureSrvHandlesGPU);
	// DirectionalLight用のCBufferの場所.
	//commandList->SetGraphicsRootConstantBufferView(3, DirectionalLight::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後.
	commandList->DrawInstanced(UINT(modelData_.vertices.size()), numInstance_, 0, 0);
}

void Particles::SetBillboardType(BillboardType billboardType) {
	if (billboardType == billboardType_) {
		return;
	}

	billboardType_ = billboardType;
}

void Particles::SetMoveType(Move moveType){
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
