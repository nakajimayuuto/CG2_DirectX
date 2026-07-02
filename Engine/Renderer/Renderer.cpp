#include "Renderer.h"
#include <vector>
Renderer::Model::~Model() {
	materialData_.clear();

	wvpData_.clear();

	isVisible_.clear();

	modelData_.clear();

	vertexResource_.clear();

	materialResource_.clear();

	wvpResource_.clear();

	vertexBufferView_.clear();

	uvTransform_.clear();

}

void Renderer::Model::Initialize(const ModelInfo& info) {
	modelMax_ = static_cast<uint32_t>(info.modelData.size());

	blendMode_ = BlendMode::kNormal;

	materialData_.resize(modelMax_);
	isVisible_.resize(modelMax_);
	modelData_.resize(modelMax_);
	wvpData_.resize(modelMax_);
	vertexResource_.resize(modelMax_);
	materialResource_.resize(modelMax_);
	uvTransform_.resize(modelMax_);
	wvpResource_.resize(modelMax_);
	vertexBufferView_.resize(modelMax_);

	for (uint32_t i = 0; i < modelMax_; i++) {
		// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)

		isVisible_[i] = true;

		modelData_[i] = info.modelData[i];

		if (modelData_[i].materialData.textureFilePath == "") {
			modelData_[i].textureSrvHandlesGPU = TextureManager::GetInstance()->GetTextureInfo("white_template").textureSrvHandlesGPU;
		}

		vertexResource_[i] = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * modelData_[i].vertices.size());

		// 【MaterialResourceを生成する】
		// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
		//Microsoft::WRL::ComPtr<ID3D12Resource> materialResource 
		materialResource_[i] = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
		// マテリアルにデータを書き込む.
		//Material* materialData = nullptr;
		// 書き込むためのアドレスを取得.
		materialResource_[i]->Map(0, nullptr, reinterpret_cast<void**>(&materialData_[i]));
		// 今回は赤を書き込んでみる
		materialData_[i]->color = modelData_[i].materialData.matarial.color;
		materialData_[i]->lightingType = static_cast<uint32_t>(LightingType::kHalfLambert);
		materialData_[i]->uvTransform = modelData_[i].materialData.matarial.uvTransform;

		// 【TransformationMatrix】
		// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する.
		//Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource 
		wvpResource_[i] = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
		// データを書き込む.
		//TransformationMatrix* wvpData = nullptr;
		// 書き込むためのアドレスを取得.
		wvpResource_[i]->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_[i]));
		// 単位行列を書き込んでおく.
		wvpData_[i]->WVP = Matrix4x4::Identity();
		wvpData_[i]->World = Matrix4x4::Identity();
		uvTransform_[i].Initialize();
		uvTransform_[i].scale = materialData_[i]->uvTransform.GetMatrixToTransform().scale;
		uvTransform_[i].rotate = materialData_[i]->uvTransform.GetMatrixToTransform().rotate;
		uvTransform_[i].translate = materialData_[i]->uvTransform.GetMatrixToTransform().translate;

		// 【VertexBufferViewを作成する】

		// 頂点バッファビューを作成する.
		//D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
		// リソースの先頭のアドレスから使う.
		vertexBufferView_[i].BufferLocation = vertexResource_[i]->GetGPUVirtualAddress();
		// 使用するリソースのサイズは頂点3つ分のサイズ.(多分ここは他の場所でも変えられる。Rendererから頂点数取ってきて代入とかできそう)
		//vertexBufferView.SizeInBytes = sizeof(VertexData) * kSubdivision * kSubdivision * 4;
		vertexBufferView_[i].SizeInBytes = UINT(sizeof(VertexData) * modelData_[i].vertices.size());
		// 1頂点あたりのサイズ.
		vertexBufferView_[i].StrideInBytes = sizeof(VertexData);


		// 【Resourceにデータを書き込む】

		// 頂点リソースにデータを書き込む.
		VertexData* vertexData = nullptr;
		// 書き込むためのアドレスを取得.
		vertexResource_[i]->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
		memcpy(vertexData, modelData_[i].vertices.data(), sizeof(VertexData) * modelData_[i].vertices.size());
	}
}

