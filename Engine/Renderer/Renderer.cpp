#include "Renderer.h"
#include <vector>
Model::~Model() {
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

void Model::Initialize(const ModelInfo& info) {
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
		materialData_[i]->reflectionType = static_cast<uint32_t>(ReflectionType::kPhong);
		materialData_[i]->shininess = 40.0f;

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

void Model::Initialize(const std::string& name){
	Initialize(ModelManager::GetInstance()->GetModelInfo(name));
}

void Model::Draw(const Transform& transform) const {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (!isVisible_[i]) {
			return;
		}

		Matrix4x4 worldMatrix = transform.GetAffineMatrix();

		wvpData_[i]->World = worldMatrix;
		wvpData_[i]->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);

		materialData_[i]->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform_[i]);

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = GameSystem::GetInstance()->GetCommandList();

		/*=============================================================
		三角形の描画のコマンド.
		=============================================================*/
		//GameSystem::GetInstance()->SetPipeline(blendMode_);
		//
		//commandList->IASetVertexBuffers(0, 1, &vertexBufferView_[i]); // VBVを設定.
		//// 形状を設定。PS0に設定しているものとはまた別。同じものを設定すると考えておけば良い.
		//commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//// CBufferの場所を設定.
		//// マテリアル用のCBufferの場所.
		//commandList->SetGraphicsRootConstantBufferView(0, materialResource_[i]->GetGPUVirtualAddress());
		//// WVP用のCBufferの場所.
		//commandList->SetGraphicsRootConstantBufferView(1, wvpResource_[i]->GetGPUVirtualAddress());
		//// SRVのDescriptorTableの先頭の設定。2はrootParameter[2]である.
		//commandList->SetGraphicsRootDescriptorTable(2, modelData_[i].textureSrvHandlesGPU);
		//// DirectionalLight用のCBufferの場所.
		//commandList->SetGraphicsRootConstantBufferView(3, DirectionalLight::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());
		//// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後.
		//commandList->DrawInstanced(UINT(modelData_[i].vertices.size()), 1, 0, 0);

		D3D12_VERTEX_BUFFER_VIEW vetexBufferView = vertexBufferView_[i];

		GameSystem::GetInstance()->DrawCommand(
			blendMode_,
			&vetexBufferView,
			nullptr,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			materialResource_[i],
			wvpResource_[i],
			modelData_[i].textureSrvHandlesGPU,
			UINT(modelData_[i].vertices.size())
		);
	}
}

void Model::SetIsVisible(bool isVisible) {
	if (modelMax_ == 1) {
		isVisible_[0] = isVisible;
		return;
	}

	for (uint32_t i = 0; i < modelMax_; i++) {
		isVisible_[i] = isVisible;
	}
}

void Model::SetIsVisible(bool isVisible, const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		isVisible_[i] = isVisible;
		break;
	}
};

bool Model::GetIsVisible() {
	if (modelMax_ == 1) {
		return isVisible_[0];
	}
};

bool Model::GetIsVisible(const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		return isVisible_[i];
	}
};

void Model::ChangeTexture(const TextureInfo& info) {
	if (modelMax_ == 1) {
		modelData_[0].textureSrvHandlesGPU = info.textureSrvHandlesGPU;
		return;
	}

	for (uint32_t i = 0; i < modelMax_; i++) {
		modelData_[i].textureSrvHandlesGPU = info.textureSrvHandlesGPU;
	}
};

void Model::ChangeTexture(const TextureInfo& info, const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		modelData_[i].textureSrvHandlesGPU = info.textureSrvHandlesGPU;
		break;
	}
};

void Model::SetColor(Vector4 color) {
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
void Model::SetColor(Vector4 color, const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		materialData_[i]->color = color;
		break;
	}
};

Vector4 Model::GetColor() {
	return materialData_[0]->color;
};
Vector4 Model::GetColor(const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		return materialData_[i]->color;
	}
};

void Model::SetUvTransform(const Transform& uvTransform) {
	if (modelMax_ == 1) {
		uvTransform_[0] = uvTransform;
		return;
	}

	for (uint32_t i = 0; i < modelMax_; i++) {
		uvTransform_[i] = uvTransform;
	}
};
void Model::SetUvTransform(const Transform& uvTransform, const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		uvTransform_[i] = uvTransform;
		break;
	}
}

Transform Model::GetUvTransform() {
	return uvTransform_[0];
};
Transform Model::GetUvTransform(const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		return uvTransform_[i];
	}
};

void Model::SetLightingType(LightingType type) {
	if (modelMax_ == 1) {
		materialData_[0]->lightingType = static_cast<int32_t>(type);
		return;
	}

	for (uint32_t i = 0; i < modelMax_; i++) {
		materialData_[i]->lightingType = static_cast<int32_t>(type);
	}
};
void Model::SetLightingType(LightingType type, const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		materialData_[i]->lightingType = static_cast<int32_t>(type);
		break;
	}
};

LightingType Model::GetLightingType() {
	return static_cast<LightingType>(materialData_[0]->lightingType);
};
LightingType Model::GetLightingType(const std::string& meshName) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (modelData_[i].meshName != meshName) {
			continue;
		}

		return static_cast<LightingType>(materialData_[i]->lightingType);
	}
}

std::vector<ModelElement*> Model::GetModelElement() const {
	ModelElement* modelElement;
	std::vector<ModelElement*> modelElements;

	for (uint32_t i = 0; i < modelMax_; i++) {
		modelElement = new ModelElement();
		modelElement->modelMax_ = modelMax_;
		modelElement->materialData_ = materialData_[i];
		modelElement->wvpData_ = wvpData_[i];
		modelElement->modelData_ = modelData_[i];
		modelElement->vertexResource_ = vertexResource_[i];
		modelElement->materialResource_ = materialResource_[i];
		modelElement->wvpResource_ = wvpResource_[i];
		modelElement->vertexBufferView_ = vertexBufferView_[i];
		modelElement->uvTransform_ = uvTransform_[i];
		modelElement->blendMode_ = blendMode_;

		modelElements.push_back(modelElement);
	}
	return modelElements;
}

