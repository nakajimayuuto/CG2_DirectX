#pragma once
#include <stdint.h>
#include <d3d12.h>
#include <wrl.h>
#include "TextureInfo.h"
class SrvManager{
public:
	static SrvManager* GetInstance();

	void Initialize();

	void PreDraw();

	void DescriptorHeapNumIncrement();

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSrvDescriptorHeap() { return descriptorHeap_; };
	uint32_t GetDescriptorSizeSRV() { return descriptorSize_; };

	TextureInfo CreateSRVforTexture2D(ID3D12Resource* pResource,DXGI_FORMAT format,UINT mipLevels,bool useMipMap);
	D3D12_GPU_DESCRIPTOR_HANDLE CreateSRVforStructureBuffer(ID3D12Resource* pResource,UINT numElements,UINT structureByteStride);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return descriptorHeap_; };

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index) {
		D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
		handleCPU.ptr += (descriptorSize_ * index);
		return handleCPU;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle( uint32_t index) {
		D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
		handleGPU.ptr += (descriptorSize_ * index);
		return handleGPU;
	}

	/// <summary>
	/// ImGui(imgui_impl_dx12)のテクスチャ用にSRVを1つ割り当てる(ImGuiManagerで使う).
	/// ImGuiはフォントの文字が増えるとテクスチャを作り直すので、古いものと新しいものが同時に存在できるよう複数用意している.
	/// </summary>
	void AllocateImGuiDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle);
	void FreeImGuiDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle);
private:
	static inline const uint32_t kDescriptorHeapNumMax = 512;

	// ヒープの先頭からImGui用に予約しておく数.
	static inline const uint32_t kImGuiDescriptorNum = 8;
	// ImGui用のSRVを使っているか.
	bool isImGuiDescriptorUsed_[kImGuiDescriptorNum] = {};

	uint32_t descriptorSize_;

	uint32_t descriptorHeapNum_ = 0;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_ = nullptr;
};