void Renderer::Model::Draw(const Transform& transform) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (!isVisible_[i]) {
			return;
		}

		Matrix4x4 worldMatrix = transform.GetAffineMatrix();

		wvpData_[i]->World = worldMatrix;
		wvpData_[i]->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);

		materialData_[i]->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform_[i]);

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = GameSystem::GetInstance()->GetCommandList();

		GameSystem::GetInstance()->DrawCommand(
			blendMode_,
			&vertexBufferView_[i],
			nullptr,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			materialResource_[i],
			wvpResource_[i],
			modelData_[i].textureSrvHandlesGPU,
			UINT(modelData_[i].vertices.size())
		);
	}
}

void Renderer::Model::SetIsVisible(bool isVisible) {
	if (modelMax_ == 1) {
		isVisible_[0] = isVisible;
		return;
	}

	for (uint32_t i = 0; i < modelMax_; i++) {
		isVisible_[i] = isVisible;
	}
}

void Renderer::Model::SetIsVisible(bool isVisible, const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		isVisible_[i] = isVisible;
		break;
	}
};

bool Renderer::Model::GetIsVisible() {
	if (modelMax_ == 1) {
		return isVisible_[0];
	}
};

bool Renderer::Model::GetIsVisible(const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		return isVisible_[i];
	}
};

void Renderer::Model::ChangeTexture(const TextureInfo& info) {
	if (modelMax_ == 1) {
		modelData_[0].textureSrvHandlesGPU = info.textureSrvHandlesGPU;
		return;
	}

	for (uint32_t i = 0; i < modelMax_; i++) {
		modelData_[i].textureSrvHandlesGPU = info.textureSrvHandlesGPU;
	}
};

void Renderer::Model::ChangeTexture(const TextureInfo& info, const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		modelData_[i].textureSrvHandlesGPU = info.textureSrvHandlesGPU;
		break;
	}
};

void Renderer::Model::SetColor(Vector4 color) {
	if (modelMax_ == 1) {
		materialData_[0]->color = color;
		materialResource_[0]->Map(0, nullptr, reinterpret_cast<void**>(&materialData_[0]));
		return;
	}

	for (uint32_t i = 0; i < modelMax_; i++) {
		materialData_[i]->color = color;
		materialResource_[i]->Map(0, nullptr, reinterpret_cast<void**>(&materialData_[i]));
	}
};
void Renderer::Model::SetColor(Vector4 color, const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		materialData_[i]->color = color;
		break;
	}
};

Vector4 Renderer::Model::GetColor() {
	return materialData_[0]->color;
};
Vector4 Renderer::Model::GetColor(const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		return materialData_[i]->color;
	}
};

void Renderer::Model::SetUvTransform(const Transform& uvTransform) {
	if (modelMax_ == 1) {
		uvTransform_[0] = uvTransform;
		return;
	}

	for (uint32_t i = 0; i < modelMax_; i++) {
		uvTransform_[i] = uvTransform;
	}
};
void Renderer::Model::SetUvTransform(const Transform& uvTransform, const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		uvTransform_[i] = uvTransform;
		break;
	}
}

Transform Renderer::Model::GetUvTransform() {
	return uvTransform_[0];
};
Transform Renderer::Model::GetUvTransform(const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		return uvTransform_[i];
	}
};

void Renderer::Model::SetLightingType(LightingType type) {
	if (modelMax_ == 1) {
		materialData_[0]->lightingType = static_cast<int32_t>(type);
		return;
	}

	for (uint32_t i = 0; i < modelMax_; i++) {
		materialData_[i]->lightingType = static_cast<int32_t>(type);
	}
};
void Renderer::Model::SetLightingType(LightingType type, const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		materialData_[i]->lightingType = static_cast<int32_t>(type);
		break;
	}
};

Renderer::LightingType Renderer::Model::GetLightingType() {
	return static_cast<LightingType>(materialData_[0]->lightingType);
};
Renderer::LightingType Renderer::Model::GetLightingType(const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		return static_cast<LightingType>(materialData_[i]->lightingType);
	}
};

