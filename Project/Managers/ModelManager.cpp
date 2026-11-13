#include "ModelManager.h"
#include "../Engine/SystemFile/GameSystem.h"
#include "TextureManager.h"
#include <algorithm>


#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")
#pragma comment(lib,"Dbghelp.lib")

#include <fstream>
#include <sstream>
#include <vector>

namespace {
	/// <summary>
	/// OBJのmtlファイルから、指定マテリアルの map_Kd に書かれた -s(スケール) / -o(オフセット) を読んでuvTransformを作る.
	/// AssimpのOBJ読み込みは -s / -o を読み飛ばすだけで AI_MATKEY_UVTRANSFORM に入れてくれないため、自前で読む.
	/// </summary>
	/// <returns>-s か -o が見つかればtrue</returns>
	bool LoadObjMtlUvTransform(const std::string& directoryPath, const std::string& objFileName, const std::string& materialName, Matrix4x4& outUvTransform) {
		// objに書かれている mtllib を集める.
		std::vector<std::string> mtlFileNames;
		{
			std::ifstream objFile(directoryPath + "/" + objFileName);
			if (!objFile.is_open()) {
				return false;
			}
			std::string line;
			while (std::getline(objFile, line)) {
				std::istringstream s(line);
				std::string identifier;
				s >> identifier;
				if (identifier == "mtllib") {
					std::string mtlFileName;
					std::getline(s >> std::ws, mtlFileName);
					// 末尾の改行コード(\r)や空白を取り除く.
					while (!mtlFileName.empty() && (mtlFileName.back() == '\r' || mtlFileName.back() == ' ')) {
						mtlFileName.pop_back();
					}
					if (!mtlFileName.empty()) {
						mtlFileNames.push_back(mtlFileName);
					}
				}
			}
		}

		for (const std::string& mtlFileName : mtlFileNames) {
			std::ifstream mtlFile(directoryPath + "/" + mtlFileName);
			if (!mtlFile.is_open()) {
				continue;
			}

			std::string line;
			std::string currentMaterial;
			while (std::getline(mtlFile, line)) {
				std::istringstream s(line);
				std::string identifier;
				s >> identifier;

				if (identifier == "newmtl") {
					s >> currentMaterial;
					continue;
				}

				if (currentMaterial != materialName || identifier != "map_Kd") {
					continue;
				}

				Vector3 scale = { 1.0f,1.0f,1.0f };
				Vector3 offset = { 0.0f,0.0f,0.0f };
				bool isFound = false;

				// map_Kd [-s u v w] [-o u v w] ファイル名.
				std::string token;
				while (s >> token) {
					if (token == "-s") {
						s >> scale.x >> scale.y >> scale.z;
						isFound = true;
					} else if (token == "-o") {
						s >> offset.x >> offset.y >> offset.z;
						isFound = true;
					} else {
						// オプション以外(ファイル名)が来たら終わり.
						break;
					}
				}

				if (isFound) {
					outUvTransform = Matrix4x4::MakeAffineMatrix(scale, { 0.0f,0.0f,0.0f }, offset);
				}
				return isFound;
			}
		}

		return false;
	}

	/// <summary>
	/// ファイル名の拡張子が .obj か(大文字小文字は区別しない).
	/// </summary>
	bool IsObjFile(const std::string& fileName) {
		if (fileName.size() < 4) {
			return false;
		}
		std::string extension = fileName.substr(fileName.size() - 4);
		std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		return extension == ".obj";
	}
}


ModelManager* ModelManager::GetInstance() {
	static ModelManager instance;
	return &instance;
}

void ModelManager::RegisterModel(const std::string& name, const std::string& directoryPath, const std::string& fileName) {
	RegisterModel(name,directoryPath,fileName,true);
}

