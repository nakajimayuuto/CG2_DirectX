#pragma once

#include <Windows.h>
#include <cstdint>
#include "ModelManager.h"
#include "TextureManager.h"
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

	namespace Renderer {
	class Model {
	public:
		void Initialize(ModelInfo info);

		void Draw();
	public:
		Material* materialData_ = nullptr;

		TransformationMatrix* wvpData_ = nullptr;
	private:
		ModelInfo modelInfo_;

		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_ = nullptr;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	};

	class Sphere {
	public:
		void Initialize(TextureInfo info);

		void Draw();

		Material* materialData_ = nullptr;

		TransformationMatrix* wvpData_ = nullptr;
	private:
		const uint32_t kSubdivision_ = 16;

		TextureInfo textureInfo_;

		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_ = nullptr;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

		Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_ = nullptr;

		D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	};

	class Sprite {
	public:
		void Initialize(TextureInfo info);

		void Draw();

		Material* materialData_ = nullptr;

		TransformationMatrix* transformationMatrixData_ = nullptr;
	private:
		const uint32_t kSubdivision_ = 16;

		TextureInfo textureInfo_;

		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

		Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_ = nullptr;

		D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	};
}