void Renderer::ModelSphere::Initialize(TextureInfo info) {
	blendMode_ = BlendMode::kNormal;

	isVisible_ = true;

	textureInfo_ = info;
	// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)
	vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * kSubdivision_ * kSubdivision_ * 6);

	// 【MaterialResourceを生成する】
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
	materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む.
	// 書き込むためのアドレスを取得.
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// 今回は赤を書き込んでみる
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->lightingType = static_cast<uint32_t>(LightingType::kHalfLambert);
	materialData_->uvTransform = Matrix4x4::Identity();

	// 【TransformationMatrix】
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する.
	wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む.
	// 書き込むためのアドレスを取得.
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	// 単位行列を書き込んでおく.
	wvpData_->WVP = Matrix4x4::Identity();
	wvpData_->World = Matrix4x4::Identity();


	// 【VertexBufferViewを作成する】

	// 頂点バッファビューを作成する.
	// リソースの先頭のアドレスから使う.
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ.(多分ここは他の場所でも変えられる。Rendererから頂点数取ってきて代入とかできそう)
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * kSubdivision_ * kSubdivision_ * 4);
	// 1頂点あたりのサイズ.
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 【IndexResourceを生成する】
	// 実際に頂点リソースを作る.
	indexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(uint32_t) * kSubdivision_ * kSubdivision_ * 6);

	// 頂点バッファビューを作成する.
	// リソースの先頭のアドレスから使う.
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ.
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * kSubdivision_ * kSubdivision_ * 6;
	// インデックスはuint32_tとする.
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;


	// 【Resourceにデータを書き込む】

	// 頂点リソースにデータを書き込む.
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得.
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// インデックスリソースにデータを書き込む.
	uint32_t* indexData = nullptr;
	// 書き込むためのアドレスを取得.
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));


	// スフィアの描画プログラム.(いつかRendererに入れる)
	const float kLonEvery = std::numbers::pi_v<float> *2.0f / kSubdivision_;
	const float kLatEvery = std::numbers::pi_v<float> / kSubdivision_;

	for (uint32_t latIndex = 0; latIndex < kSubdivision_; latIndex++) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision_; lonIndex++) {
			uint32_t start = (latIndex * kSubdivision_ + lonIndex) * 4;
			uint32_t indexStart = (latIndex * kSubdivision_ + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;

			float u = static_cast<float>(lonIndex) / static_cast<float>(kSubdivision_);
			float v = 1.0f - static_cast<float>(latIndex) / static_cast<float>(kSubdivision_);

			vertexData[start].position = { cos(lat) * cos(lon),sin(lat),cos(lat) * sin(lon) ,1.0f };
			vertexData[start].texcoord = { u - 1.0f / static_cast<float>(kSubdivision_),v };
			vertexData[start + 1].position = { cos(lat + std::numbers::pi_v<float> / kSubdivision_) * cos(lon),sin(lat + std::numbers::pi_v<float> / kSubdivision_),cos(lat + std::numbers::pi_v<float> / kSubdivision_) * sin(lon) ,1.0f };
			vertexData[start + 1].texcoord = { u - 1.0f / static_cast<float>(kSubdivision_) ,v - 1.0f / static_cast<float>(kSubdivision_) };
			vertexData[start + 2].position = { cos(lat) * cos(lon + std::numbers::pi_v<float> *2.0f / kSubdivision_),sin(lat),cos(lat) * sin(lon + std::numbers::pi_v<float> *2.0f / kSubdivision_) ,1.0f };
			vertexData[start + 2].texcoord = { u ,v };
			vertexData[start + 3].position = { cos(lat + std::numbers::pi_v<float> / kSubdivision_) * cos(lon + std::numbers::pi_v<float> *2.0f / kSubdivision_),sin(lat + std::numbers::pi_v<float> / kSubdivision_),cos(lat + std::numbers::pi_v<float> / kSubdivision_) * sin(lon + std::numbers::pi_v<float> *2.0f / kSubdivision_),1.0f };
			vertexData[start + 3].texcoord = { u ,v - 1.0f / static_cast<float>(kSubdivision_) };

			indexData[indexStart] = start;
			indexData[indexStart + 1] = start + 1;
			indexData[indexStart + 2] = start + 2;
			indexData[indexStart + 3] = start + 1;
			indexData[indexStart + 4] = start + 3;
			indexData[indexStart + 5] = start + 2;


			for (uint32_t i = 0; i < 4; i++) {
				vertexData[start + i].normal.x = vertexData[start + i].position.x;
				vertexData[start + i].normal.y = vertexData[start + i].position.y;
				vertexData[start + i].normal.z = vertexData[start + i].position.z;
			}
		}
	}
}

