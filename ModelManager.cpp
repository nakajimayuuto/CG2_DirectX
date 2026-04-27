#include "ModelManager.h"
#include "GameSystem.h"
#include "TextureManager.h"
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")
#pragma comment(lib,"Dbghelp.lib")


ModelManager* ModelManager::GetInstance() {
	static ModelManager instance;
	return &instance;
}

void ModelManager::RegisterObj(const std::string& name, const std::string& directoryPath, const std::string& fileName) {
	if (models_.find(name) != models_.end()) {
		return;
	}

	models_[name].modelData = LoadObjFile(directoryPath, fileName);

	for (ModelData& data : models_[name].modelData) {
		TextureManager::GetInstance()->RegisterTexture(name + "_" + data.meshName, data.materialData.textureFilePath);

		data.textureSrvHandlesGPU = TextureManager::GetInstance()->GetTextureInfo(name + "_" + data.meshName).textureSrvHandlesGPU;
	}
}

//ModelData ModelManager::GetModelData(const std::string& name) {
//	auto it = models_.find(name);
//
//	assert(it != models_.end());
//	return it->second.modelData;
//}

ModelInfo ModelManager::GetModelInfo(const std::string& name) {
	auto it = models_.find(name);

	assert(it != models_.end());
	return it->second;
}

MaterialData ModelManager::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& fileName, const std::string& usemtl) {
	// 1. 中で必要となる変数の宣言.
	MaterialData materialData; // 構築するModelData.
	std::string line; // ファイルから読んだ1行を格納するもの.
	std::string mtlName;


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
	// 1. 中で必要となる変数の宣言.
	ModelData modelData; // 構築するModelData.
	std::vector<ModelData> returnData; // 構築するModelData.
	std::vector<Vector4> positions; // 位置.
	std::vector<Vector3> normals; // 法線.
	std::vector<Vector2> texcoords; // テクスチャ座標.
	std::string line; // ファイルから読んだ1行を格納するもの.

	// MaterialTemplateLiblaryファイルの名前を取得する.
	std::string materialFilename;

	// 2. ファイルを開く.
	std::ifstream file(directoryPath + "/" + fileName); // ファイルを開く.
	assert(file.is_open()); // とりあえず開けなかったら止める.


	// 3. 実際にファイルを読み、ModelDataを構築していく.
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を読む.

		// identifierに応じた処理.

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.x *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);
		} else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		} else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
		} else if (identifier == "f") {
			VertexData triangle[3];
			// 面は三角形限定。その他は未対応.
			for (uint32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する.
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (uint32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');// 区切りでインデックスを読んでいく.
					elementIndices[element] = std::stoi(index);
				}

				if (positions.size() < 0 || faceVertex > positions.size()) {
					assert(false);
				}

				if (texcoords.size() < 0 || faceVertex > texcoords.size()) {
					assert(false);
				}

				if (normals.size() < 0 || faceVertex > normals.size()) {
					//	assert(false);
				}


				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築する.
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				triangle[faceVertex] = { position,texcoord,normal };
			}

			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib") {
			s >> materialFilename;
			//// 基本的にObjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す.
			//modelData.materialData = LoadMaterialTemplateFile(directoryPath, materialFilename,);
		} else if (identifier == "usemtl") {
			s >> modelData.materialData.textureName;
			// 基本的にObjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す.
			modelData.materialData = LoadMaterialTemplateFile(directoryPath, materialFilename, modelData.materialData.textureName);
		} else if (identifier == "o") {
			if (positions.size() != 0) {
				returnData.push_back(modelData);

				modelData.vertices.clear();
				modelData.materialData.textureFilePath = "";
				modelData.materialData.textureName = "";
			}

			s >> modelData.meshName;
		}
	}

	// 4. ModelDataを返す.
	returnData.push_back(modelData);


	return returnData;
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
