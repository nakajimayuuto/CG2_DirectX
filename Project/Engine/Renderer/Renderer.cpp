#include "Renderer.h"
#include "Camera.h"

#include "../../Managers/SoundManager.h"
#include "../../Managers/SrvManager.h"
#include "../../Managers/ImGuiManager.h"
#include <vector>

namespace {
	// スプライトのインデックス(2枚の三角形).
	constexpr uint32_t kSpriteIndices[6] = { 0, 1, 2, 1, 3, 2 };

	/// <summary>
	/// 3D描画用の行列を作る(World / WVP / WorldInverseTranspose).
	/// </summary>
	TransformationMatrix MakeTransformationMatrix(Matrix4x4 rootMatrix, Matrix4x4 worldMatrix) {
		TransformationMatrix result;
		result.World = rootMatrix * worldMatrix;
		result.WVP = rootMatrix * Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);
		result.WorldInverseTranspose = rootMatrix * worldMatrix.Transpose().Inverse();
		return result;
	}

	/// <summary>
	/// スプライト描画用の行列を作る.
	/// </summary>
	TransformationMatrix MakeSpriteTransformationMatrix(Matrix4x4 worldMatrix) {
		TransformationMatrix result;
		result.World = worldMatrix;
		result.WVP = Camera::GetInstance()->GetWorldViewProjectionMatrixSprite(worldMatrix);
		result.WorldInverseTranspose = worldMatrix.Transpose().Inverse();
		return result;
	}

	/// <summary>
	/// スプライト用のマテリアル(ライティングなし)を作る.
	/// </summary>
	Material MakeSpriteMaterial(const Vector4& color) {
		Material material{};
		material.color = color;
		material.lightingType = static_cast<int32_t>(LightingType::kNone);
		material.uvTransform = Matrix4x4::Identity();
		material.reflectionType = static_cast<int32_t>(ReflectionType::kNone);
		material.shininess = 0.0f;
		return material;
	}

	/// <summary>
	/// 中心が原点のスプライトの頂点4つを作る.
	/// </summary>
	void MakeSpriteVertices(VertexData(&vertices)[4], float width, float height) {
		float halfWidth = width * 0.5f;
		float halfHeight = height * 0.5f;
		vertices[0] = { { -halfWidth, halfHeight,0.0f,1.0f },{ 0.0f,1.0f },{ 0.0f,0.0f,-1.0f } }; // 左下.
		vertices[1] = { { -halfWidth,-halfHeight,0.0f,1.0f },{ 0.0f,0.0f },{ 0.0f,0.0f,-1.0f } }; // 左上.
		vertices[2] = { {  halfWidth, halfHeight,0.0f,1.0f },{ 1.0f,1.0f },{ 0.0f,0.0f,-1.0f } }; // 右下.
		vertices[3] = { {  halfWidth,-halfHeight,0.0f,1.0f },{ 1.0f,0.0f },{ 0.0f,0.0f,-1.0f } }; // 右上.
	}
}

Model::~Model() {
	materialData_.clear();

	isVisible_.clear();

	meshes_.clear();

	materialResource_.clear();

	uvTransform_.clear();
}

void Model::Initialize(const ModelInfo& info) {
	modelMax_ = static_cast<uint32_t>(info.modelData.size());

	radius_ = info.radius;

	rootMatrix_ = info.rootMatrix;

	blendMode_ = BlendMode::kNormal;

	meshes_.resize(modelMax_);
	materialData_.resize(modelMax_);
	isVisible_.resize(modelMax_);
	materialResource_.resize(modelMax_);
	uvTransform_.resize(modelMax_);

	// 【TransformationMatrix】
	// 全メッシュで同じ行列を使うので、モデルにつき1つだけ作る.
	wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	wvpData_->WVP = Matrix4x4::Identity();
	wvpData_->World = Matrix4x4::Identity();
	wvpData_->WorldInverseTranspose = Matrix4x4::Identity();

	for (uint32_t i = 0; i < modelMax_; i++) {
		const ModelData& data = info.modelData[i];

		isVisible_[i] = true;

		// 【頂点バッファ】
		// 頂点データはコピーせず、ModelManagerが持っているバッファをそのまま使う.
		meshes_[i].meshName = data.meshName;
		meshes_[i].vertexCount = data.vertexCount;
		meshes_[i].vertexBufferView = data.vertexBufferView_;
		meshes_[i].textureSrvHandlesGPU = data.textureSrvHandlesGPU;

		if (data.materialData.textureFilePath == "") {
			meshes_[i].textureSrvHandlesGPU = TextureManager::GetInstance()->GetTextureInfo("white_template").textureSrvHandlesGPU;
		}

		// 【MaterialResourceを生成する】
		materialResource_[i] = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
		materialResource_[i]->Map(0, nullptr, reinterpret_cast<void**>(&materialData_[i]));
		materialData_[i]->color = data.materialData.matarial.color;
		materialData_[i]->lightingType = static_cast<uint32_t>(LightingType::kHalfLambert);
		materialData_[i]->uvTransform = data.materialData.matarial.uvTransform;
		materialData_[i]->reflectionType = static_cast<uint32_t>(ReflectionType::kPhong);
		materialData_[i]->shininess = 40.0f;

		Transform uvTransform = materialData_[i]->uvTransform.GetMatrixToTransform();
		uvTransform_[i].Initialize();
		uvTransform_[i].scale = uvTransform.scale;
		uvTransform_[i].rotate = uvTransform.rotate;
		uvTransform_[i].translate = uvTransform.translate;
	}
}