void Renderer::ModelSphere::Draw(const Transform& transform) {
	if (!isVisible_) {
		return;
	}

	Matrix4x4 worldMatrix = transform.GetAffineMatrix();

	wvpData_->World = worldMatrix;
	wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);
	/*=============================================================
	三角形の描画のコマンド.
	=============================================================*/
	GameSystem::GetInstance()->SetPipeline(blendMode_);

	GameSystem::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定.
	GameSystem::GetInstance()->GetCommandList()->IASetIndexBuffer(&indexBufferView_); // IBVを設定.
	// 形状を設定。PS0に設定しているものとはまた別。同じものを設定すると考えておけば良い.
	GameSystem::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// CBufferの場所を設定.
	// マテリアル用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// WVP用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭の設定。2はrootParameter[2]である.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureInfo_.textureSrvHandlesGPU);
	// DirectionalLight用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(3, DirectionalLight::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後.
	GameSystem::GetInstance()->GetCommandList()->DrawIndexedInstanced(kSubdivision_ * kSubdivision_ * 6, 1, 0, 0, 0);
}

void Renderer::ModelBox::Initialize(const TextureInfo& info) {
	isVisible_ = true;
	uvTransform_.Initialize();
	modelData_ = ModelManager::GetInstance()->GetModelInfo("block_template").modelData[0];
	modelData_.textureSrvHandlesGPU = info.textureSrvHandlesGPU;
	// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)
	vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());

	// 【MaterialResourceを生成する】
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
	materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む.
	// 書き込むためのアドレスを取得.
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// 今回は赤を書き込んでみる
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->lightingType = static_cast<uint32_t>(LightingType::kHalfLambert);
	materialData_->uvTransform = Matrix4x4::Identity();

	// 【TransformationMatrix】
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する.
	wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む.
	// 書き込むためのアドレスを取得.
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	// 単位行列を書き込んでおく.
	wvpData_->WVP = Matrix4x4::Identity();
	wvpData_->World = Matrix4x4::Identity();

	// 【VertexBufferViewを作成する】
	// 頂点バッファビューを作成する.
	// リソースの先頭のアドレスから使う.
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ.(多分ここは他の場所でも変えられる。Rendererから頂点数取ってきて代入とかできそう)
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	// 1頂点あたりのサイズ.
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 【Resourceにデータを書き込む】
	// 頂点リソースにデータを書き込む.
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得.
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
}

void Renderer::ModelBox::Initialize() {
	blendMode_ = BlendMode::kNormal;

	isVisible_ = true;
	uvTransform_.Initialize();
	modelData_ = ModelManager::GetInstance()->GetModelInfo("block_template").modelData[0];
	// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)
	vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());

	// 【MaterialResourceを生成する】
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
	materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む.
	// 書き込むためのアドレスを取得.
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// 今回は赤を書き込んでみる
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->lightingType = static_cast<uint32_t>(LightingType::kHalfLambert);
	materialData_->uvTransform = Matrix4x4::Identity();

	// 【TransformationMatrix】
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する.
	wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む.
	// 書き込むためのアドレスを取得.
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	// 単位行列を書き込んでおく.
	wvpData_->WVP = Matrix4x4::Identity();
	wvpData_->World = Matrix4x4::Identity();

	// 【VertexBufferViewを作成する】
	// 頂点バッファビューを作成する.
	// リソースの先頭のアドレスから使う.
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ.(多分ここは他の場所でも変えられる。Rendererから頂点数取ってきて代入とかできそう)
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	// 1頂点あたりのサイズ.
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 【Resourceにデータを書き込む】
	// 頂点リソースにデータを書き込む.
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得.
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
}

void Renderer::ModelBox::Draw(const Transform& transform) {
	if (!isVisible_) {
		return;
	}

	Matrix4x4 worldMatrix = transform.GetAffineMatrix();

	wvpData_->World = worldMatrix;
	wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);

	materialData_->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform_);

	/*=============================================================
	三角形の描画のコマンド.
	=============================================================*/
	GameSystem::GetInstance()->SetPipeline(blendMode_);

	GameSystem::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定.
	// 形状を設定。PS0に設定しているものとはまた別。同じものを設定すると考えておけば良い.
	GameSystem::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// CBufferの場所を設定.
	// マテリアル用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// WVP用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭の設定。2はrootParameter[2]である.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(2, modelData_.textureSrvHandlesGPU);
	// DirectionalLight用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(3, DirectionalLight::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後.
	GameSystem::GetInstance()->GetCommandList()->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);

}