void ModelManager::RegisterModel(const std::string& name, const std::string& directoryPath, const std::string& fileName, bool useMipMap){
	if (models_.find(name) != models_.end()) {
		return;
	}

	ModelInfo& info = models_[name];
	info.modelData = LoadModelFile(directoryPath, fileName, &info.rootMatrix);

	// 全メッシュの頂点数を数える.
	size_t totalVertexCount = 0;
	for (const ModelData& data : info.modelData) {
		totalVertexCount += data.vertices.size();
	}

	// 【VertexResourceを作成する】
	// メッシュ(マテリアル)ごとにバッファを作らず、モデル全体で1つのバッファにまとめる.
	VertexData* mappedVertexData = nullptr;
	if (totalVertexCount > 0) {
		info.vertexResource = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * totalVertexCount);
		info.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertexData));
	}

	Vector3 min = { 0.0f,0.0f,0.0f };
	Vector3 max = { 0.0f,0.0f,0.0f };
	size_t vertexOffset = 0;
	for (ModelData& data : info.modelData) {
		data.vertexCount = static_cast<uint32_t>(data.vertices.size());

		// テクスチャの登録(同じテクスチャは1回だけ読み込む).
		if (data.materialData.textureFilePath != "") {
			const std::string textureName = name + "_" + data.materialData.textureFilePath;
			TextureManager::GetInstance()->RegisterTexture(textureName, data.materialData.textureFilePath, useMipMap);

			data.textureSrvHandlesCPU = TextureManager::GetInstance()->GetTextureInfo(textureName).textureSrvHandlesCPU;
			data.textureSrvHandlesGPU = TextureManager::GetInstance()->GetTextureInfo(textureName).textureSrvHandlesGPU;
		}

		if (data.vertexCount == 0) {
			continue;
		}

		// 【VertexBufferViewを作成する】
		// 共有バッファの中の、このModelDataの範囲だけを参照する.
		data.vertexResource_ = info.vertexResource;
		data.vertexData = mappedVertexData + vertexOffset;
		data.vertexBufferView_.BufferLocation = info.vertexResource->GetGPUVirtualAddress() + sizeof(VertexData) * vertexOffset;
		data.vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * data.vertexCount);
		data.vertexBufferView_.StrideInBytes = sizeof(VertexData);

		// 【Resourceにデータを書き込む】
		memcpy(data.vertexData, data.vertices.data(), sizeof(VertexData) * data.vertexCount);
		vertexOffset += data.vertexCount;

		for (const VertexData& vertexData : data.vertices) {
			max.x = (std::max)(max.x, vertexData.position.x);
			min.x = (std::min)(min.x, vertexData.position.x);
			max.y = (std::max)(max.y, vertexData.position.y);
			min.y = (std::min)(min.y, vertexData.position.y);
			max.z = (std::max)(max.z, vertexData.position.z);
			min.z = (std::min)(min.z, vertexData.position.z);
		}
	}

	float maxSize = max.x - min.x;
	if (max.y - min.y > maxSize) {
		maxSize = max.y - min.y;
	}
	if (max.z - min.z > maxSize) {
		maxSize = max.z - min.z;
	}

	info.radius = maxSize * 1.7f;
}

const ModelInfo& ModelManager::GetModelInfo(const std::string& name) {
	auto it = models_.find(name);

	assert(it != models_.end());
	return it->second;
}

MaterialData ModelManager::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& fileName, const std::string& usemtl) {
	// 1. 中で必要となる変数の宣言.
	MaterialData materialData; // 構築するModelData.
	std::string line; // ファイルから読んだ1行を格納するもの.
	std::string mtlName;

	materialData.matarial.color = { 1.0f,1.0f,1.0f,1.0f };
	materialData.matarial.uvTransform = Matrix4x4::Identity();

	// 2. ファイルを開く.
	std::ifstream file(directoryPath + "/" + fileName); // ファイルを開く.
	assert(file.is_open()); // とりあえず開けなかったら止める.



	// 3. 実際にファイルを読み、MaterialDataを構築していく.
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を読む.

		// identifierに応じた処理.

		if (identifier == "newmtl") {
			s >> mtlName;
		}

		if (mtlName == usemtl) {
			if (identifier == "map_Kd") {
				std::string textureFilename;
				s >> textureFilename;
				if (textureFilename == "-s") {
					Vector3 scale;
					s >> scale.x >> scale.y >> scale.z;

					materialData.matarial.uvTransform = Matrix4x4::MakeScaleMatrix(scale);
					s >> textureFilename;
				}

				// 連結してファイルパスにする.
				materialData.textureFilePath = directoryPath + "/" + textureFilename;
			} else if (identifier == "Kd") {
				s >> materialData.matarial.color.x >> materialData.matarial.color.y >> materialData.matarial.color.z;
				materialData.matarial.color.w = 1.0f;
			}
		}
	}


	// 4. MaterialDataを返す.

	return materialData;
}