void Model::Initialize(const std::string& name) {
	Initialize(ModelManager::GetInstance()->GetModelInfo(name));
}

void Model::Draw() {
	Draw(transform_);
}
void Model::Draw(const Transform& transform) {
	Draw(transform, true);
}

void Model::Draw(const Transform& transform, bool useTransparent) {
	(void)useTransparent;
	if (modelMax_ == 0) {
		return;
	}

	// カリングと行列計算はメッシュ数に関係なく1回だけ行う.
	if (!Camera::GetInstance()->IsInCameraFrustum(transform.GetWorldPosition(), radius_ * transform.GetMaxScale())) {
		return;
	}

	*wvpData_ = MakeTransformationMatrix(rootMatrix_, transform.GetAffineMatrix());

	for (uint32_t i = 0; i < modelMax_; i++) {
		if (!isVisible_[i]) {
			continue;
		}

		materialData_[i]->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform_[i]);

		/*=============================================================
		三角形の描画のコマンド.
		=============================================================*/
		GameSystem::GetInstance()->DrawCommand(
			blendMode_,
			ShaderType::kNormal,
			&meshes_[i].vertexBufferView,
			nullptr,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			materialResource_[i],
			wvpResource_,
			meshes_[i].textureSrvHandlesGPU,
			meshes_[i].vertexCount
		);
	}
}

int32_t Model::FindMeshIndex(const std::string& meshName) const {
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (meshes_[i].meshName == meshName) {
			return static_cast<int32_t>(i);
		}
	}
	return -1;
}

void Model::SetIsVisible(bool isVisible) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		isVisible_[i] = isVisible;
	}
}

void Model::SetIsVisible(bool isVisible, const std::string& meshName) {
	int32_t index = FindMeshIndex(meshName);
	if (index < 0) {
		return;
	}
	isVisible_[index] = isVisible;
};

bool Model::GetIsVisible()  const {
	// どれか1つでも表示されていればtrue.
	for (uint32_t i = 0; i < modelMax_; i++) {
		if (isVisible_[i]) {
			return true;
		}
	}
	return false;
};

bool Model::GetIsVisible(const std::string& meshName) const {
	int32_t index = FindMeshIndex(meshName);
	if (index < 0) {
		return isVisible_[0];
	}
	return isVisible_[index];
};

void Model::ChangeTexture(const TextureInfo& info) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		meshes_[i].textureSrvHandlesGPU = info.textureSrvHandlesGPU;
	}
};

void Model::ChangeTexture(const TextureInfo& info, const std::string& meshName) {
	int32_t index = FindMeshIndex(meshName);
	if (index < 0) {
		return;
	}
	meshes_[index].textureSrvHandlesGPU = info.textureSrvHandlesGPU;
};

void Model::ChangeTexture(const TextureInfo& info, uint32_t index) {
	meshes_[index].textureSrvHandlesGPU = info.textureSrvHandlesGPU;
};

void Model::SetColor(Vector4 color) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		materialData_[i]->color = color;
	}
};
void Model::SetColor(Vector4 color, const std::string& meshName) {
	int32_t index = FindMeshIndex(meshName);
	if (index < 0) {
		return;
	}
	materialData_[index]->color = color;
}
void Model::SetColor(Vector4 color, uint32_t index) {
	materialData_[index]->color = color;
}

Vector4 Model::GetColor() {
	return materialData_[0]->color;
};
Vector4 Model::GetColor(const std::string& meshName) {
	int32_t index = FindMeshIndex(meshName);
	if (index < 0) {
		return materialData_[0]->color;
	}
	return materialData_[index]->color;
};

Vector4 Model::GetColor(uint32_t index) {
	return materialData_[index]->color;
};

void Model::SetUvTransform(const Transform& uvTransform) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		uvTransform_[i] = uvTransform;
	}
};
void Model::SetUvTransform(const Transform& uvTransform, const std::string& meshName) {
	int32_t index = FindMeshIndex(meshName);
	if (index < 0) {
		return;
	}
	uvTransform_[index] = uvTransform;
}

void Model::SetUvTransform(const Transform& uvTransform, uint32_t index) {
	uvTransform_[index] = uvTransform;
}

Transform Model::GetUvTransform() {
	return uvTransform_[0];
};
Transform Model::GetUvTransform(const std::string& meshName) {
	int32_t index = FindMeshIndex(meshName);
	if (index < 0) {
		return uvTransform_[0];
	}
	return uvTransform_[index];
}
Transform Model::GetUvTransform(uint32_t index) {
	return uvTransform_[index];
}

void Model::SetLightingType(LightingType type) {
	for (uint32_t i = 0; i < modelMax_; i++) {
		materialData_[i]->lightingType = static_cast<int32_t>(type);
	}
};
void Model::SetLightingType(LightingType type, const std::string& meshName) {
	int32_t index = FindMeshIndex(meshName);
	if (index < 0) {
		return;
	}
	materialData_[index]->lightingType = static_cast<int32_t>(type);
};

void Model::SetLightingType(LightingType type, uint32_t index) {
	materialData_[index]->lightingType = static_cast<int32_t>(type);
};

LightingType Model::GetLightingType() {
	return static_cast<LightingType>(materialData_[0]->lightingType);
};
LightingType Model::GetLightingType(const std::string& meshName) {
	int32_t index = FindMeshIndex(meshName);
	if (index < 0) {
		return static_cast<LightingType>(materialData_[0]->lightingType);
	}
	return static_cast<LightingType>(materialData_[index]->lightingType);
}
LightingType Model::GetLightingType(uint32_t index) {
	return static_cast<LightingType>(materialData_[index]->lightingType);
};