void Renderer::Sprite::Initialize(TextureInfo info) {
	isVisible_ = true;

	blendMode_ = BlendMode::kNormal;
	uvTransform_.Initialize();

	textureInfo_ = info;
	/*=============================================================
	Sprite用のResourceとView.
	=============================================================*/
	// 【VertexResourceを生成する】
	// 実際に頂点リソースを作る.
	vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * 4);

	// 頂点バッファビューを作成する.
	// リソースの先頭のアドレスから使う.
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ.
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点あたりのサイズ.
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 【IndexResourceを生成する】
	// 実際に頂点リソースを作る.
	indexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(uint32_t) * 6);

	// 頂点バッファビューを作成する.
	// リソースの先頭のアドレスから使う.
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ.
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする.
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;


	// 【MaterialResourceを生成する】
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite
	materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む.
	// 書き込むためのアドレスを取得.
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// 今回は赤を書き込んでみる
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->lightingType = static_cast<uint32_t>(LightingType::kAspectNone);
	materialData_->uvTransform = Matrix4x4::Identity();

	// 【TransformationMatrix】
	//Sprite用のTransformationMatrixを作る。Matrix4x4 1つ分のサイズを用意する.
	transformationMatrixResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む.
	// 書き込むためのアドレスを取得.
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	// 単位行列を書き込んでおく.
	transformationMatrixData_->WVP = Matrix4x4::Identity();
	transformationMatrixData_->World = Matrix4x4::Identity();

	// 【Resourceにデータを書き込む】

	// 頂点リソースにデータを書き込む.
	// 書き込むためのアドレスを取得.
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 1枚目の三角形.
	vertexData[0].position = { 0.0f,360.0f,0.0f,1.0f }; // 左下.
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[0].normal = { 0.0f,0.0f,-1.0f };
	vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f }; // 左上.
	vertexData[1].texcoord = { 0.0f,0.0f };
	vertexData[1].normal = { 0.0f,0.0f,-1.0f };
	vertexData[2].position = { 640.0f,360.0f,0.0f,1.0f }; // 右下.
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[2].normal = { 0.0f,0.0f,-1.0f };
	vertexData[3].position = { 640.0f,0.0f,0.0f,1.0f }; // 右上.
	vertexData[3].texcoord = { 1.0f,0.0f };
	vertexData[3].normal = { 0.0f,0.0f,-1.0f };

	// インデックスリソースにデータを書き込む.
	uint32_t* indexDataSprite = nullptr;
	// 書き込むためのアドレスを取得.
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	// 1枚目の三角形.
	indexDataSprite[0] = 0;
	indexDataSprite[1] = 1;
	indexDataSprite[2] = 2;
	indexDataSprite[3] = 1;
	indexDataSprite[4] = 3;
	indexDataSprite[5] = 2;

	size_ = { static_cast<float>(textureInfo_.width),static_cast<float>(textureInfo_.height) };
	AdaptationSize();
}

void Renderer::Sprite::Initialize() {
	isVisible_ = true;

	blendMode_ = BlendMode::kNormal;
	uvTransform_.Initialize();

	textureInfo_ = TextureManager::GetInstance()->GetTextureInfo("white_template");
	/*=============================================================
	Sprite用のResourceとView.
	=============================================================*/
	// 【VertexResourceを生成する】
	// 実際に頂点リソースを作る.
	vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * 4);

	// 頂点バッファビューを作成する.
	// リソースの先頭のアドレスから使う.
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ.
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点あたりのサイズ.
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 【IndexResourceを生成する】
	// 実際に頂点リソースを作る.
	indexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(uint32_t) * 6);

	// 頂点バッファビューを作成する.
	// リソースの先頭のアドレスから使う.
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ.
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする.
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;


	// 【MaterialResourceを生成する】
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite
	materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む.
	// 書き込むためのアドレスを取得.
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// 今回は赤を書き込んでみる
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->lightingType = static_cast<uint32_t>(LightingType::kAspectNone);
	materialData_->uvTransform = Matrix4x4::Identity();

	// 【TransformationMatrix】
	//Sprite用のTransformationMatrixを作る。Matrix4x4 1つ分のサイズを用意する.
	transformationMatrixResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む.
	// 書き込むためのアドレスを取得.
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	// 単位行列を書き込んでおく.
	transformationMatrixData_->WVP = Matrix4x4::Identity();
	transformationMatrixData_->World = Matrix4x4::Identity();

	// 【Resourceにデータを書き込む】

	// 頂点リソースにデータを書き込む.
	// 書き込むためのアドレスを取得.
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 1枚目の三角形.
	vertexData[0].position = { 0.0f,360.0f,0.0f,1.0f }; // 左下.
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[0].normal = { 0.0f,0.0f,-1.0f };
	vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f }; // 左上.
	vertexData[1].texcoord = { 0.0f,0.0f };
	vertexData[1].normal = { 0.0f,0.0f,-1.0f };
	vertexData[2].position = { 640.0f,360.0f,0.0f,1.0f }; // 右下.
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[2].normal = { 0.0f,0.0f,-1.0f };
	vertexData[3].position = { 640.0f,0.0f,0.0f,1.0f }; // 右上.
	vertexData[3].texcoord = { 1.0f,0.0f };
	vertexData[3].normal = { 0.0f,0.0f,-1.0f };

	// インデックスリソースにデータを書き込む.
	uint32_t* indexDataSprite = nullptr;
	// 書き込むためのアドレスを取得.
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	// 1枚目の三角形.
	indexDataSprite[0] = 0;
	indexDataSprite[1] = 1;
	indexDataSprite[2] = 2;
	indexDataSprite[3] = 1;
	indexDataSprite[4] = 3;
	indexDataSprite[5] = 2;

	size_ = { static_cast<float>(textureInfo_.width),static_cast<float>(textureInfo_.height) };
	AdaptationSize();
}