std::vector<ModelData>ModelManager::LoadObjFile(const std::string& directoryPath, const std::string& fileName) {
	ModelData modelData; // 構築するModelData.
	std::vector<ModelData> returnData; // 構築するModelData.

	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + fileName;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate);
	assert(scene->HasMeshes());

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());
		assert(mesh->HasTextureCoords(0));
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			//assert(face.mNumIndices == 3); // 三角形のみサポート、後々四角形対応します.
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
				VertexData vertex;
				vertex.position = { position.x,position.y,position.z,1.0f };
				vertex.normal = { normal.x,normal.y,normal.z };
				vertex.texcoord = { texcoord.x,texcoord.y };

				vertex.position.x *= -1.0f;
				vertex.normal.x *= -1.0f;

				modelData.vertices.push_back(vertex);
			}
		}
	}

	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			aiString materialName;
			aiUVTransform uvTransform;
			aiColor3D diffuseColor(1.f, 1.f, 1.f);
			material->Get(AI_MATKEY_NAME, materialName);
			material->Get(AI_MATKEY_UVTRANSFORM(aiTextureType_DIFFUSE, materialIndex), uvTransform);
			material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
			modelData.materialData.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();
			modelData.materialData.matarial.uvTransform = Matrix4x4::MakeScaleMatrix({ uvTransform.mScaling.x,uvTransform.mScaling.y,1.0f });
			modelData.materialData.matarial.color = { diffuseColor.r,diffuseColor.g,diffuseColor.b,1.0f };
			modelData.materialData.textureName = materialName.C_Str();
			//modelData.materialData = LoadMaterialTemplateFile(directoryPath,textureFilePath.C_Str(),materialName.C_Str());
			returnData.push_back(modelData);
		}
	}

	return returnData;
}

std::vector<ModelData> ModelManager::LoadModelFile(const std::string& directoryPath, const std::string& fileName, Matrix4x4* rootMatrix) {
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + fileName;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate);
	assert(scene && scene->HasMeshes());

	// 全メッシュ共通のルートノード行列.
	if (rootMatrix) {
		*rootMatrix = ReadNode(scene->mRootNode).localMatrix;
	}

	// 【マテリアルごとにModelDataを用意する】
	// 同じマテリアルを使うメッシュは1つのModelDataにまとめ、1回のドローコールで描画する.
	std::vector<ModelData> materialData(scene->mNumMaterials);
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		ModelData& modelData = materialData[materialIndex];

		aiString materialName;
		aiColor3D diffuseColor(1.f, 1.f, 1.f);
		material->Get(AI_MATKEY_NAME, materialName);
		material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);

		modelData.meshName = materialName.C_Str();
		modelData.materialData.textureName = materialName.C_Str();
		modelData.materialData.matarial.color = { diffuseColor.r,diffuseColor.g,diffuseColor.b,1.0f };
		modelData.materialData.matarial.uvTransform = Matrix4x4::Identity();

		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			aiUVTransform uvTransform;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			// 第2引数はテクスチャのインデックス(0番目のディフューズテクスチャ).
			modelData.materialData.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();

			// OBJはAssimpがmtlの -s / -o をuvTransformに入れてくれないので、mtlを直接読む.
			Matrix4x4 objUvTransform = Matrix4x4::Identity();
			if (IsObjFile(fileName) && LoadObjMtlUvTransform(directoryPath, fileName, materialName.C_Str(), objUvTransform)) {
				modelData.materialData.matarial.uvTransform = objUvTransform;
			} else if (material->Get(AI_MATKEY_UVTRANSFORM(aiTextureType_DIFFUSE, 0), uvTransform) == AI_SUCCESS) {
				// glTFなど、Assimpがuvトランスフォームを持っている場合.
				modelData.materialData.matarial.uvTransform = Matrix4x4::MakeAffineMatrix(
					{ uvTransform.mScaling.x,uvTransform.mScaling.y,1.0f },
					{ 0.0f,0.0f,uvTransform.mRotation },
					{ uvTransform.mTranslation.x,uvTransform.mTranslation.y,0.0f });
			}
		}
	}

	// 【メッシュの頂点を、使っているマテリアルのModelDataに追加する】
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());
		assert(mesh->HasTextureCoords(0));
		assert(mesh->mMaterialIndex < materialData.size());

		std::vector<VertexData>& vertices = materialData[mesh->mMaterialIndex].vertices;
		vertices.reserve(vertices.size() + static_cast<size_t>(mesh->mNumFaces) * 3);

		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
				VertexData vertex;
				vertex.position = { position.x,position.y,position.z,1.0f };
				vertex.normal = { normal.x,normal.y,normal.z };
				vertex.texcoord = { texcoord.x,texcoord.y };

				vertex.position.x *= -1.0f;
				vertex.normal.x *= -1.0f;

				vertices.push_back(vertex);
			}
		}
	}

	// 頂点を持たない(どのメッシュにも使われていない)マテリアルは除外する.
	std::vector<ModelData> returnData;
	for (ModelData& modelData : materialData) {
		if (modelData.vertices.empty()) {
			continue;
		}
		returnData.push_back(std::move(modelData));
	}

	return returnData;
}