void Sprite::Initialize(TextureInfo info) {
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
	materialData_->lightingType = static_cast<uint32_t>(LightingType::kNone);
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

void Sprite::Initialize() {
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
	materialData_->lightingType = static_cast<uint32_t>(LightingType::kNone);
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

void Sprite::Draw(const Transform& transform) {
	if (!isVisible_) {
		return;
	}

	Transform worldTransform = transform;

	worldTransform.translate.x = transform.translate.x - (size_.x / 2.0f);
	worldTransform.translate.y = transform.translate.y - (size_.y / 2.0f);

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
		nullptr,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		materialResource_,
		transformationMatrixResource_,
		textureInfo_.textureSrvHandlesGPU,
		6
	);
	
	//GameSystem::GetInstance()->SetPipeline(blendMode_);
	//
	//GameSystem::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定.
	//GameSystem::GetInstance()->GetCommandList()->IASetIndexBuffer(&indexBufferView_); // IBVを設定.
	//// 形状を設定。PS0に設定しているものとはまた別。同じものを設定すると考えておけば良い.
	//GameSystem::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//// マテリアル用のCBufferの場所.
	//GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	//// transformationMatrixCBufferの場所.
	//GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	//// SRVのDescriptorTableの先頭の設定。2はrootParameter[2]である.
	//GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureInfo_.textureSrvHandlesGPU);
	//GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(3, DirectionalLight::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());
	//// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後.
	//GameSystem::GetInstance()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);



}

void Sprite::Draw(const Transform2D& transform) {
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

void Sprite::SetSize(Vector2 size) {
	size_ = size;
	AdaptationSize();
}

void Sprite::SetSize(WindowSize windowSize) {
	size_ = { static_cast<float>(windowSize.width),static_cast<float>(windowSize.height) };
	AdaptationSize();
}

void Sprite::AdaptationSize() {
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

ModelElement* Sprite::GetModelElement() const {
	ModelElement* modelElement;
	modelElement = new ModelElement();
	modelElement->materialData_ = materialData_;
	modelElement->wvpData_ = transformationMatrixData_;
	modelElement->modelData_.textureSrvHandlesGPU = textureInfo_.textureSrvHandlesGPU;
	modelElement->vertexResource_ = vertexResource_;
	modelElement->materialResource_ = materialResource_;
	modelElement->wvpResource_ = transformationMatrixResource_;
	modelElement->vertexBufferView_ = vertexBufferView_;
	modelElement->uvTransform_ = uvTransform_;
	modelElement->indexResource_ = indexResource_;
	modelElement->indexBufferView_ = indexBufferView_;
	modelElement->blendMode_ = blendMode_;
	return modelElement;
}

Renderer* Renderer::GetInstance() {
	static Renderer instance;
	return &instance;
}

void Renderer::Initialize() {
	currentDrawIndex_ = 0;

	blendMode_ = BlendMode::kNormal;

	lightingType_ = LightingType::kHalfLambert;

	reflectionType_ = ReflectionType::kBlinnPhong;
}

void Renderer::ClearDrawIndex() {
	currentDrawIndex_ = 0;
	//modelElement.clear();
}

void Renderer::SetBlendMode(BlendMode blendMode) {
	if (blendMode == blendMode_) {
		return;
	}

	blendMode_ = blendMode;
}

void Renderer::SetLightingType(LightingType lightingType) {
	if (lightingType == lightingType_) {
		return;
	}

	lightingType_ = lightingType;
}

void Renderer::SetReflectionType(ReflectionType reflectionType){
	if (reflectionType == reflectionType_) {
		return;
	}

	reflectionType_ = reflectionType;
}

void Renderer::DrawLine(const Vector3& startVector3, const Vector3& endVector3, const Vector4& color) {
	ModelElement* newElement;
	newElement = new ModelElement();
	CreateLine(newElement);
	Vector3 centerVector3;
	centerVector3.x = (static_cast<Vector3>(startVector3) + endVector3).x / 2.0f;
	centerVector3.y = (static_cast<Vector3>(startVector3) + endVector3).y / 2.0f;
	centerVector3.z = (static_cast<Vector3>(startVector3) + endVector3).z / 2.0f;
	Vector3 diff = (static_cast<Vector3>(startVector3) - endVector3);

	Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, centerVector3);

	newElement->vertexData[0].position = { -diff.x / 2.0f,-diff.y / 2.0f ,-diff.z / 2.0f ,1.0f };
	newElement->vertexData[0].texcoord = { 0.0f,1.0f };
	newElement->vertexData[0].normal = { 0.0f,0.0f,0.0f };
	newElement->vertexData[1].position = { diff.x / 2.0f,diff.y / 2.0f ,diff.z / 2.0f ,1.0f };
	newElement->vertexData[1].texcoord = { 0.0f,1.0f };
	newElement->vertexData[1].normal = { 0.0f,0.0f,0.0f };

	newElement->wvpData_->World = worldMatrix;
	newElement->wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);
	newElement->wvpData_->WorldInverseTranspose = worldMatrix.Transpose().Inverse();

	newElement->materialData_->uvTransform = Matrix4x4::MakeAffineMatrix(newElement->uvTransform_);
	newElement->materialData_->color = color;

	/*=============================================================
	三角形の描画のコマンド.
	=============================================================*/
	GameSystem::GetInstance()->DrawCommand(
		newElement->blendMode_,
		&newElement->vertexBufferView_,
		nullptr,
		D3D_PRIMITIVE_TOPOLOGY_LINELIST,
		newElement->materialResource_,
		newElement->wvpResource_,
		newElement->modelData_.textureSrvHandlesGPU,
		2
	);
	currentDrawIndex_++;
}

void Renderer::DrawSphere(const Transform& transform, const TextureInfo& textureInfo, const Vector4& color) {
	const uint32_t kSubdivision_ = 16;

	Matrix4x4 worldMatrix = transform.GetAffineMatrix();
	ModelElement* newElement;
	newElement = new ModelElement();
	CreateSphere(newElement);

	newElement->modelData_.textureSrvHandlesGPU = textureInfo.textureSrvHandlesGPU;

	newElement->wvpData_->World = worldMatrix;
	newElement->wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);
	newElement->wvpData_->WorldInverseTranspose = worldMatrix.Transpose().Inverse();
	/*=============================================================
	三角形の描画のコマンド.
	=============================================================*/
	newElement->indexInstanceNum_ = kSubdivision_ * kSubdivision_ * 6;
	GameSystem::GetInstance()->DrawCommand(
		newElement->blendMode_,
		&newElement->vertexBufferView_,
		&newElement->indexBufferView_,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		newElement->materialResource_,
		newElement->wvpResource_,
		newElement->modelData_.textureSrvHandlesGPU,
		newElement->indexInstanceNum_
	);
}