void Renderer::Sprite::Draw(const Transform& transform) {
	if (!isVisible_) {
		return;
	}

	Transform worldTransform = transform;

	worldTransform.translate.x = transform .translate.x - (size_.x / 2.0f);
	worldTransform.translate.y = transform .translate.y - (size_.y / 2.0f);

	Matrix4x4 worldMatrix = worldTransform.GetAffineMatrix();

	transformationMatrixData_->World = worldMatrix;
	transformationMatrixData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrixSprite(worldMatrix);

	materialData_->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform_);
	/*=============================================================
	三角形のSpriteの描画のコマンド.
	=============================================================*/
	// Spriteの描画。変更が必要なものだけ変更する.
	GameSystem::GetInstance()->DrawCommand(
		blendMode_,
		&vertexBufferView_,
		&indexBufferView_,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		materialResource_,
		transformationMatrixResource_,
		textureInfo_.textureSrvHandlesGPU,
		6
	);

	GameSystem::GetInstance()->SetPipeline(blendMode_);

	GameSystem::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定.
	GameSystem::GetInstance()->GetCommandList()->IASetIndexBuffer(&indexBufferView_); // IBVを設定.
	// 形状を設定。PS0に設定しているものとはまた別。同じものを設定すると考えておけば良い.
	GameSystem::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// マテリアル用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// transformationMatrixCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭の設定。2はrootParameter[2]である.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureInfo_.textureSrvHandlesGPU);
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(3, DirectionalLight::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後.
	GameSystem::GetInstance()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);



}

void Renderer::Sprite::Draw(const Transform2D& transform) {
	Transform transform3D;
	transform3D.scale.x = transform.scale.x;
	transform3D.scale.y = transform.scale.y;
	transform3D.scale.z = 1.0f;
	transform3D.rotate.x = transform.rotate;
	transform3D.rotate.y = 0.0f;
	transform3D.rotate.z = 0.0f;
	transform3D.translate.x = transform.translate.x;
	transform3D.translate.y = transform.translate.y;
	transform3D.translate.z = 0.0f;
	;
	Draw(transform3D);
}

void Renderer::Sprite::SetSize(Vector2 size) {
	size_ = size;
	AdaptationSize();
}

void Renderer::Sprite::SetSize(WindowSize windowSize) {
	size_ = { static_cast<float>(windowSize.width),static_cast<float>(windowSize.height) };
	AdaptationSize();
}

void Renderer::Sprite::AdaptationSize() {
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0].position = { 0.0f,size_.y,0.0f,1.0f }; // 左下.
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[0].normal = { 0.0f,0.0f,-1.0f };
	vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f }; // 左上.
	vertexData[1].texcoord = { 0.0f,0.0f };
	vertexData[1].normal = { 0.0f,0.0f,-1.0f };
	vertexData[2].position = { size_.x,size_.y,0.0f,1.0f }; // 右下.
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[2].normal = { 0.0f,0.0f,-1.0f };
	vertexData[3].position = { size_.x,0.0f,0.0f,1.0f }; // 右上.
	vertexData[3].texcoord = { 1.0f,0.0f };
	vertexData[3].normal = { 0.0f,0.0f,-1.0f };
}

Renderer::Line* Renderer::Line::GetInstance() {
	static Renderer::Line instance;
	return &instance;
}