Animation ModelManager::LoadAnimationFile(const std::string& directoryPath, const std::string& fileName){
	Animation animation;
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + fileName;
	const aiScene* scene = importer.ReadFile(filePath.c_str(),0);
	assert(scene->mAnimations != 0);
	aiAnimation* animationAssimp = scene->mAnimations[0];
	animation.duration = static_cast<float>(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);
	for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels;++channelIndex) {
		aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
		NodeAnimation<Quaternion>& nodeAnimation = animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys;++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = static_cast<float>(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { -keyAssimp.mValue.x,keyAssimp.mValue.y,keyAssimp.mValue.z };
			nodeAnimation.translate.keyframes.push_back(keyframe);
		}
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys;++keyIndex) {
			aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
			KeyframeQuaternion keyframe;
			keyframe.time = static_cast<float>(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { keyAssimp.mValue.x,-keyAssimp.mValue.y,-keyAssimp.mValue.z,keyAssimp.mValue.w };
			nodeAnimation.rotate.keyframes.push_back(keyframe);
		}
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = static_cast<float>(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { keyAssimp.mValue.x,keyAssimp.mValue.y,keyAssimp.mValue.z };
			nodeAnimation.scale.keyframes.push_back(keyframe);
		}
	}
	return animation;
}

Node ModelManager::ReadNode(aiNode* aiNode) {
	Node result;
	aiMatrix4x4 aiLoaclMatrix4x4 = aiNode->mTransformation;
	for (uint32_t i = 0; i < 4; i++) {
		for (uint32_t j = 0; j < 4; j++) {
			result.localMatrix.matrix[j][i] = aiLoaclMatrix4x4[j][i];
		}
	}

	result.name = aiNode->mName.C_Str();
	result.children.resize(aiNode->mNumChildren);
	for (uint32_t i = 0; i < aiNode->mNumChildren; i++) {
		result.children[i] = ReadNode(aiNode->mChildren[i]);
	}
	return result;
}

DirectX::ScratchImage ModelManager::LoadTexture(const std::string& filePath) {
	// テクスチャファイルを読んでプログラムを扱えるようにする.
	DirectX::ScratchImage image{};
	std::wstring filePathW = Convert::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミニマップの作成.
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// ミニマップ付きのデータを返す.
	return mipImages;
}

Microsoft::WRL::ComPtr<ID3D12Resource> ModelManager::CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metaData) {
	// 1. metadataを基にResourceの設定.
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metaData.width); // Textureの幅.
	resourceDesc.Height = UINT(metaData.height); // Textureの高さ.
	resourceDesc.MipLevels = UINT16(metaData.mipLevels); // mipmapの数.
	resourceDesc.DepthOrArraySize = UINT16(metaData.arraySize); // 奥行き or 配列Textureの配列数.
	resourceDesc.Format = metaData.format; // TextureのFormat.
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定.
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metaData.dimension); // Textureの次元数。普段使っているのは2次元.

	// 2. 利用するHeapの設定。非常に特殊な運用。02_04exで一般的なケース版がある(後々そっちに変えましょね).
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // 細かい設定を行う(03_00_exで変更した).

	// 3. Resourceを生成する.

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties, // Heapの設定.
		D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定。特になし.
		&resourceDesc, // Resource設定.
		D3D12_RESOURCE_STATE_COPY_DEST, // データ転送される設定(03_00_exで変更した).
		nullptr, // Clear最適値。使わないのでnullptr.
		IID_PPV_ARGS(&resource)); // 作成するResourceポインタへのポインタ.

	assert(SUCCEEDED(hr));

	return resource;
}

//Microsoft::WRL::ComPtr<ID3D12Resource> ModelManager::UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages, Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList) {
//
//	std::vector<D3D12_SUBRESOURCE_DATA> subresource;
//	DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresource);
//	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresource.size()));
//	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = GameSystem::CreateBufferResource(device.Get(), intermediateSize);
//	UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, UINT(subresource.size()), subresource.data());
//	// Textureへの転用後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する.
//	D3D12_RESOURCE_BARRIER barrier{};
//	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
//	barrier.Transition.pResource = texture.Get();
//	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
//	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
//	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
//	commandList->ResourceBarrier(1, &barrier);
//	return intermediateResource;
//}
