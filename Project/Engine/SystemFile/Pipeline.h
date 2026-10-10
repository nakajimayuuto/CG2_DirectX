#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <stdint.h>
#include <assert.h>
#include <array>

struct Pipeline {
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
};

enum class BlendMode {
	kNone, // ブレンドモード無し.
	kNormal, // 通常.
	kAdd, // 加算.
	kSubtract, // 減算.
	kMultily, // 乗算.
	kScreen, // スクリーン.

	kNormalCullNone, // 通常ブレンド。背面カリング無し.
	kLine, // 線の描画に使用.

	kParticleNone, // パーティクル用
	kParticleNormal, // パーティクル用の通常.
	kParticleCount,
	kCount, // ブレンドモードの最大数.
};

class Pipeline {
public:
	void Initialize();

	void CreateAllPipeline();
	/// <summary>
	/// 
	/// </summary>
	/// <param name="blendMode">ブレンドモードの設定</param>
	/// <param name="mask">Depthを書き込むかどうか(通常はD3D12_DEPTH_WRITE_MASK_ALL,パーティクルはD3D12_DEPTH_WRITE_MASK_ZERO)</param>
	/// <returns></returns>
	Pipeline PipelineCreate(BlendMode blendMode, D3D12_DEPTH_WRITE_MASK mask);
private:
	D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc(D3D12_DEPTH_WRITE_MASK mask);
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);
private:
	std::array<Pipeline, static_cast<uint32_t>(BlendMode::kCount)> object3Dpipeline_;
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = nullptr;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
};