void Sprite::Initialize(TextureInfo info) {
	isVisible_ = true;

	blendMode_ = BlendMode::kNormal;
	uvTransform_.Initialize();

	anchorPoint_ = { 0.5f,0.5f };

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
	// 頂点の中身は下のAdaptationSize()で書き込む.
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// インデックスリソースにデータを書き込む.
	uint32_t* indexDataSprite = nullptr;
	// 書き込むためのアドレスを取得.
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	memcpy(indexDataSprite, kSpriteIndices, sizeof(kSpriteIndices));

	size_ = { static_cast<float>(textureInfo_.width),static_cast<float>(textureInfo_.height) };
	cutSize_ = size_;
	AdaptationSize();
}

void Sprite::Initialize() {
	Initialize(TextureManager::GetInstance()->GetTextureInfo("white_template"));
}

void Sprite::Draw() {
	Draw(transform_);
}
void Sprite::Draw(const Transform& transform) {
	if (!isVisible_) {
		return;
	}

	if (!ImGuiManager::GetInstance()->IsGameUIVisible()) {
		return;
	}

	transformationMatrixData_->World = transform.GetAffineMatrix();
	transformationMatrixData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrixSprite(transform.GetAffineMatrix());

	materialData_->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform_);
	/*=============================================================
	三角形のSpriteの描画のコマンド.
	=============================================================*/
	// Spriteの描画。変更が必要なものだけ変更する.
	GameSystem::GetInstance()->DrawCommand(
		blendMode_,
		ShaderType::kSprite,
		&vertexBufferView_,
		&indexBufferView_,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		materialResource_,
		transformationMatrixResource_,
		textureInfo_.textureSrvHandlesGPU,
		6
	);
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
	transform3D.translate.z = depth_;
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
	float texLeft = leftTop_.x / size_.x;
	float texRight = (leftTop_.x + cutSize_.x) / size_.x;
	float texTop = leftTop_.y / size_.y;
	float texBottom = (leftTop_.y + cutSize_.y) / size_.y;

	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0].position = { -cutSize_.x * anchorPoint_.x,cutSize_.y * (1.0f - anchorPoint_.y),0.0f,1.0f }; // 左下.
	vertexData[0].texcoord = { texLeft, texBottom };
	vertexData[0].normal = { 0.0f,0.0f,-1.0f };
	vertexData[1].position = { -cutSize_.x * anchorPoint_.x,-cutSize_.y * anchorPoint_.y,0.0f,1.0f }; // 左上.
	vertexData[1].texcoord = { texLeft, texTop };
	vertexData[1].normal = { 0.0f,0.0f,-1.0f };
	vertexData[2].position = { cutSize_.x * (1.0f - anchorPoint_.x),cutSize_.y * (1.0f - anchorPoint_.y),0.0f,1.0f }; // 右下.
	vertexData[2].texcoord = { texRight, texBottom };
	vertexData[2].normal = { 0.0f,0.0f,-1.0f };
	vertexData[3].position = { cutSize_.x * (1.0f - anchorPoint_.x),-cutSize_.y * anchorPoint_.y,0.0f,1.0f }; // 右上.
	vertexData[3].texcoord = { texRight, texTop };
	vertexData[3].normal = { 0.0f,0.0f,-1.0f };
}

Renderer* Renderer::GetInstance() {
	static Renderer instance;
	return &instance;
}

void Renderer::Initialize() {
#ifdef _DEBUG
	useDebugLine_ = true;
#endif // _DEBUG


	currentDrawModelIndex_ = 0;
	currentDrawSpriteIndex_ = 0;

	blendMode_ = BlendMode::kNormal;

	lightingType_ = LightingType::kHalfLambert;

	reflectionType_ = ReflectionType::kBlinnPhong;

	lineElement_ = std::make_unique<ModelElement>();
	CreateLine(lineElement_.get());
	modelInstances.clear();
	modelInstances.resize(maxModelNum);

	for (auto& instance : modelInstances) {
		instance = std::make_unique<ModelInstance>();
		instance->materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
		instance->wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
		// 書き込み用のアドレスを先に取得しておく(Upload Heapなので Map したままで良い).
		instance->materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&instance->materialData_));
		instance->wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&instance->wvpData_));
	}

	spriteInstances.clear();
	spriteInstances.resize(maxSpriteNum);

	// スプライトのインデックスはどれも同じなので、1つのバッファを全スプライトで共有する.
	Microsoft::WRL::ComPtr<ID3D12Resource> spriteIndexResource = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(kSpriteIndices));
	uint32_t* spriteIndexData = nullptr;
	spriteIndexResource->Map(0, nullptr, reinterpret_cast<void**>(&spriteIndexData));
	memcpy(spriteIndexData, kSpriteIndices, sizeof(kSpriteIndices));

	for (auto& instance : spriteInstances) {
		instance = std::make_unique<SpriteInstance>();
		instance->vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * 4);
		instance->indexResource_ = spriteIndexResource;
		instance->materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
		instance->wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));

		// Viewと書き込み用アドレスは初期化時に用意しておく(毎回作らない).
		instance->vertexBufferView_.BufferLocation = instance->vertexResource_->GetGPUVirtualAddress();
		instance->vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
		instance->vertexBufferView_.StrideInBytes = sizeof(VertexData);
		instance->indexBufferView_.BufferLocation = instance->indexResource_->GetGPUVirtualAddress();
		instance->indexBufferView_.SizeInBytes = sizeof(kSpriteIndices);
		instance->indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
		instance->vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&instance->vertexData_));
		instance->materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&instance->materialData_));
		instance->wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&instance->wvpData_));
	}

	// Primitive図形(形が変わらない図形は初回描画時に作る).
	staticPrimitiveMeshes_.clear();
	CreateDynamicPrimitivePools();
}