void Renderer::DrawSphereWireFrame(const Transform& transform, const Vector4& color){
	if (true) {
		return;
	}

	const uint32_t kSubdivision = 4;
	const float kLonEvery = std::numbers::pi_v<float> *2.0f / kSubdivision;
	const float kLatEvery = std::numbers::pi_v<float> / kSubdivision;


	for (uint32_t latIndex = 0; latIndex < kSubdivision; latIndex++) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; lonIndex++) {
			float lon = lonIndex * kLonEvery;

			Vector3 a = { cos(lat) * cos(lon),sin(lat),cos(lat) * sin(lon) };
			Vector3 b = { cos(lat + std::numbers::pi_v<float> / kSubdivision) * cos(lon),sin(lat + std::numbers::pi_v<float> / kSubdivision),cos(lat + std::numbers::pi_v<float> / kSubdivision) * sin(lon) };
			Vector3 c = { cos(lat) * cos(lon + std::numbers::pi_v<float> *2.0f / kSubdivision),sin(lat),cos(lat) * sin(lon + std::numbers::pi_v<float> *2.0f / kSubdivision) };

			Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			Vector3 startPosition = worldMatrix.MatrixTransform(a);
			Vector3 endPosition = worldMatrix.MatrixTransform(b);

			DrawLine(startPosition, endPosition, color);

			startPosition = worldMatrix.MatrixTransform(a);
			endPosition = worldMatrix.MatrixTransform(c);

			DrawLine(startPosition, endPosition, color);
		}
	}
}

void Renderer::DrawBox(const Transform& transform, const TextureInfo& textureInfo, const Vector4& color) {
	ModelElement* newElement;
	newElement = new ModelElement();
	CreateBox(newElement);

	Matrix4x4 worldMatrix = transform.GetAffineMatrix();

	newElement->wvpData_->World = worldMatrix;
	newElement->wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);
	newElement->wvpData_->WorldInverseTranspose = worldMatrix.Transpose().Inverse();

	newElement->materialData_->uvTransform = Matrix4x4::MakeAffineMatrix(newElement->uvTransform_);

	/*=============================================================
	三角形の描画のコマンド.
	=============================================================*/
	GameSystem::GetInstance()->DrawCommand(
		newElement->blendMode_,
		&newElement->vertexBufferView_,
		nullptr	,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		newElement->materialResource_,
		newElement->wvpResource_,
		newElement->modelData_.textureSrvHandlesGPU,
		UINT(newElement->modelData_.vertices.size())
	);
}

void Renderer::DrawBoxWireFrame(const Transform& transform,const Vector3& size, const Vector4& color){
	std::vector<Vector3> vertices;

	Transform VertexTransform;
	VertexTransform.Initialize();
	VertexTransform.SetParent(&transform);

	VertexTransform.translate = Vector3(-size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f);
	vertices.push_back(VertexTransform.GetAffineMatrix().GetMatrixToTranslate());
	VertexTransform.translate = Vector3(-size.x / 2.0f, size.y / 2.0f, -size.z / 2.0f);
	vertices.push_back(VertexTransform.GetAffineMatrix().GetMatrixToTranslate());
	VertexTransform.translate = Vector3(size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f);
	vertices.push_back(VertexTransform.GetAffineMatrix().GetMatrixToTranslate());
	VertexTransform.translate = Vector3(size.x / 2.0f, size.y / 2.0f, -size.z / 2.0f);
	vertices.push_back(VertexTransform.GetAffineMatrix().GetMatrixToTranslate());

	VertexTransform.translate = Vector3(-size.x / 2.0f, -size.y / 2.0f, size.z / 2.0f);
	vertices.push_back(VertexTransform.GetAffineMatrix().GetMatrixToTranslate());
	VertexTransform.translate = Vector3(-size.x / 2.0f, size.y / 2.0f, size.z / 2.0f);
	vertices.push_back(VertexTransform.GetAffineMatrix().GetMatrixToTranslate());
	VertexTransform.translate = Vector3(size.x / 2.0f, -size.y / 2.0f, size.z / 2.0f);
	vertices.push_back(VertexTransform.GetAffineMatrix().GetMatrixToTranslate());
	VertexTransform.translate = Vector3(size.x / 2.0f, size.y / 2.0f, size.z / 2.0f);
	vertices.push_back(VertexTransform.GetAffineMatrix().GetMatrixToTranslate());

	DrawLine(vertices[0],vertices[1],color);
	DrawLine(vertices[0],vertices[2],color);
	DrawLine(vertices[1],vertices[3],color);
	DrawLine(vertices[2],vertices[3],color);

	DrawLine(vertices[4],vertices[5],color);
	DrawLine(vertices[4],vertices[6],color);
	DrawLine(vertices[5],vertices[7],color);
	DrawLine(vertices[6],vertices[7],color);

	DrawLine(vertices[0],vertices[4],color);
	DrawLine(vertices[1],vertices[5],color);
	DrawLine(vertices[2],vertices[6],color);
	DrawLine(vertices[3],vertices[7],color);
}

void Renderer::DrawBoxWireFrame(const AABB& aabb, const Vector4& color){
	std::vector<Vector3> vertices;

	vertices.push_back(aabb.min);
	vertices.push_back(Vector3(aabb.min.x,aabb.max.y,aabb.min.z));
	vertices.push_back(Vector3(aabb.max.x, aabb.min.y, aabb.min.z));
	vertices.push_back(Vector3(aabb.max.x, aabb.max.y, aabb.min.z));

	vertices.push_back(Vector3(aabb.min.x, aabb.min.y, aabb.max.z));
	vertices.push_back(Vector3(aabb.min.x, aabb.max.y, aabb.max.z));
	vertices.push_back(Vector3(aabb.max.x, aabb.min.y, aabb.max.z));
	vertices.push_back(aabb.max);

	DrawLine(vertices[0], vertices[1], color);
	DrawLine(vertices[0], vertices[2], color);
	DrawLine(vertices[1], vertices[3], color);
	DrawLine(vertices[2], vertices[3], color);

	DrawLine(vertices[4], vertices[5], color);
	DrawLine(vertices[4], vertices[6], color);
	DrawLine(vertices[5], vertices[7], color);
	DrawLine(vertices[6], vertices[7], color);

	DrawLine(vertices[0], vertices[4], color);
	DrawLine(vertices[1], vertices[5], color);
	DrawLine(vertices[2], vertices[6], color);
	DrawLine(vertices[3], vertices[7], color);
}

