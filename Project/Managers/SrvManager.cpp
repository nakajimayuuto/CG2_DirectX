#include "SrvManager.h"
#include "../Engine/SystemFile/GameSystem.h"
#include <cassert>
#pragma comment(lib,"d3d12.lib")
SrvManager* SrvManager::GetInstance() {
	static SrvManager instance;
	return &instance;
}

void SrvManager::Initialize() {
	// 先頭はImGui用に予約しておく.
	descriptorHeapNum_ = kImGuiDescriptorNum;

	descriptorHeap_ = GameSystem::GetInstance()->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kDescriptorHeapNumMax,true);

	descriptorSize_ = GameSystem::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void SrvManager::PreDraw(){
	// 描画用のDescriptorHeapの設定.
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { descriptorHeap_.Get() };
	GameSystem::GetInstance()->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());
}

void SrvManager::AllocateImGuiDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle) {
	for (uint32_t index = 0; index < kImGuiDescriptorNum; ++index) {
		if (!isImGuiDescriptorUsed_[index]) {
			isImGuiDescriptorUsed_[index] = true;
			*outCpuHandle = GetCPUDescriptorHandle(index);
			*outGpuHandle = GetGPUDescriptorHandle(index);
			return;
		}
	}
	// 予約した数を使い切った(kImGuiDescriptorNumを増やす).
	assert(false && "ImGui用のSRVを使い切りました(SrvManager::kImGuiDescriptorNum)");
	*outCpuHandle = GetCPUDescriptorHandle(0);
	*outGpuHandle = GetGPUDescriptorHandle(0);
}

void SrvManager::FreeImGuiDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) {
	const SIZE_T start = descriptorHeap_->GetCPUDescriptorHandleForHeapStart().ptr;
	if (cpuHandle.ptr < start) {
		return;
	}
	const uint32_t index = static_cast<uint32_t>((cpuHandle.ptr - start) / descriptorSize_);
	if (index < kImGuiDescriptorNum) {
		isImGuiDescriptorUsed_[index] = false;
	}
}

void SrvManager::DescriptorHeapNumIncrement() {
	if (descriptorHeapNum_ >= kDescriptorHeapNumMax) {
#ifdef _DEBUG

		assert(false && "現在使えるsrvDescriptorHeapのサイズ(kDescriptorHeapNumMax)を使い切りました");

#endif // _DEBUG
		return;
	}
	descriptorHeapNum_++;
}

TextureInfo SrvManager::CreateSRVforTexture2D(ID3D12Resource* pResource, DXGI_FORMAT format, UINT mipLevels, bool useMipMap){
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	TextureInfo info;
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ.
	if (useMipMap) {
		srvDesc.Texture2D.MipLevels = UINT(mipLevels);
	} else {
		srvDesc.Texture2D.MipLevels = 1;
	}

	// SRVを生成するDescriptorHeapを決める.
	info.textureSrvHandlesCPU = GetCPUDescriptorHandle(descriptorHeapNum_);
	info.textureSrvHandlesGPU = GetGPUDescriptorHandle(descriptorHeapNum_);
	info.number = descriptorHeapNum_;

	// SRVの生成.
	GameSystem::GetInstance()->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, info.textureSrvHandlesCPU);
	DescriptorHeapNumIncrement();
	return info;
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::CreateSRVforStructureBuffer( ID3D12Resource* pResource, UINT numElements, UINT structureByteStride){
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.StructureByteStride = structureByteStride;
	D3D12_GPU_DESCRIPTOR_HANDLE descriptorHandleGPU = GetGPUDescriptorHandle(descriptorHeapNum_);
	// SRVの生成.
	GameSystem::GetInstance()->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(descriptorHeapNum_));
	DescriptorHeapNumIncrement();

	return descriptorHandleGPU;
}