void Renderer::ClearDrawIndex() {
	currentDrawModelIndex_ = 0;
	currentDrawLineIndex_ = 0;
	currentDrawSpriteIndex_ = 0;
	// 形が変わるPrimitive図形のプールも、また先頭から使う.
	for (auto& pool : dynamicPrimitivePools_) {
		pool.second.currentIndex = 0;
	}
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

void Renderer::SetReflectionType(ReflectionType reflectionType) {
	if (reflectionType == reflectionType_) {
		return;
	}

	reflectionType_ = reflectionType;
}

void Renderer::DrawLine(const Vector3& startVector3, const Vector3& endVector3, const Vector4& color) {
	if (currentDrawLineIndex_ * 2 >= maxLineNum_) {
		return;
	}

	Vector3 centerVector3;
	centerVector3.x = (static_cast<Vector3>(startVector3) + endVector3).x / 2.0f;
	centerVector3.y = (static_cast<Vector3>(startVector3) + endVector3).y / 2.0f;
	centerVector3.z = (static_cast<Vector3>(startVector3) + endVector3).z / 2.0f;
	Vector3 diff = (static_cast<Vector3>(startVector3) - endVector3);

	Matrix4x4 worldMatrix = Matrix4x4::Identity();

	lineElement_->vertexDataLine[currentDrawLineIndex_ * 2].position = { (-diff.x / 2.0f) + centerVector3.x,(-diff.y / 2.0f) + centerVector3.y ,(-diff.z / 2.0f) + centerVector3.z ,1.0f };
	lineElement_->vertexDataLine[currentDrawLineIndex_ * 2].color = color;
	lineElement_->vertexDataLine[(currentDrawLineIndex_ * 2) + 1].position = { (diff.x / 2.0f) + centerVector3.x,(diff.y / 2.0f) + centerVector3.y,(diff.z / 2.0f) + centerVector3.z,1.0f };
	lineElement_->vertexDataLine[(currentDrawLineIndex_ * 2) + 1].color = color;
	currentDrawLineIndex_++;
}



void Renderer::DrawLineAll() {
	if (!useDebugLine_) {
		return;
	}

	Matrix4x4 worldMatrix = Matrix4x4::Identity();
	lineElement_->wvpData_->World = worldMatrix;
	lineElement_->wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);
	lineElement_->wvpData_->WorldInverseTranspose = worldMatrix.Transpose().Inverse();

	GameSystem::GetInstance()->DrawCommand(
		lineElement_->blendMode_,
		ShaderType::kLine,
		&lineElement_->vertexBufferView_,
		nullptr,
		D3D_PRIMITIVE_TOPOLOGY_LINELIST,
		lineElement_->materialResource_,
		lineElement_->wvpResource_,
		lineElement_->modelData_.textureSrvHandlesGPU,
		currentDrawLineIndex_ * 2
	);
}

void Renderer::ChangeUseDebugLine() {
	if (useDebugLine_) {
		useDebugLine_ = false;
	} else {
		useDebugLine_ = true;
	}
}

void Renderer::CreateDynamicPrimitivePools() {
	dynamicPrimitivePools_.clear();

	for (PrimitiveType type : { PrimitiveType::kTours, PrimitiveType::kRing, PrimitiveType::kCylinder }) {
		DynamicPrimitivePool& pool = dynamicPrimitivePools_[type];
		pool.meshes.resize(maxDynamicPrimitiveNum);

		// 1つ目のメッシュがインデックスバッファを作り、残りはそれを共有する.
		pool.meshes[0] = std::make_unique<PrimitiveMesh>();
		pool.meshes[0]->Initialize(type);
		for (uint32_t i = 1; i < maxDynamicPrimitiveNum; i++) {
			pool.meshes[i] = std::make_unique<PrimitiveMesh>();
			pool.meshes[i]->Initialize(type, PrimitiveParam{}, *pool.meshes[0]);
		}
		pool.currentIndex = 0;
	}
}

const PrimitiveMesh& Renderer::AcquirePrimitiveMesh(PrimitiveType type, const PrimitiveParam& param) {
	// 形が変わる図形: プールから1つ取り出して頂点を書き直す.
	if (PrimitiveGeometry::IsDynamic(type)) {
		DynamicPrimitivePool& pool = dynamicPrimitivePools_[type];
		assert(pool.currentIndex < pool.meshes.size());
		PrimitiveMesh* mesh = pool.meshes[pool.currentIndex].get();
		pool.currentIndex++;
		mesh->UpdateVertices(param);
		return *mesh;
	}

	// 形が変わらない図形: 初めて使う時に作って、以降は使い回す.
	auto it = staticPrimitiveMeshes_.find(type);
	if (it == staticPrimitiveMeshes_.end()) {
		PrimitiveMesh& mesh = staticPrimitiveMeshes_[type];
		if (type == PrimitiveType::kBox) {
			// 箱は block_template モデルの頂点バッファをそのまま使う.
			mesh.InitializeFromModelData(ModelManager::GetInstance()->GetModelInfo("block_template").modelData[0]);
		} else {
			mesh.Initialize(type);
		}
		return mesh;
	}
	return it->second;
}