void Renderer::DrawModel(const Transform& transform, const ModelInfo& modelInfo, const Vector4& color) {
	uint32_t modelMax_ = static_cast<uint32_t>(modelInfo.modelData.size());
	std::vector<ModelElement*> newElements;
	newElements.resize(modelMax_);
	for (uint32_t i = 0; i < modelMax_; i++) {
		newElements[i] = new ModelElement();
		newElements[i]->modelData_ = modelInfo.modelData[i];
	}

	CreateNewModel(newElements, modelMax_);

	for (uint32_t i = 0; i < modelMax_; i++) {
		Matrix4x4 worldMatrix = transform.GetAffineMatrix();

		newElements[i]->wvpData_->World = worldMatrix;
		newElements[i]->wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);
		newElements[i]->wvpData_->WorldInverseTranspose = worldMatrix.Transpose().Inverse();

		newElements[i]->materialData_->uvTransform = Matrix4x4::MakeAffineMatrix(newElements[i]->uvTransform_);

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = GameSystem::GetInstance()->GetCommandList();

		/*=============================================================
		三角形の描画のコマンド.
		=============================================================*/
		GameSystem::GetInstance()->DrawCommand(
			blendMode_,
			&newElements[i]->vertexBufferView_,
			nullptr,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			newElements[i]->materialResource_,
			newElements[i]->wvpResource_,
			newElements[i]->modelData_.textureSrvHandlesGPU,
			UINT(newElements[i]->modelData_.vertices.size())
		);
	}
}

void Renderer::DrawModel(const Transform& transform, const Model* model) {
	uint32_t modelMax_ = static_cast<uint32_t>(model->GetModelCountMax());
	std::vector<ModelElement*> newElements;
	newElements.resize(modelMax_);
	newElements = model->GetModelElement();

	CreateModel(newElements, modelMax_);

	for (uint32_t i = 0; i < modelMax_; i++) {
		Matrix4x4 worldMatrix = transform.GetAffineMatrix();

		newElements[i]->wvpData_->World = worldMatrix;
		newElements[i]->wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);
		newElements[i]->wvpData_->WorldInverseTranspose = worldMatrix.Transpose().Inverse();

		newElements[i]->materialData_->uvTransform = Matrix4x4::MakeAffineMatrix(newElements[i]->uvTransform_);

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = GameSystem::GetInstance()->GetCommandList();

		/*=============================================================
		三角形の描画のコマンド.
		=============================================================*/
		GameSystem::GetInstance()->DrawCommand(
			blendMode_,
			&newElements[i]->vertexBufferView_,
			nullptr,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			newElements[i]->materialResource_,
			newElements[i]->wvpResource_,
			newElements[i]->modelData_.textureSrvHandlesGPU,
			UINT(newElements[i]->modelData_.vertices.size())
		);
	}

}

void Renderer::DrawSprite(const Transform& transform, const TextureInfo& textureInfo, const Vector4& color) {
	Transform worldTransform = transform;

	Vector2 size;
	size.x = textureInfo.width;
	size.y = textureInfo.height;

	ModelElement* newElement;
	newElement = new ModelElement();
	newElement->modelData_.textureSrvHandlesGPU = textureInfo.textureSrvHandlesGPU;

	CreateNewSprite(newElement, size.x, size.y);

	worldTransform.translate.x = transform.translate.x - (size.x / 2.0f);
	worldTransform.translate.y = transform.translate.y - (size.y / 2.0f);

	Matrix4x4 worldMatrix = worldTransform.GetAffineMatrix();

	newElement->wvpData_->World = worldMatrix;
	newElement->wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrixSprite(worldMatrix);
	newElement->wvpData_->WorldInverseTranspose = worldMatrix.Transpose().Inverse();

	newElement->materialData_->uvTransform = Matrix4x4::MakeAffineMatrix(newElement->uvTransform_);
	/*=============================================================
	三角形のSpriteの描画のコマンド.
	=============================================================*/
	GameSystem::GetInstance()->DrawCommand(
		blendMode_,
		&newElement->vertexBufferView_,
		&newElement->indexBufferView_,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		newElement->materialResource_,
		newElement->wvpResource_,
		newElement->modelData_.textureSrvHandlesGPU,
		6
	);
}

void Renderer::DrawSprite(const Transform& transform, const Sprite& sprite) {
	Transform worldTransform = transform;

	Vector2 size = sprite.GetSize();

	ModelElement* newElement;
	newElement = new ModelElement();
	newElement = sprite.GetModelElement();

	CreateSprite(newElement, size.x, size.y);

	worldTransform.translate.x = transform.translate.x - (size.x / 2.0f);
	worldTransform.translate.y = transform.translate.y - (size.y / 2.0f);

	Matrix4x4 worldMatrix = worldTransform.GetAffineMatrix();

	newElement->wvpData_->World = worldMatrix;
	newElement->wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrixSprite(worldMatrix);
	newElement->wvpData_->WorldInverseTranspose = worldMatrix.Transpose().Inverse();

	newElement->materialData_->uvTransform = Matrix4x4::MakeAffineMatrix(newElement->uvTransform_);
	/*=============================================================
	三角形のSpriteの描画のコマンド.
	=============================================================*/
	GameSystem::GetInstance()->DrawCommand(
		blendMode_,
		&newElement->vertexBufferView_,
		&newElement->indexBufferView_,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		newElement->materialResource_,
		newElement->wvpResource_,
		newElement->modelData_.textureSrvHandlesGPU,
		6
	);
}

