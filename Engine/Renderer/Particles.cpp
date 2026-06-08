#include "Particles.h"
#include "../SystemFile/DeltaTime.h"
#include "../Math/Random.h"
#include "../Math/Easing.h"
#include "Renderer.h"

void Particles::Initialize(const ModelInfo& info) {
	// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)
	modelMax_ = static_cast<uint32_t>(info.modelData.size());

	if (modelMax_ > 1) {
		assert(false, "テスト用のやつなんでメッシュ1以上のやつはやらんといてください");
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

	GameSystem::GetInstance()->GetDevice()->CreateShaderResourceView(instancingResource_.Get(), &instancingSrvDesc, instancingSrvHandleCPU);

	blendMode_ = BlendMode::kAdd;
}

void Particles::Initialize(const TextureInfo& info) {
	// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)
	ModelInfo modelInfo = ModelManager::GetInstance()->GetModelInfo("effect_plane");
	modelMax_ = static_cast<uint32_t>(modelInfo.modelData.size());

	if (modelMax_ > 1) {
		assert(false, "テスト用のやつなんでメッシュ1以上のやつはやらんといてください");
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

	instancingSrvHandleCPU = GameSystem::GetInstance()->GetCPUDescriptorHandle(GameSystem::GetInstance()->GetSrvDescriptorHeap(), GameSystem::GetInstance()->GetDescriptorSizeSRV(), 50);
	instancingSrvHandleGPU = GameSystem::GetInstance()->GetGPUDescriptorHandle(GameSystem::GetInstance()->GetSrvDescriptorHeap(), GameSystem::GetInstance()->GetDescriptorSizeSRV(), 50);

	GameSystem::GetInstance()->GetDevice()->CreateShaderResourceView(instancingResource_.Get(), &instancingSrvDesc, instancingSrvHandleCPU);

	//for (uint32_t index = 0; index < 10; index++) {
	//	MakeNewParticle();
	//}

	billboardMatrix_.Identity();
	blendMode_ = BlendMode::kAdd;
	billboardType_ = BillboardType::kNone;
}

void Particles::MakeNewParticle(const Vector3& position) {
	ParticleData newParticleData;
	newParticleData.transform.Initialize();
	newParticleData.transform.rotate.x = Radian(70.0f);
	newParticleData.transform.translate = position;
	newParticleData.velocity = Random::GetInstance()->RandomVector3({ -1.0f,-1.0f,-1.0f }, { 1.0f,1.0f,1.0f });
	newParticleData.color.SetColorWithoutAlpha(Random::GetInstance()->RandomVector3({ 0.0f,0.0f,0.0f }, { 1.0f,1.0f,1.0f }), 1.0f);
	newParticleData.currentTime = 0;
	newParticleData.lifeTime = Random::GetInstance()->RandomFloat(1.0f, 3.0f);

	particleData_.push_back(newParticleData);
}

void Particles::MakeNewParticle(const Transform& transform) {
	Transform particleTransform = transform;
	particleTransform.scale = { 1.0f,1.0f,1.0f };

	ParticleData newParticleData;
	newParticleData.transform.Initialize();
	newParticleData.transform.rotate.x = Radian(70.0f);
	newParticleData.transform.translate = particleTransform.GetAffineMatrix().GetMatrixToTranslate();
	newParticleData.velocity = Random::GetInstance()->RandomVector3({ -1.0f,-1.0f,-1.0f }, { 1.0f,1.0f,1.0f });
	newParticleData.color.SetColorWithoutAlpha(Random::GetInstance()->RandomVector3({ 0.0f,0.0f,0.0f }, { 1.0f,1.0f,1.0f }), 1.0f);
	newParticleData.currentTime = 0;
	newParticleData.lifeTime = Random::GetInstance()->RandomFloat(1.0f, 3.0f);

	particleData_.push_back(newParticleData);
}

void Particles::Update() {
	for (ParticleData& particle : particleData_) {
		particle.transform.translate += particle.velocity * DeltaTime::GetInstance()->GetDeltaTime();
		particle.currentTime += DeltaTime::GetInstance()->GetDeltaTime();
		particle.color.w = Easing(1.0f, 0.0f, particle.currentTime, particle.lifeTime, EaseType::kConstant);
	}

	//switch (billboardType_) {
	//case BillboardType::kAllAxis:
	//	billboardMatrix_ = Matrix4x4::MakeRotateMatrix(Camera::GetInstance()->GetTransform().rotate);
	//		break;
	//case BillboardType::kOnlyX:
	//	billboardMatrix_ = Matrix4x4::MakeRotateXMatrix(Camera::GetInstance()->GetTransform().rotate.x);
	//	break;
	//case BillboardType::kOnlyY:
	//	billboardMatrix_ = Matrix4x4::MakeRotateYMatrix(Camera::GetInstance()->GetTransform().rotate.y);
	//	break;
	//case BillboardType::kOnlyZ:
	//	billboardMatrix_ = Matrix4x4::MakeRotateZMatrix(Camera::GetInstance()->GetTransform().rotate.z);
	//	break;
	//}

}

void Particles::CheckCollision(const Field& field) {
	for (std::list<ParticleData>::iterator particleIterator = particleData_.begin(); particleIterator != particleData_.end();++particleIterator) {
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

	for (std::list<ParticleData>::iterator particleIterator = particleData_.begin(); particleIterator != particleData_.end();) {
		if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
			particleIterator = particleData_.erase(particleIterator);
			continue;
		}

		if (numInstance_ < kNumMaxInstance) {

			Matrix4x4 worldMatrix;
			Matrix4x4 bill;
			bill = Matrix4x4::Identity();
			bill *= Matrix4x4::MakeRotateYMatrix(Radian(180.0f));
			worldMatrix.Identity();

			switch (billboardType_) {
			case BillboardType::kAllAxis:
			case BillboardType::kOnlyX:
			case BillboardType::kOnlyY:
			case BillboardType::kOnlyZ:
				worldMatrix = (*particleIterator).transform.GetScaleMatrix() * billboardMatrix_ * (*particleIterator).transform.GetTranslateMatrix();
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

void Emitter::Initialize(const Transform& transform, uint32_t count, float frequency) {
	transform_ = transform;
	count_ = count;
	frequency_ = frequency;
	frequencyTime_ = 0.0f;
}

void Emitter::CreateParticle() {
	for (uint32_t count = 0; count < count_; count++) {
		particles_->MakeNewParticle(transform_.translate + Random::GetInstance()->RandomVector3(-(transform_.scale / 2.0f), (transform_.scale / 2.0f)));
	}
}

void Emitter::Update() {
	frequencyTime_ += DeltaTime::GetInstance()->GetDeltaTime();

	GameSystem::Log(std::format("frequencyTime_ : {}\n",frequencyTime_));

	if (frequency_ <= frequencyTime_) {
		CreateParticle();
		frequencyTime_ -= frequency_;
	}
}

void Emitter::DebugDraw() {
#ifdef _DEBUG

	Renderer::GetInstance()->DrawBoxWireFrame(transform_, { 1.0f,1.0f,1.0f }, {1.0f,1.0f,1.0f,1.0f});

#endif // _DEBUG
}