void Renderer::DrawPrimitiveShape(PrimitiveType type, const Transform& transform, const PrimitiveParam& param, const TextureInfo& textureInfo, const Vector4& color, const Transform& uvTransform) {
	const PrimitiveMesh& mesh = AcquirePrimitiveMesh(type, param);

	DrawPrimitive(
		transform.GetAffineMatrix(),
		MakeMaterial(color, Matrix4x4::MakeAffineMatrix(uvTransform)),
		mesh.GetVertexBufferView(),
		mesh.GetIndexBufferView(),
		textureInfo.textureSrvHandlesGPU,
		mesh.GetDrawCount()
	);
}

void Renderer::DrawSphere(const Transform& transform, const TextureInfo& textureInfo, const Vector4& color) {
	DrawSphere(transform, textureInfo, color, Transform::GetInitialValue());
}

void Renderer::DrawSphere(const Transform& transform, const TextureInfo& textureInfo, const Vector4& color, const Transform& uvTransform) {
	DrawPrimitiveShape(PrimitiveType::kSphere, transform, {}, textureInfo, color, uvTransform);
}

void Renderer::DrawTorus(const Transform& transform, float majorRadius, float minorRadius, const TextureInfo& textureInfo, const Vector4& color) {
	DrawPrimitiveShape(PrimitiveType::kTours, transform, { majorRadius,minorRadius }, textureInfo, color, Transform::GetInitialValue());
}

void Renderer::DrawPlane(const Transform& transform, const TextureInfo& textureInfo, const Vector4& color, const Transform& uvTransform) {
	DrawPrimitiveShape(PrimitiveType::kPlane, transform, {}, textureInfo, color, uvTransform);
}

void Renderer::DrawRing(const Transform& transform, float majorRadius, float minorRadius, const TextureInfo& textureInfo, const Vector4& color, const Transform& uvTransform){
	DrawPrimitiveShape(PrimitiveType::kRing, transform, { majorRadius,minorRadius }, textureInfo, color, uvTransform);
}

void Renderer::DrawCylinder(const Transform& transform, float topRadius, float bottomRadius, const TextureInfo& textureInfo, const Vector4& color, const Transform& uvTransform){
	DrawPrimitiveShape(PrimitiveType::kCylinder, transform, { topRadius,bottomRadius }, textureInfo, color, uvTransform);
}

void Renderer::DrawSphereWireFrame(const Transform& transform, const Vector4& color) {
	if (!Camera::GetInstance()->IsInCameraFrustum(transform.GetWorldPosition(), transform.GetMaxScale())) {
		return;
	}

	const uint32_t kSubdivision = 8;
	const float kLonEvery = std::numbers::pi_v<float> *2.0f / kSubdivision;
	const float kLatEvery = std::numbers::pi_v<float> / kSubdivision;
	Transform sphereTransform = transform;
	sphereTransform.scale = sphereTransform.scale * 0.5f;

	for (uint32_t latIndex = 0; latIndex < kSubdivision; latIndex++) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; lonIndex++) {
			float lon = lonIndex * kLonEvery;

			Vector3 a = { cos(lat) * cos(lon),sin(lat),cos(lat) * sin(lon) };
			Vector3 b = { cos(lat + std::numbers::pi_v<float> / kSubdivision) * cos(lon),sin(lat + std::numbers::pi_v<float> / kSubdivision),cos(lat + std::numbers::pi_v<float> / kSubdivision) * sin(lon) };
			Vector3 c = { cos(lat) * cos(lon + std::numbers::pi_v<float> *2.0f / kSubdivision),sin(lat),cos(lat) * sin(lon + std::numbers::pi_v<float> *2.0f / kSubdivision) };

			Matrix4x4 worldMatrix = sphereTransform.GetAffineMatrix();
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
	DrawBox(transform, Transform::GetInitialValue(), textureInfo, color);
}

void Renderer::DrawBox(const Transform& transform, const Transform& uvTransform, const TextureInfo& textureInfo, const Vector4& color) {
	DrawPrimitiveShape(PrimitiveType::kBox, transform, {}, textureInfo, color, uvTransform);
}

void Renderer::DrawBoxEdges(const Vector3(&vertices)[8], const Vector4& color) {
	// 頂点の並び: 0~3が手前の面(-z)、4~7が奥の面(+z).
	static constexpr uint32_t kEdges[12][2] = {
		{0,1},{0,2},{1,3},{2,3}, // 手前の面.
		{4,5},{4,6},{5,7},{6,7}, // 奥の面.
		{0,4},{1,5},{2,6},{3,7}, // 手前と奥をつなぐ辺.
	};

	for (const auto& edge : kEdges) {
		DrawLine(vertices[edge[0]], vertices[edge[1]], color);
	}
}

void Renderer::DrawBoxWireFrame(const Transform& transform, const Vector3& size, const Vector4& color) {
	Transform vertexTransform;
	vertexTransform.Initialize();
	vertexTransform.SetParent(&transform);

	const Vector3 half = { size.x / 2.0f, size.y / 2.0f, size.z / 2.0f };
	const Vector3 localVertices[8] = {
		{ -half.x,-half.y,-half.z },{ -half.x, half.y,-half.z },{ half.x,-half.y,-half.z },{ half.x, half.y,-half.z },
		{ -half.x,-half.y, half.z },{ -half.x, half.y, half.z },{ half.x,-half.y, half.z },{ half.x, half.y, half.z },
	};

	Vector3 vertices[8];
	for (uint32_t i = 0; i < 8; i++) {
		vertexTransform.translate = localVertices[i];
		vertices[i] = vertexTransform.GetAffineMatrix().GetMatrixToTranslate();
	}

	DrawBoxEdges(vertices, color);
}