void Renderer::DrawShadow(const Transform& transform, const Model* model){
	uint32_t modelMax_ = static_cast<uint32_t>(model->GetModelCountMax());
	std::vector<ModelElement*> newElements;
	newElements.resize(modelMax_);
	newElements = model->GetModelElement();

	CreateModel(newElements, modelMax_);

	for (uint32_t i = 0; i < modelMax_; i++) {
		newElements[i]->materialData_->lightingType = static_cast<uint32_t>(LightingType::kNone);
		newElements[i]->materialData_->reflectionType = static_cast<uint32_t>(ReflectionType::kNone);
		Matrix4x4 worldMatrix = transform.GetAffineMatrix();
		Matrix4x4 projectionMatrix = Matrix4x4::Identity();
		projectionMatrix.matrix[1][1] = 0.01f;
		worldMatrix = worldMatrix * projectionMatrix;
		newElements[i]->wvpData_->World = worldMatrix;
		newElements[i]->wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);
		newElements[i]->wvpData_->WorldInverseTranspose = worldMatrix.Transpose().Inverse();

		newElements[i]->materialData_->uvTransform = Matrix4x4::MakeAffineMatrix(newElements[i]->uvTransform_);

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = GameSystem::GetInstance()->GetCommandList();

		/*=============================================================
		三角形の描画のコマンド.
		=============================================================*/
		GameSystem::GetInstance()->DrawCommand(
			blendMode_,
			&newElements[i]->vertexBufferView_,
			nullptr,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			newElements[i]->materialResource_,
			newElements[i]->wvpResource_,
			newElements[i]->modelData_.textureSrvHandlesGPU,
			UINT(newElements[i]->modelData_.vertices.size())
		);
	}
}

void Renderer::DrawShadow(const Transform& transform, const Model* model, const Vector4& color) {
	uint32_t modelMax_ = static_cast<uint32_t>(model->GetModelCountMax());
	std::vector<ModelElement*> newElements;
	newElements.resize(modelMax_);
	newElements = model->GetModelElement();
	CreateNewModel(newElements, modelMax_);

	for (uint32_t i = 0; i < modelMax_; i++) {
		newElements[i]->materialData_->lightingType = static_cast<uint32_t>(LightingType::kNone);
		newElements[i]->materialData_->reflectionType = static_cast<uint32_t>(ReflectionType::kNone);
		Matrix4x4 worldMatrix = transform.GetAffineMatrix();
		Matrix4x4 projectionMatrix = Matrix4x4::Identity();
		projectionMatrix.matrix[1][1] = 0.01f;
		worldMatrix = worldMatrix * projectionMatrix;
		newElements[i]->wvpData_->World = worldMatrix;
		newElements[i]->wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);
		newElements[i]->wvpData_->WorldInverseTranspose = worldMatrix.Transpose().Inverse();

		newElements[i]->materialData_->uvTransform = Matrix4x4::MakeAffineMatrix(newElements[i]->uvTransform_);
		newElements[i]->materialData_->color = color;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = GameSystem::GetInstance()->GetCommandList();

		/*=============================================================
		三角形の描画のコマンド.
		=============================================================*/
		GameSystem::GetInstance()->DrawCommand(
			blendMode_,
			&newElements[i]->vertexBufferView_,
			nullptr,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			newElements[i]->materialResource_,
			newElements[i]->wvpResource_,
			newElements[i]->modelData_.textureSrvHandlesGPU,
			UINT(newElements[i]->modelData_.vertices.size())
		);
	}
}

void Renderer::CreateLine(ModelElement* newElement) {
	newElement->modelData_.textureSrvHandlesGPU = TextureManager::GetInstance()->GetTextureInfo("white_template").textureSrvHandlesGPU;
	newElement->blendMode_ = BlendMode::kLine;

	// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)
	newElement->vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * 2);

	// 【MaterialResourceを生成する】
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
	newElement->materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む.
	// 書き込むためのアドレスを取得.
	newElement->materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElement->materialData_));
	// 今回は赤を書き込んでみる
	newElement->materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	newElement->materialData_->lightingType = static_cast<uint32_t>(LightingType::kNone);
	newElement->materialData_->uvTransform = Matrix4x4::Identity();

	// 【TransformationMatrix】
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する.
	newElement->wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む.
	// 書き込むためのアドレスを取得.
	newElement->wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElement->wvpData_));
	// 単位行列を書き込んでおく.
	newElement->wvpData_->WVP = Matrix4x4::Identity();
	newElement->wvpData_->World = Matrix4x4::Identity();

	// 【VertexBufferViewを作成する】
	// 頂点バッファビューを作成する.
	// リソースの先頭のアドレスから使う.
	newElement->vertexBufferView_.BufferLocation = newElement->vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ.(多分ここは他の場所でも変えられる。Rendererから頂点数取ってきて代入とかできそう)
	newElement->vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * 2);
	// 1頂点あたりのサイズ.
	newElement->vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 【Resourceにデータを書き込む】
	// 書き込むためのアドレスを取得.
	newElement->vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElement->vertexData));
}

void Renderer::CreateSphere(ModelElement* newElement) {
	const uint32_t kSubdivision_ = 16;

	newElement->blendMode_ = blendMode_;//BlendMode::kNormal;

	// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)
	newElement->vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * kSubdivision_ * kSubdivision_ * 6);

	// 【MaterialResourceを生成する】
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
	newElement->materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む.
	// 書き込むためのアドレスを取得.
	newElement->materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElement->materialData_));
	// 今回は赤を書き込んでみる
	newElement->materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	newElement->materialData_->lightingType = static_cast<uint32_t>(lightingType_);//static_cast<uint32_t>(LightingType::kHalfLambert);
	newElement->materialData_->reflectionType = static_cast<uint32_t>(reflectionType_);
	newElement->materialData_->shininess = 40.0f;
	newElement->materialData_->uvTransform = Matrix4x4::Identity();

	// 【TransformationMatrix】
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する.
	newElement->wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む.
	// 書き込むためのアドレスを取得.
	newElement->wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElement->wvpData_));
	// 単位行列を書き込んでおく.
	newElement->wvpData_->WVP = Matrix4x4::Identity();
	newElement->wvpData_->World = Matrix4x4::Identity();


	// 【VertexBufferViewを作成する】

	// 頂点バッファビューを作成する.
	// リソースの先頭のアドレスから使う.
	newElement->vertexBufferView_.BufferLocation = newElement->vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ.(多分ここは他の場所でも変えられる。Rendererから頂点数取ってきて代入とかできそう)
	newElement->vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * kSubdivision_ * kSubdivision_ * 4);
	// 1頂点あたりのサイズ.
	newElement->vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 【IndexResourceを生成する】
	// 実際に頂点リソースを作る.
	newElement->indexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(uint32_t) * kSubdivision_ * kSubdivision_ * 6);

	// 頂点バッファビューを作成する.
	// リソースの先頭のアドレスから使う.
	newElement->indexBufferView_.BufferLocation = newElement->indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ.
	newElement->indexBufferView_.SizeInBytes = sizeof(uint32_t) * kSubdivision_ * kSubdivision_ * 6;
	// インデックスはuint32_tとする.
	newElement->indexBufferView_.Format = DXGI_FORMAT_R32_UINT;


	// 【Resourceにデータを書き込む】

	// 頂点リソースにデータを書き込む.
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得.
	newElement->vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// インデックスリソースにデータを書き込む.
	uint32_t* indexData = nullptr;
	// 書き込むためのアドレスを取得.
	newElement->indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));


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