void Renderer::Line::Initialize() {
	for (uint32_t i = 0; i < kLineMax; i++) {
		lineDatas_[i] = new Renderer::Line::LineData();
		lineDatas_[i]->textureInfo_ = TextureManager::GetInstance()->GetTextureInfo("white_template");
		lineDatas_[i]->blendMode_ = BlendMode::kLine;

		// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)
		lineDatas_[i]->vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData)* 2);

		// 【MaterialResourceを生成する】
		// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
		lineDatas_[i]->materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
		// マテリアルにデータを書き込む.
		// 書き込むためのアドレスを取得.
		lineDatas_[i]->materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&lineDatas_[i]->materialData_));
		// 今回は赤を書き込んでみる
		lineDatas_[i]->materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		lineDatas_[i]->materialData_->lightingType = static_cast<uint32_t>(LightingType::kAspectNone);
		lineDatas_[i]->materialData_->uvTransform = Matrix4x4::Identity();

		// 【TransformationMatrix】
		// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する.
		lineDatas_[i]->wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
		// データを書き込む.
		// 書き込むためのアドレスを取得.
		lineDatas_[i]->wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&lineDatas_[i]->wvpData_));
		// 単位行列を書き込んでおく.
		lineDatas_[i]->wvpData_->WVP = Matrix4x4::Identity();
		lineDatas_[i]->wvpData_->World = Matrix4x4::Identity();

		// 【VertexBufferViewを作成する】
		// 頂点バッファビューを作成する.
		// リソースの先頭のアドレスから使う.
		lineDatas_[i]->vertexBufferView_.BufferLocation = lineDatas_[i]->vertexResource_->GetGPUVirtualAddress();
		// 使用するリソースのサイズは頂点3つ分のサイズ.(多分ここは他の場所でも変えられる。Rendererから頂点数取ってきて代入とかできそう)
		lineDatas_[i]->vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * 2);
		// 1頂点あたりのサイズ.
		lineDatas_[i]->vertexBufferView_.StrideInBytes = sizeof(VertexData);

		// 【Resourceにデータを書き込む】
		// 書き込むためのアドレスを取得.
		lineDatas_[i]->vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&lineDatas_[i]->vertexData));
	}

	currentDrawLineIndex_ = 0;
}

void Renderer::Line::Draw(const Vector3& startVector3, const Vector3& endVector3,const Vector4& color) {

	Vector3 centerVector3;
	centerVector3.x = (static_cast<Vector3>(startVector3) + endVector3 ).x / 2.0f;
	centerVector3.y = (static_cast<Vector3>(startVector3) + endVector3 ).y / 2.0f;
	centerVector3.z = (static_cast<Vector3>(startVector3) + endVector3 ).z / 2.0f;
	Vector3 diff = (static_cast<Vector3>(startVector3) - endVector3 );

	Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix({1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, centerVector3);

	lineDatas_[currentDrawLineIndex_]->vertexData[0].position = { -diff.x / 2.0f,-diff.y / 2.0f ,-diff.z / 2.0f ,1.0f};
	lineDatas_[currentDrawLineIndex_]->vertexData[0].texcoord = {0.0f,1.0f};
	lineDatas_[currentDrawLineIndex_]->vertexData[0].normal = { 0.0f,0.0f,0.0f };
	lineDatas_[currentDrawLineIndex_]->vertexData[1].position = { diff.x / 2.0f,diff.y / 2.0f ,diff.z / 2.0f ,1.0f};
	lineDatas_[currentDrawLineIndex_]->vertexData[1].texcoord = { 0.0f,1.0f };
	lineDatas_[currentDrawLineIndex_]->vertexData[1].normal = { 0.0f,0.0f,0.0f };

	lineDatas_[currentDrawLineIndex_]->wvpData_->World = worldMatrix;
	lineDatas_[currentDrawLineIndex_]->wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);

	lineDatas_[currentDrawLineIndex_]->materialData_->uvTransform = Matrix4x4::MakeAffineMatrix(lineDatas_[currentDrawLineIndex_]->uvTransform_);
	lineDatas_[currentDrawLineIndex_]->materialData_->color = color;

	/*=============================================================
	三角形の描画のコマンド.
	=============================================================*/
	GameSystem::GetInstance()->SetPipeline(lineDatas_[currentDrawLineIndex_]->blendMode_);

	GameSystem::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &lineDatas_[currentDrawLineIndex_]->vertexBufferView_); // VBVを設定.
	// 形状を設定。PS0に設定しているものとはまた別。同じものを設定すると考えておけば良い.
	GameSystem::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	// CBufferの場所を設定.
	// マテリアル用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, lineDatas_[currentDrawLineIndex_]->materialResource_->GetGPUVirtualAddress());
	// WVP用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, lineDatas_[currentDrawLineIndex_]->wvpResource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭の設定。2はrootParameter[2]である.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(2, lineDatas_[currentDrawLineIndex_]->textureInfo_.textureSrvHandlesGPU);
	// DirectionalLight用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(3, DirectionalLight::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後.
	GameSystem::GetInstance()->GetCommandList()->DrawInstanced(2, 1, 0, 0);
	currentDrawLineIndex_++;
}