void Renderer::DrawPlaneWireFrame(Plane& plane) {
	Vector3 center = plane.normal * plane.distance * -1.0f; // 1.
	Vector3 perpendiculars[4];
	perpendiculars[0] = Vector3::Normalize(plane.normal.Perpendicular()); // 2.
	perpendiculars[1] = { -perpendiculars[0].x,-perpendiculars[0].y ,-perpendiculars[0].z }; // 3.
	perpendiculars[2] = plane.normal.Cross(perpendiculars[0]); // 4.
	perpendiculars[3] = { -perpendiculars[2].x,-perpendiculars[2].y ,-perpendiculars[2].z }; // 5.
	// 6.
	Vector3 points[4];
	for (int32_t index = 0; index < 4; ++index) {
		Vector3 extend = perpendiculars[index] * 2.0f;
		points[index] = center + extend;

	}


	DrawLine(points[0], points[3], { 1.0f,1.0f,1.0f,1.0f });
	DrawLine(points[3], points[1], { 1.0f,1.0f,1.0f,1.0f });
	DrawLine(points[1], points[2], { 1.0f,1.0f,1.0f,1.0f });
	DrawLine(points[2], points[0], { 1.0f,1.0f,1.0f,1.0f });

	DrawLine(
		center,
		center + plane.normal * 5.0f,
		{ 1.0f,0.0f,0.0f,1.0f });
}

void Renderer::DrawBoxWireFrame(const AABB& aabb, const Vector4& color) {
	const Vector3 vertices[8] = {
		aabb.min,
		Vector3(aabb.min.x, aabb.max.y, aabb.min.z),
		Vector3(aabb.max.x, aabb.min.y, aabb.min.z),
		Vector3(aabb.max.x, aabb.max.y, aabb.min.z),
		Vector3(aabb.min.x, aabb.min.y, aabb.max.z),
		Vector3(aabb.min.x, aabb.max.y, aabb.max.z),
		Vector3(aabb.max.x, aabb.min.y, aabb.max.z),
		aabb.max,
	};

	DrawBoxEdges(vertices, color);
}

void Renderer::DrawBoxWireFrame(const OBB& obb, const Vector4& color) {
	Matrix4x4 matrix;
	Matrix4x4 rotateMatrix;
	rotateMatrix = rotateMatrix.Identity();

	for (uint32_t i = 0; i < 3; i++) {
		rotateMatrix.matrix[i][0] = obb.orientations[i].x;
		rotateMatrix.matrix[i][1] = obb.orientations[i].y;
		rotateMatrix.matrix[i][2] = obb.orientations[i].z;
	}

	matrix = rotateMatrix;
	matrix = matrix * Matrix4x4::MakeTranslateMatrix(obb.center);

	const Vector3& s = obb.size;
	const Vector3 vertices[8] = {
		matrix.MatrixTransform({ -s.x,-s.y,-s.z }),
		matrix.MatrixTransform({ -s.x, s.y,-s.z }),
		matrix.MatrixTransform({  s.x,-s.y,-s.z }),
		matrix.MatrixTransform({  s.x, s.y,-s.z }),
		matrix.MatrixTransform({ -s.x,-s.y, s.z }),
		matrix.MatrixTransform({ -s.x, s.y, s.z }),
		matrix.MatrixTransform({  s.x,-s.y, s.z }),
		matrix.MatrixTransform({  s.x, s.y, s.z }),
	};

	DrawBoxEdges(vertices, color);
}

void Renderer::DrawModel(const Transform& transform, const ModelInfo& modelInfo, const Vector4& color, bool useTransparent) {
	(void)color;
	if (!Camera::GetInstance()->IsInCameraFrustum(transform.translate, modelInfo.radius * transform.GetMaxScale())) {
		return;
	}

	// 行列はメッシュ数に関係なく1回だけ計算する.
	Matrix4x4 worldMatrix = transform.GetAffineMatrix();
	DrawModelInfo(MakeTransformationMatrix(modelInfo.rootMatrix, worldMatrix), worldMatrix, modelInfo, nullptr, useTransparent);
}

void Renderer::DrawModel(const Transform& transform, const Model* model, bool useTransparent) {
	if (!model->GetIsVisible()) {
		return;
	}

	Matrix4x4 worldMatrix = transform.GetAffineMatrix();
	DrawModelInstance(MakeTransformationMatrix(model->rootMatrix_, worldMatrix), worldMatrix, model, nullptr, useTransparent);
}

ModelInstance* Renderer::AcquireModelInstance() {
	// 1フレームで使えるCBufferの数を超えたら止める.
	assert(currentDrawModelIndex_ < maxModelNum);

	ModelInstance* instance = modelInstances[currentDrawModelIndex_].get();
	currentDrawModelIndex_++;
	return instance;
}

Material Renderer::MakeMaterial(const Vector4& color, const Matrix4x4& uvTransform) const {
	Material material{};
	material.color = color;
	material.lightingType = static_cast<int32_t>(lightingType_);
	material.uvTransform = uvTransform;
	material.reflectionType = static_cast<int32_t>(reflectionType_);
	material.shininess = 40.0f;
	return material;
}

D3D12_GPU_DESCRIPTOR_HANDLE Renderer::GetWhiteTextureHandle() {
	return TextureManager::GetInstance()->GetTextureInfo("white_template").textureSrvHandlesGPU;
}