void Renderer::CreateBox(ModelElement* newElement) {
	newElement->blendMode_ = blendMode_;
	newElement->uvTransform_.Initialize();
	newElement->modelData_ = ModelManager::GetInstance()->GetModelInfo("block_template").modelData[0];
	newElement->modelData_.textureSrvHandlesGPU = newElement->modelData_.textureSrvHandlesGPU;
	// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)
	newElement->vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * newElement->modelData_.vertices.size());

	// 【MaterialResourceを生成する】
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
	newElement->materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む.
	// 書き込むためのアドレスを取得.
	newElement->materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElement->materialData_));
	// 今回は赤を書き込んでみる
	newElement->materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	newElement->materialData_->lightingType = static_cast<uint32_t>(lightingType_);;
	newElement->materialData_->uvTransform = Matrix4x4::Identity();
	newElement->materialData_->reflectionType = static_cast<uint32_t>(reflectionType_);
	newElement->materialData_->shininess = 40.0f;

	// 【TransformationMatrix】
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する.
	newElement->wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む.
	// 書き込むためのアドレスを取得.
	newElement->wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElement->wvpData_));
	// 単位行列を書き込んでおく.
	newElement->wvpData_->WVP = Matrix4x4::Identity();
	newElement->wvpData_->World = Matrix4x4::Identity();

	// 【VertexBufferViewを作成する】
	// 頂点バッファビューを作成する.
	// リソースの先頭のアドレスから使う.
	newElement->vertexBufferView_.BufferLocation = newElement->vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ.(多分ここは他の場所でも変えられる。Rendererから頂点数取ってきて代入とかできそう)
	newElement->vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * newElement->modelData_.vertices.size());
	// 1頂点あたりのサイズ.
	newElement->vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 【Resourceにデータを書き込む】
	// 頂点リソースにデータを書き込む.
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得.
	newElement->vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	memcpy(vertexData, newElement->modelData_.vertices.data(), sizeof(VertexData) * newElement->modelData_.vertices.size());
}

void Renderer::CreateNewModel(std::vector<ModelElement*> newElements, const uint32_t modelMax) {

	for (uint32_t i = 0; i < modelMax; i++) {
		// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)
		if (newElements[i]->modelData_.materialData.textureFilePath == "") {
			newElements[i]->modelData_.textureSrvHandlesGPU = TextureManager::GetInstance()->GetTextureInfo("white_template").textureSrvHandlesGPU;
		}

		newElements[i]->blendMode_ = blendMode_;
		newElements[i]->vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * newElements[i]->modelData_.vertices.size());

		// 【MaterialResourceを生成する】
		// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
		//Microsoft::WRL::ComPtr<ID3D12Resource> materialResource 
		newElements[i]->materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
		// マテリアルにデータを書き込む.
		//Material* materialData = nullptr;
		// 書き込むためのアドレスを取得.
		newElements[i]->materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElements[i]->materialData_));
		// 今回は赤を書き込んでみる
		newElements[i]->materialData_->color = newElements[i]->modelData_.materialData.matarial.color;
		newElements[i]->materialData_->lightingType = static_cast<uint32_t>(lightingType_);;
		newElements[i]->materialData_->uvTransform = newElements[i]->modelData_.materialData.matarial.uvTransform;
		newElements[i]->materialData_->reflectionType = static_cast<uint32_t>(reflectionType_);
		newElements[i]->materialData_->shininess = 40.0f;

		// 【TransformationMatrix】
		// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する.
		//Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource 
		newElements[i]->wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
		// データを書き込む.
		//TransformationMatrix* wvpData = nullptr;
		// 書き込むためのアドレスを取得.
		newElements[i]->wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElements[i]->wvpData_));
		// 単位行列を書き込んでおく.
		newElements[i]->wvpData_->WVP = Matrix4x4::Identity();
		newElements[i]->wvpData_->World = Matrix4x4::Identity();
		newElements[i]->uvTransform_.Initialize();
		newElements[i]->uvTransform_.scale = newElements[i]->materialData_->uvTransform.GetMatrixToTransform().scale;
		newElements[i]->uvTransform_.rotate = newElements[i]->materialData_->uvTransform.GetMatrixToTransform().rotate;
		newElements[i]->uvTransform_.translate = newElements[i]->materialData_->uvTransform.GetMatrixToTransform().translate;

		// 【VertexBufferViewを作成する】

		// 頂点バッファビューを作成する.
		//D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
		// リソースの先頭のアドレスから使う.
		newElements[i]->vertexBufferView_.BufferLocation = newElements[i]->vertexResource_->GetGPUVirtualAddress();
		// 使用するリソースのサイズは頂点3つ分のサイズ.(多分ここは他の場所でも変えられる。Rendererから頂点数取ってきて代入とかできそう)
		//vertexBufferView.SizeInBytes = sizeof(VertexData) * kSubdivision * kSubdivision * 4;
		newElements[i]->vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * newElements[i]->modelData_.vertices.size());
		// 1頂点あたりのサイズ.
		newElements[i]->vertexBufferView_.StrideInBytes = sizeof(VertexData);


		// 【Resourceにデータを書き込む】

		// 頂点リソースにデータを書き込む.
		VertexData* vertexData = nullptr;
		// 書き込むためのアドレスを取得.
		newElements[i]->vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
		memcpy(vertexData, newElements[i]->modelData_.vertices.data(), sizeof(VertexData) * newElements[i]->modelData_.vertices.size());
	}


}