void Renderer::ModelTriangle::Initialize(TextureInfo info) {
	blendMode_ = BlendMode::kNormal;

	isVisible_ = true;

	textureInfo_ = info;
	// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)
	vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * 3);

	// 【MaterialResourceを生成する】
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
	materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む.
	// 書き込むためのアドレスを取得.
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// 今回は赤を書き込んでみる
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->lightingType = static_cast<uint32_t>(LightingType::kHalfLambert);
	materialData_->uvTransform = Matrix4x4::Identity();

	// 【TransformationMatrix】
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する.
	wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む.
	// 書き込むためのアドレスを取得.
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	// 単位行列を書き込んでおく.
	wvpData_->WVP = Matrix4x4::Identity();
	wvpData_->World = Matrix4x4::Identity();


	// 【VertexBufferViewを作成する】

	// 頂点バッファビューを作成する.
	// リソースの先頭のアドレスから使う.
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ.(多分ここは他の場所でも変えられる。Rendererから頂点数取ってきて代入とかできそう)
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * 3);
	// 1頂点あたりのサイズ.
	vertexBufferView_.StrideInBytes = sizeof(VertexData);


	// 【Resourceにデータを書き込む】
	// 書き込むためのアドレスを取得.
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	vertexData[0].position = {-0.5f,-0.5f,0.0f,1.0f};
	vertexData[0].texcoord = {0.0f,1.0f};
	vertexData[0].normal = {0.0f,0.0f,1.0f};

	vertexData[1].position = {0.0f,0.5f,0.0f,1.0f};
	vertexData[1].texcoord = { 0.5f,0.0f };
	vertexData[1].normal = { 0.0f,0.0f,1.0f };
	
	vertexData[2].position = {0.5f,-0.5f,0.0f,1.0f};
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[2].normal = { 0.0f,0.0f,1.0f };
}

void Renderer::ModelTriangle::SetVertexPosition(const Vector3& topVertex, const Vector3& leftVertex, const Vector3& rightVertex) {
	vertexData[0].position = { leftVertex.x,leftVertex.y,leftVertex.z,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };

	vertexData[1].position = { topVertex.x,topVertex.y,topVertex.z,1.0f };

	vertexData[2].position = { rightVertex.x,rightVertex.y,rightVertex.z,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };

	float topTexcoordX = (topVertex.x - leftVertex.x) / (rightVertex.x - leftVertex.x);

	vertexData[1].texcoord = { topTexcoordX,0.0f };
}

Vector3* Renderer::ModelTriangle::GetVertexPosition(){
	Vector3 vertexPosition[3] = { 0.0f };
	vertexPosition[0] = { vertexData[1].position.x,vertexData[1].position.y,vertexData[1].position.z };
	vertexPosition[1] = { vertexData[0].position.x,vertexData[0].position.y,vertexData[0].position.z };
	vertexPosition[2] = { vertexData[2].position.x,vertexData[2].position.y,vertexData[2].position.z };
	return vertexPosition;
}

void Renderer::ModelTriangle::Draw(const Transform& transform) {
	if (!isVisible_) {
		return;
	}

	Matrix4x4 worldMatrix = transform.GetAffineMatrix();

	wvpData_->World = worldMatrix;
	wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);
	/*=============================================================
	三角形の描画のコマンド.
	=============================================================*/
	GameSystem::GetInstance()->SetPipeline(blendMode_);

	GameSystem::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定.
	// 形状を設定。PS0に設定しているものとはまた別。同じものを設定すると考えておけば良い.
	GameSystem::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// CBufferの場所を設定.
	// マテリアル用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// WVP用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭の設定。2はrootParameter[2]である.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureInfo_.textureSrvHandlesGPU);
	// DirectionalLight用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(3, DirectionalLight::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後.
	GameSystem::GetInstance()->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}