void Renderer::DrawMesh(ModelInstance* instance, const Material& material, const Microsoft::WRL::ComPtr<ID3D12Resource>& wvpResource,
	const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView, const D3D12_INDEX_BUFFER_VIEW* indexBufferView,
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, uint32_t drawCount) {
	*instance->materialData_ = material;

	// DrawCommandは非constのポインタを受け取るのでコピーを渡す.
	D3D12_VERTEX_BUFFER_VIEW vbv = vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW ibv{};
	if (indexBufferView) {
		ibv = *indexBufferView;
	}

	/*=============================================================
	三角形の描画のコマンド.
	=============================================================*/
	GameSystem::GetInstance()->DrawCommand(
		blendMode_,
		ShaderType::kNormal,
		&vbv,
		indexBufferView ? &ibv : nullptr,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		instance->materialResource_,
		wvpResource,
		textureHandle,
		drawCount
	);
}

void Renderer::DrawPrimitive(const Matrix4x4& worldMatrix, const Material& material,
	const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView, const D3D12_INDEX_BUFFER_VIEW* indexBufferView,
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, uint32_t drawCount) {
	ModelInstance* instance = AcquireModelInstance();
	*instance->wvpData_ = MakeTransformationMatrix(Matrix4x4::Identity(), worldMatrix);
	DrawMesh(instance, material, instance->wvpResource_, vertexBufferView, indexBufferView, textureHandle, drawCount);
}

void Renderer::DrawModelInfo(const TransformationMatrix& matrix, const Matrix4x4& worldMatrix, const ModelInfo& modelInfo, const Vector4* overrideColor, bool useTransparent) {
	const uint32_t meshCount = static_cast<uint32_t>(modelInfo.modelData.size());
	if (meshCount == 0) {
		return;
	}

	// 行列は全メッシュ共通なので1つだけ書き込み、全メッシュで同じResourceを使う.
	ModelInstance* matrixInstance = AcquireModelInstance();
	*matrixInstance->wvpData_ = matrix;

	Matrix4x4 world = worldMatrix;
	Vector3 worldTranslate = world.GetMatrixToTranslate();

	for (uint32_t i = 0; i < meshCount; i++) {
		// ModelDataはコピーせずに参照する(頂点データのコピーが発生しない).
		const ModelData& data = modelInfo.modelData[i];

		Material material = MakeMaterial(overrideColor ? *overrideColor : data.materialData.matarial.color, data.materialData.matarial.uvTransform);
		if (useTransparent) {
			material.color = Camera::GetInstance()->GetTransparentColor(worldTranslate, material.color);
		}

		D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = data.textureSrvHandlesGPU;
		if (data.materialData.textureFilePath == "") {
			textureHandle = GetWhiteTextureHandle();
		}

		// 最初のメッシュは行列用に取得したCBufferのマテリアルをそのまま使う.
		ModelInstance* instance = (i == 0) ? matrixInstance : AcquireModelInstance();
		DrawMesh(instance, material, matrixInstance->wvpResource_, data.vertexBufferView_, nullptr, textureHandle, data.vertexCount);
	}
}

void Renderer::DrawModelInstance(const TransformationMatrix& matrix, const Matrix4x4& worldMatrix, const Model* model, const Vector4* overrideColor, bool useTransparent) {
	const uint32_t meshCount = model->GetModelCountMax();
	if (meshCount == 0) {
		return;
	}

	// 行列は全メッシュ共通なので1つだけ書き込み、全メッシュで同じResourceを使う.
	ModelInstance* matrixInstance = AcquireModelInstance();
	*matrixInstance->wvpData_ = matrix;
	bool isMatrixInstanceMaterialUsed = false;

	Matrix4x4 world = worldMatrix;
	Vector3 worldTranslate = world.GetMatrixToTranslate();

	for (uint32_t i = 0; i < meshCount; i++) {
		if (!model->isVisible_[i]) {
			continue;
		}

		const ModelMesh& mesh = model->meshes_[i];

		// Modelに設定されているマテリアルを使う.
		Material material = *model->materialData_[i];
		material.uvTransform = Matrix4x4::MakeAffineMatrix(model->uvTransform_[i]);
		if (overrideColor) {
			material.color = *overrideColor;
		}
		if (useTransparent) {
			material.color = Camera::GetInstance()->GetTransparentColor(worldTranslate, material.color);
		}

		// 最初に描くメッシュは行列用に取得したCBufferのマテリアルをそのまま使う.
		ModelInstance* instance = isMatrixInstanceMaterialUsed ? AcquireModelInstance() : matrixInstance;
		isMatrixInstanceMaterialUsed = true;
		DrawMesh(instance, material, matrixInstance->wvpResource_, mesh.vertexBufferView, nullptr, mesh.textureSrvHandlesGPU, mesh.vertexCount);
	}
}

void Renderer::DrawSpriteCommon(const Matrix4x4& worldMatrix, const VertexData* vertices, const Material& material, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
	// 停止中 / 一時停止中にUIを非表示にしている時は描画しない("Game"ウィンドウのUIボタン).
	if (!ImGuiManager::GetInstance()->IsGameUIVisible()) {
		return;
	}

	assert(currentDrawSpriteIndex_ < maxSpriteNum);
	SpriteInstance* instance = spriteInstances[currentDrawSpriteIndex_].get();
	currentDrawSpriteIndex_++;

	// インデックスは初期化時に書き込み済みなので、頂点・マテリアル・行列だけ書き込む.
	memcpy(instance->vertexData_, vertices, sizeof(VertexData) * 4);
	*instance->materialData_ = material;
	*instance->wvpData_ = MakeSpriteTransformationMatrix(worldMatrix);

	/*=============================================================
	三角形のSpriteの描画のコマンド.
	=============================================================*/
	GameSystem::GetInstance()->DrawCommand(
		blendMode_,
		ShaderType::kSprite,
		&instance->vertexBufferView_,
		&instance->indexBufferView_,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		instance->materialResource_,
		instance->wvpResource_,
		textureHandle,
		6
	);
}