void Renderer::CreateModel(std::vector<ModelElement*> newElements, const uint32_t modelMax) {
	for (uint32_t i = 0; i < modelMax; i++) {
		// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)

		if (newElements[i]->modelData_.materialData.textureFilePath == "") {
			newElements[i]->modelData_.textureSrvHandlesGPU = TextureManager::GetInstance()->GetTextureInfo("white_template").textureSrvHandlesGPU;
		}

		newElements[i]->vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * newElements[i]->modelData_.vertices.size());

		// 【MaterialResourceを生成する】
		// 書き込むためのアドレスを取得.
		newElements[i]->materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElements[i]->materialData_));
		// 今回は赤を書き込んでみる
		//newElements[i]->materialData_->color;
		//newElements[i]->materialData_->lightingType = static_cast<uint32_t>(LightingType::kHalfLambert);
		//newElements[i]->materialData_->uvTransform = newElements[i]->modelData_.materialData.matarial.uvTransform;

		// 【TransformationMatrix】
		// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する.
		newElements[i]->wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
		// データを書き込む.
		//TransformationMatrix* wvpData = nullptr;
		// 書き込むためのアドレスを取得.
		newElements[i]->wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElements[i]->wvpData_));
		// 単位行列を書き込んでおく.
		newElements[i]->wvpData_->WVP = Matrix4x4::Identity();
		newElements[i]->wvpData_->World = Matrix4x4::Identity();
		//newElements[i]->uvTransform_.Initialize();
		//newElements[i]->uvTransform_.scale = newElements[i]->materialData_->uvTransform.GetMatrixToTransform().scale;
		//newElements[i]->uvTransform_.rotate = newElements[i]->materialData_->uvTransform.GetMatrixToTransform().rotate;
		//newElements[i]->uvTransform_.translate = newElements[i]->materialData_->uvTransform.GetMatrixToTransform().translate;

		// 【VertexBufferViewを作成する】

		// 頂点バッファビューを作成する.
		//D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
		// リソースの先頭のアドレスから使う.
		newElements[i]->vertexBufferView_.BufferLocation = newElements[i]->vertexResource_->GetGPUVirtualAddress();
		// 使用するリソースのサイズは頂点3つ分のサイズ.(多分ここは他の場所でも変えられる。Rendererから頂点数取ってきて代入とかできそう)
		//vertexBufferView.SizeInBytes = sizeof(VertexData) * kSubdivision * kSubdivision * 4;
		newElements[i]->vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * newElements[i]->modelData_.vertices.size());
		// 1頂点あたりのサイズ.
		newElements[i]->vertexBufferView_.StrideInBytes = sizeof(VertexData);


		// 【Resourceにデータを書き込む】

		// 頂点リソースにデータを書き込む.
		VertexData* vertexData = nullptr;
		// 書き込むためのアドレスを取得.
		newElements[i]->vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
		memcpy(vertexData, newElements[i]->modelData_.vertices.data(), sizeof(VertexData) * newElements[i]->modelData_.vertices.size());
	}
}

void Renderer::CreateNewSprite(ModelElement* newElement, float width, float height) {
	newElement->blendMode_ = blendMode_;
	newElement->uvTransform_.Initialize();

	/*=============================================================
	Sprite用のResourceとView.
	=============================================================*/
	// 【VertexResourceを生成する】
	// 実際に頂点リソースを作る.
	newElement->vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * 4);

	// 頂点バッファビューを作成する.
	// リソースの先頭のアドレスから使う.
	newElement->vertexBufferView_.BufferLocation = newElement->vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ.
	newElement->vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点あたりのサイズ.
	newElement->vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 【IndexResourceを生成する】
	// 実際に頂点リソースを作る.
	newElement->indexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(uint32_t) * 6);

	// 頂点バッファビューを作成する.
	// リソースの先頭のアドレスから使う.
	newElement->indexBufferView_.BufferLocation = newElement->indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ.
	newElement->indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする.
	newElement->indexBufferView_.Format = DXGI_FORMAT_R32_UINT;


	// 【MaterialResourceを生成する】
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite
	newElement->materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む.
	// 書き込むためのアドレスを取得.
	newElement->materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElement->materialData_));
	// 今回は赤を書き込んでみる
	newElement->materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	newElement->materialData_->lightingType = static_cast<uint32_t>(LightingType::kNone);
	newElement->materialData_->uvTransform = Matrix4x4::Identity();
	newElement->materialData_->reflectionType = static_cast<uint32_t>(ReflectionType::kNone);
	newElement->materialData_->shininess = 0.0f;

	// 【TransformationMatrix】
	//Sprite用のTransformationMatrixを作る。Matrix4x4 1つ分のサイズを用意する.
	newElement->wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む.
	// 書き込むためのアドレスを取得.
	newElement->wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElement->wvpData_));
	// 単位行列を書き込んでおく.
	newElement->wvpData_->WVP = Matrix4x4::Identity();
	newElement->wvpData_->World = Matrix4x4::Identity();

	// 【Resourceにデータを書き込む】

	// 頂点リソースにデータを書き込む.
	// 書き込むためのアドレスを取得.
	newElement->vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElement->vertexData));
	newElement->vertexData[0].position = { 0.0f,height,0.0f,1.0f }; // 左下.
	newElement->vertexData[0].texcoord = { 0.0f,1.0f };
	newElement->vertexData[0].normal = { 0.0f,0.0f,-1.0f };
	newElement->vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f }; // 左上.
	newElement->vertexData[1].texcoord = { 0.0f,0.0f };
	newElement->vertexData[1].normal = { 0.0f,0.0f,-1.0f };
	newElement->vertexData[2].position = { width,height,0.0f,1.0f }; // 右下.
	newElement->vertexData[2].texcoord = { 1.0f,1.0f };
	newElement->vertexData[2].normal = { 0.0f,0.0f,-1.0f };
	newElement->vertexData[3].position = { width,0.0f,0.0f,1.0f }; // 右上.
	newElement->vertexData[3].texcoord = { 1.0f,0.0f };
	newElement->vertexData[3].normal = { 0.0f,0.0f,-1.0f };

	// インデックスリソースにデータを書き込む.
	uint32_t* indexDataSprite = nullptr;
	// 書き込むためのアドレスを取得.
	newElement->indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	// 1枚目の三角形.
	indexDataSprite[0] = 0;
	indexDataSprite[1] = 1;
	indexDataSprite[2] = 2;
	indexDataSprite[3] = 1;
	indexDataSprite[4] = 3;
	indexDataSprite[5] = 2;
}

