#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <stdint.h>
#include <assert.h>
#include <array>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <string>

struct PipelineData {
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
};

enum class ShaderType {
	kNormal, // オブジェクト3D.
	kNoTexture, // テクスチャ無しモデル.
	kLine, // テクスチャ無しモデル.
	kSprite, // スプライト.
	kParticle, // パーティクル.
	kCount // 最大数.
};

enum class BlendMode {
	kNone, // ブレンドモード無し.
	kNormal, // 通常.
	kAdd, // 加算.
	kSubtract, // 減算.
	kMultily, // 乗算.
	kScreen, // スクリーン.

	//kNormalCullNone, // 通常ブレンド。背面カリング無し.
	//kLine, // 線の描画に使用.

	//kParticleNone, // パーティクル用
	//kParticleNormal, // パーティクル用の通常.
	//kParticleCount,
	kCount, // ブレンドモードの最大数.
};

class Pipeline {
private:
	static inline const D3D12_FILTER kUsingFilter_ = D3D12_FILTER_MIN_MAG_MIP_POINT; // D3D12_FILTER_MIN_MAG_MIP_LINEAR.
public:
	void Initialize();

	void CreateAllPipeline();
	/// <summary>
	/// 
	/// </summary>
	/// <param name="blendMode">ブレンドモードの設定</param>
	/// <param name="shaderType"></param>
	/// <param name="depthMask">Depthを書き込むかどうか(通常はD3D12_DEPTH_WRITE_MASK_ALL,パーティクルはD3D12_DEPTH_WRITE_MASK_ZERO)</param>
	/// <param name="usingFilter">描画時の色合成の形式(通常はLinerでマイクラ的にしたければPoint)</param>
	/// <param name="cullMode">カリングのタイプ(基本back,たまにnone)</param>
	/// <returns></returns>
	PipelineData PipelineCreate(BlendMode blendMode,ShaderType shaderType, D3D12_DEPTH_WRITE_MASK depthMask, D3D12_FILTER usingFilter, D3D12_CULL_MODE cullMode);
	
	PipelineData GetPSO(BlendMode blendMode,ShaderType shaderType);
	PipelineData GetObject3dPSO(BlendMode blendMode) { return pipelineObject3d_[static_cast<uint32_t>(blendMode)]; };
	PipelineData GetSpritePSO(BlendMode blendMode) { return pipelineSprite_[static_cast<uint32_t>(blendMode)]; };
	PipelineData GetParticlePSO(BlendMode blendMode) { return pipelineParticle_[static_cast<uint32_t>(blendMode)]; };
	PipelineData GetLinePSO() { return pipelineLine_; };
private:
	std::array<PipelineData, static_cast<size_t>(BlendMode::kCount)> pipelineObject3d_;
	std::array<PipelineData, static_cast<size_t>(BlendMode::kCount)> pipelineSprite_;
	std::array<PipelineData, static_cast<size_t>(BlendMode::kCount)> pipelineParticle_;
	PipelineData pipelineLine_;
private:
	struct ShaderBlobs {
		// 【VertexShader】
		Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob;
		// 【PixelShader】
		Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob;
	};

	D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc(D3D12_DEPTH_WRITE_MASK mask, ShaderType shaderType);

	void CreateDescriptorRange(D3D12_DESCRIPTOR_RANGE* descriptorRange,UINT baseShaderRegister,UINT numDescriptors);

	void CreateConstantBufferView(D3D12_ROOT_PARAMETER& rootParameter, D3D12_SHADER_VISIBILITY shaderVisibility,UINT descriptorShaderRegister);

	void CreateDescriptorTable(D3D12_ROOT_PARAMETER& rootParameter, D3D12_SHADER_VISIBILITY shaderVisibility, D3D12_DESCRIPTOR_RANGE* descriptorRange, UINT numDescriptorRanges);

	void CreateRootParameters(D3D12_ROOT_SIGNATURE_DESC& rootSignature,ShaderType shaderType);
	
	void CreateStaticSamplers(D3D12_ROOT_SIGNATURE_DESC& rootSignature,ShaderType shaderType,D3D12_FILTER filter);

	D3D12_INPUT_LAYOUT_DESC CreateInputRayout(ShaderType shaderType);

	void SelectShaderBlobs(ShaderBlobs& shaderBlobs,ShaderType shaderType);
public:
	// CompileShader関数(どうやってファイル分けするかね).
	static IDxcBlob* CompileShader(
		// CompilerするShaderファイルへのパス.
		const std::wstring& filePath,
		// Compilerに使用するProfile.
		const wchar_t* profile,
		// 初期化で生成したものを3つ.
		IDxcUtils* dxcUtils,
		IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler);
private:
	void DXCompilerInitialize();

	void ShaderCompile();

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = nullptr;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
private:
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_ = nullptr;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_ = nullptr;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_ = nullptr;

	// Shaderをコンパイルする.
	// 【VertexShader】
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob3dObject;
	// 【PixelShader】
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob3dObject;
	// 【VertexShader】
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlobParticle;
	// 【PixelShader】
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlobParticle;
	// 【VertexShader】
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlobLine;
	// 【PixelShader】
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlobLine;
	// 【VertexShader】
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlobNoTexture;
	// 【PixelShader】
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlobNoTexture;
private:

	D3D12_ROOT_PARAMETER rootParameters[8] = {};
	D3D12_ROOT_PARAMETER rootParametersNoTexture[7] = {};
	D3D12_ROOT_PARAMETER rootParametersParticle[3] = {};
	D3D12_ROOT_PARAMETER rootParametersLine[1] = {};

	//D3D12_ROOT_PARAMETER rootParameter;

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};


	D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	D3D12_DESCRIPTOR_RANGE spotLightDescriptorRange[1] = {};
	D3D12_DESCRIPTOR_RANGE pointLightDescriptorRange[1] = {};

	D3D12_INPUT_ELEMENT_DESC inputElementalDescs[3] = {};
	D3D12_INPUT_ELEMENT_DESC inputElementalDescsLine[2] = {};
	D3D12_INPUT_ELEMENT_DESC inputElementalDescsNoTexture[2] = {};
};