void Renderer::DrawSprite(const Transform& transform, const TextureInfo& textureInfo, const Vector4& color) {
	// テクスチャの大きさで描画する.
	Vector2 size;
	size.x = static_cast<float>(textureInfo.width);
	size.y = static_cast<float>(textureInfo.height);

	DrawSprite(transform, size, textureInfo, color);
}

void Renderer::DrawSprite(const Transform& transform, const Vector2& size, const TextureInfo& textureInfo, const Vector4& color) {
	VertexData vertices[4];
	MakeSpriteVertices(vertices, size.x, size.y);

	DrawSpriteCommon(transform.GetAffineMatrix(), vertices, MakeSpriteMaterial(color), textureInfo.textureSrvHandlesGPU);
}

void Renderer::DrawSprite(const Transform& transform, const Sprite& sprite) {
	Vector2 size = sprite.GetSize();

	Transform worldTransform = transform;
	worldTransform.translate.x = transform.translate.x - (size.x / 2.0f);
	worldTransform.translate.y = transform.translate.y - (size.y / 2.0f);

	// Spriteが持っている頂点・マテリアル・テクスチャをそのまま使う.
	Material material = *sprite.materialData_;
	material.uvTransform = Matrix4x4::MakeAffineMatrix(sprite.uvTransform_);

	DrawSpriteCommon(worldTransform.GetAffineMatrix(), sprite.vertexData, material, sprite.textureInfo_.textureSrvHandlesGPU);
}

void Renderer::DrawShadowModel(const Transform& transform, const Model* model, const Vector4* overrideColor) {
	Matrix4x4 worldMatrix = transform.GetAffineMatrix();
	Matrix4x4 projectionMatrix = Matrix4x4::Identity();
	projectionMatrix.matrix[1][1] = 0.01f;
	worldMatrix = worldMatrix * projectionMatrix;

	TransformationMatrix matrix = MakeTransformationMatrix(model->rootMatrix_, worldMatrix);
	Matrix4x4 rootMatrix = model->rootMatrix_;
	matrix.World = rootMatrix * worldMatrix * projectionMatrix;

	DrawModelInstance(matrix, worldMatrix, model, overrideColor, false);
}

void Renderer::DrawBackGround(const Vector4& color){
	VertexData vertices[4];
	MakeSpriteVertices(
		vertices, 
		static_cast<float>(Environment::GetInstance()->GetWindowSize().width),
		static_cast<float>(Environment::GetInstance()->GetWindowSize().height)
		);
	Transform worldTransform;
	worldTransform.Initialize();
	worldTransform.translate.z = kBackGroundTranslateZ;
	DrawSpriteCommon(worldTransform.GetAffineMatrix(), vertices, MakeSpriteMaterial(color), TextureManager::GetInstance()->GetTextureInfo("white_template").textureSrvHandlesGPU);
}

void Renderer::DrawBackGround(){
	DrawBackGround({0.1f,0.25f,0.5f,1.0f});
}

void Renderer::DrawShadow(const Transform& transform, const Model* model) {
	DrawShadowModel(transform, model, nullptr);
}

void Renderer::DrawShadow(const Transform& transform, const Model* model, const Vector4& color) {
	DrawShadowModel(transform, model, &color);
}

void Renderer::DrawShadow(const Transform& transform, const ModelInfo& modelInfo, const Vector4& color) {
	Matrix4x4 worldMatrix = transform.GetAffineMatrix();
	Matrix4x4 projectionMatrix = Matrix4x4::Identity();
	projectionMatrix.matrix[1][1] = 0.0f;
	worldMatrix = worldMatrix * projectionMatrix;
	worldMatrix.matrix[3][1] = 0.01f;

	DrawModelInfo(MakeTransformationMatrix(modelInfo.rootMatrix, worldMatrix), worldMatrix, modelInfo, &color, false);
}

void Renderer::CreateLine(ModelElement* newElement) {
	newElement->modelData_.textureSrvHandlesGPU = TextureManager::GetInstance()->GetTextureInfo("white_template").textureSrvHandlesGPU;
	newElement->blendMode_ = BlendMode::kNormal;

	// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)
	newElement->vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexDataLine) * maxLineNum_);

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
	newElement->vertexBufferView_.SizeInBytes = UINT(sizeof(VertexDataLine) * maxLineNum_);
	// 1頂点あたりのサイズ.
	newElement->vertexBufferView_.StrideInBytes = sizeof(VertexDataLine);

	// 【Resourceにデータを書き込む】
	// 書き込むためのアドレスを取得.
	newElement->vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&newElement->vertexDataLine));
}

void TestParticle::Initialize(const ModelInfo& info, uint32_t numInstanced) {
	numInstance_ = numInstanced;
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

	SrvManager::GetInstance()->CreateSRVforStructureBuffer(instancingResource_.Get(), numInstance_, sizeof(TransformationMatrix));
}

void TestParticle::Draw(const Transform& transform) const {
	if (!isVisible_) {
		return;
	}

	for (uint32_t index = 0; index < numInstance_; index++) {
		Transform instancingTransform = transform;
		instancingTransform.translate = { index * 0.1f,index * 0.1f, index * 0.1f };

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
