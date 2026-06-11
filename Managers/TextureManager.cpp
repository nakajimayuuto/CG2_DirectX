#include "TextureManager.h"
#include "../Engine/SystemFile/GameSystem.h"
TextureManager* TextureManager::GetInstance() {
	static TextureManager instance;
	return &instance;
}

TextureInfo TextureManager::RegisterTexture(const std::string& name, const std::string& filePath){
	/*=============================================================
	Texture読み込み.
	=============================================================*/
	// Textureを読んで転送する.
	DirectX::ScratchImage mipImage;
	DirectX::TexMetadata metadata;
	mipImage = LoadTexture(filePath);

	metadata = mipImage.GetMetadata();
	textures_[name].textureResource = CreateTextureResource(GameSystem::GetInstance()->GetDevice(), metadata);
	textures_[name].intermediateResource = UploadTextureData(textures_[name].textureResource, mipImage, GameSystem::GetInstance()->GetDevice(), GameSystem::GetInstance()->GetCommandList());
	textures_[name].width = mipImage.GetMetadata().width;
	textures_[name].height = mipImage.GetMetadata().height;

	// commandListをCloseし、キックしたりする(スワップチェーン無しのフレーム更新みたいなもの).
	HRESULT hr = GameSystem::GetInstance()->GetCommandList()->Close();
	assert(SUCCEEDED(hr));

	//GPUにコマンドリストの実行を行わせる.
	Microsoft::WRL::ComPtr<ID3D12CommandList> commandLists[] = { GameSystem::GetInstance()->GetCommandList() };
	GameSystem::GetInstance()->GetCommandQueue()->ExecuteCommandLists(1, commandLists->GetAddressOf());

	// Fanceの値を更新.
	GameSystem::GetInstance()->FenceValueIncrement();
	// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る.
	GameSystem::GetInstance()->GetCommandQueue()->Signal(GameSystem::GetInstance()->GetFence().Get(), GameSystem::GetInstance()->GetFenceValue());

	// Fenceの値が指定したSignal値にたどり着いているか確認する.
	// GetCompletedValueの初期値はFence作成時に渡した初期値.
	if (GameSystem::GetInstance()->GetFence()->GetCompletedValue() < GameSystem::GetInstance()->GetFenceValue()) {
		// 指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する.
		GameSystem::GetInstance()->GetFence()->SetEventOnCompletion(GameSystem::GetInstance()->GetFenceValue(), GameSystem::GetInstance()->GetFenceEvent());
		// イベント待つ.
		WaitForSingleObject(GameSystem::GetInstance()->GetFenceEvent(), INFINITE);
	}

	hr = GameSystem::GetInstance()->GetCommandAllocator()->Reset();
	assert(SUCCEEDED(hr));
	hr = GameSystem::GetInstance()->GetCommandList()->Reset(GameSystem::GetInstance()->GetCommandAllocator().Get(), nullptr);
	assert(SUCCEEDED(hr));

	/*=============================================================
	ShaderResourceViewを作る.
	=============================================================*/
	// metaDataを基にSRVの設定.

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ.
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	// SRVを生成するDescriptorHeapを決める.
	textures_[name].textureSrvHandlesCPU = GameSystem::GetCPUDescriptorHandle(GameSystem::GetInstance()->GetSrvDescriptorHeap(), GameSystem::GetInstance()->GetDescriptorSizeSRV(), GameSystem::GetInstance()->GetSrvDescriptorHeapNum());
	textures_[name].textureSrvHandlesGPU = GameSystem::GetGPUDescriptorHandle(GameSystem::GetInstance()->GetSrvDescriptorHeap(), GameSystem::GetInstance()->GetDescriptorSizeSRV(), GameSystem::GetInstance()->GetSrvDescriptorHeapNum());
	textures_[name].number = GameSystem::GetInstance()->GetSrvDescriptorHeapNum();

	// SRVの生成.
	GameSystem::GetInstance()->GetDevice()->CreateShaderResourceView(textures_[name].textureResource.Get(), &srvDesc, textures_[name].textureSrvHandlesCPU);
	GameSystem::GetInstance()->SrvDescriptorHeapNumIncrement();

	return textures_[name];
}

TextureInfo TextureManager::GetTextureInfo(const std::string& name){
	auto it = textures_.find(name);

	assert(it != textures_.end());
	return it->second;
}


DirectX::ScratchImage TextureManager::LoadTexture(const std::string& filePath) {
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

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metaData) {
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

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages, Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList) {

	std::vector<D3D12_SUBRESOURCE_DATA> subresource;
	DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresource);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresource.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = GameSystem::CreateBufferResource(device.Get(), intermediateSize);
	UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, UINT(subresource.size()), subresource.data());
	// Textureへの転用後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する.
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	return intermediateResource;
}