void Renderer::CreateSprite(ModelElement* newElement, float width, float height) {
	//newElement->blendMode_ = BlendMode::kNormal;
	newElement->uvTransform_.Initialize();
	/*=============================================================
	Sprite用のResourceとView.
	=============================================================*/
	// 【VertexResourceを生成する】
	// 実際に頂点リソースを作る.
	//newElement->vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * 4);

	// 頂点バッファビューを作成する.
	// リソースの先頭のアドレスから使う.
	newElement->vertexBufferView_.BufferLocation = newElement->vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ.
	newElement->vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点あたりのサイズ.
	newElement->vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 【IndexResourceを生成する】
	// 実際に頂点リソースを作る.
	newElement->indexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(uint32_t) * 6);

	// 頂点バッファビューを作成する.
	// リソースの先頭のアドレスから使う.
	newElement->indexBufferView_.BufferLocation = newElement->indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ.
	newElement->indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする.
	newElement->indexBufferView_.Format = DXGI_FORMAT_R32_UINT;


	// 【MaterialResourceを生成する】
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite
	//newElement->materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む.
	// 書き込むためのアドレスを取得.
	newElement->materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElement->materialData_));
	// 今回は赤を書き込んでみる
	//newElement->materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//newElement->materialData_->lightingType = static_cast<uint32_t>(LightingType::kAspectNone);
	//newElement->materialData_->uvTransform = Matrix4x4::Identity();

	// 【TransformationMatrix】
	//Sprite用のTransformationMatrixを作る。Matrix4x4 1つ分のサイズを用意する.
	newElement->wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む.
	// 書き込むためのアドレスを取得.
	newElement->wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElement->wvpData_));
	// 単位行列を書き込んでおく.
	newElement->wvpData_->WVP = Matrix4x4::Identity();
	newElement->wvpData_->World = Matrix4x4::Identity();

	// 【Resourceにデータを書き込む】

	// 頂点リソースにデータを書き込む.
	// 書き込むためのアドレスを取得.
	newElement->vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElement->vertexData));
	newElement->vertexData[0].position = { 0.0f,height,0.0f,1.0f }; // 左下.
	newElement->vertexData[0].texcoord = { 0.0f,1.0f };
	newElement->vertexData[0].normal = { 0.0f,0.0f,-1.0f };
	newElement->vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f }; // 左上.
	newElement->vertexData[1].texcoord = { 0.0f,0.0f };
	newElement->vertexData[1].normal = { 0.0f,0.0f,-1.0f };
	newElement->vertexData[2].position = { width,height,0.0f,1.0f }; // 右下.
	newElement->vertexData[2].texcoord = { 1.0f,1.0f };
	newElement->vertexData[2].normal = { 0.0f,0.0f,-1.0f };
	newElement->vertexData[3].position = { width,0.0f,0.0f,1.0f }; // 右上.
	newElement->vertexData[3].texcoord = { 1.0f,0.0f };
	newElement->vertexData[3].normal = { 0.0f,0.0f,-1.0f };

	// インデックスリソースにデータを書き込む.
	uint32_t* indexDataSprite = nullptr;
	// 書き込むためのアドレスを取得.
	newElement->indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	// 1枚目の三角形.
	indexDataSprite[0] = 0;
	indexDataSprite[1] = 1;
	indexDataSprite[2] = 2;
	indexDataSprite[3] = 1;
	indexDataSprite[4] = 3;
	indexDataSprite[5] = 2;
}

/*
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
	=============================================================
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
	=============================================================
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
	=============================================================
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
	=============================================================
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
*/

void TestParticle::Initialize(const ModelInfo& info,uint32_t numInstanced) {
	numInstance_ = numInstanced;
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
	materialData_->lightingType = static_cast<uint32_t>(LightingType::kHalfLambert);
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


	instancingResource_ = GameSystem::GetInstance()->CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix) * numInstance_);

	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));

	for (uint32_t index = 0; index < numInstance_; index++) {
		instancingData_[index].WVP = Matrix4x4::Identity();
		instancingData_[index].World = Matrix4x4::Identity();
	}

	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = numInstance_;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(TransformationMatrix);

	instancingSrvHandleCPU = GameSystem::GetInstance()->GetCPUDescriptorHandle(GameSystem::GetInstance()->GetSrvDescriptorHeap(),GameSystem::GetInstance()->GetDescriptorSizeSRV(), GameSystem::GetInstance()->GetSrvDescriptorHeapNum());
	instancingSrvHandleGPU = GameSystem::GetInstance()->GetGPUDescriptorHandle(GameSystem::GetInstance()->GetSrvDescriptorHeap(),GameSystem::GetInstance()->GetDescriptorSizeSRV(), GameSystem::GetInstance()->GetSrvDescriptorHeapNum());

	GameSystem::GetInstance()->SrvDescriptorHeapNumIncrement();

	GameSystem::GetInstance()->GetDevice()->CreateShaderResourceView(instancingResource_.Get(), &instancingSrvDesc, instancingSrvHandleCPU);
}

void TestParticle::Draw(const Transform& transform) const {
	if (!isVisible_) {
		return;
	}

	for (uint32_t index = 0; index < numInstance_; index++) {
		Transform instancingTransform = transform;
		instancingTransform.translate = {index * 0.1f,index * 0.1f, index * 0.1f };

		Matrix4x4 worldMatrix = instancingTransform.GetAffineMatrix();

		instancingData_[index].World = worldMatrix;
		instancingData_[index].WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);
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
