#pragma once

#include <Windows.h>
#include <cstdint>
#include "../../Managers/ModelManager.h"
#include "../../Managers/TextureManager.h"
#include "../../Environment.h"
#include "../../externals/DirectXTex/DirectXTex.h"
#include "../../externals/DirectXTex/d3dx12.h"
#include "../SystemFile/GameSystem.h"
#include <array>

namespace Renderer {
	enum class LightingType {
		kNone = 0,
		kHalfLambert = 1,
		kLambert = 2,
	};

	class Model {
	public:
		~Model();
		void Initialize(const ModelInfo& info);

		void Draw(const Transform& transform);

		void SetIsVisible(bool isVisible);
		void SetIsVisible(bool isVisible, const std::string& meshName);

		bool GetIsVisible();
		bool GetIsVisible(const std::string& meshName);

		void ChangeTexture(const TextureInfo& info);
		void ChangeTexture(const TextureInfo& info, const std::string& meshName);

		void SetColor(Vector4 color);
		void SetColor(Vector4 color, const std::string& meshName);

		Vector4 GetColor();
		Vector4 GetColor(const std::string& meshName);

		void SetUvTransform(const Transform& uvTransform);
		void SetUvTransform(const Transform& uvTransform, const std::string& meshName);

		Transform GetUvTransform();
		Transform GetUvTransform(const std::string& meshName);

		void SetLightingType(LightingType type);
		void SetLightingType(LightingType type , const std::string& meshName);

		LightingType GetLightingType();
		LightingType GetLightingType(const std::string& meshName);

		void SetBlendMode(BlendMode blendMode) { blendMode_ = blendMode; };
	private:
		uint32_t modelMax_;

		std::vector<Material*> materialData_;

		std::vector<TransformationMatrix*> wvpData_;

		std::vector<bool> isVisible_;

		std::vector<ModelData> modelData_;

		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResource_;

		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResource_;

		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> wvpResource_;

		std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferView_{};

		std::vector<Transform> uvTransform_;

		BlendMode blendMode_;
	};

	class ModelSphere {
	public:
		void Initialize(TextureInfo info);

		void Draw(const Transform& transform);

		void SetIsVisible(bool isVisible) { isVisible_ = isVisible; };

		bool GetIsVisible() { return isVisible_; };

		void ChangeTexture(const TextureInfo& info) { textureInfo_.textureSrvHandlesGPU = info.textureSrvHandlesGPU; };

		void SetColor(Vector4 color) { materialData_->color = color; };

		Vector4 GetColor() { return materialData_->color; };

		void SetLightingType(LightingType type) { materialData_->lightingType = static_cast<uint32_t>(type); };

		LightingType GetLightingType() { return static_cast<LightingType>(materialData_->lightingType); };
	private:

		Material* materialData_ = nullptr;

		TransformationMatrix* wvpData_ = nullptr;

		bool isVisible_;

		const uint32_t kSubdivision_ = 16;

		TextureInfo textureInfo_;

		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_ = nullptr;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

		Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_ = nullptr;

		D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

		BlendMode blendMode_;
	};

	class ModelBox {
	public:
		void Initialize(const TextureInfo& info);
		void Initialize();

		void Draw(const Transform& transform);

		void SetIsVisible(bool isVisible) { isVisible_ = isVisible; };

		bool GetIsVisible() { return isVisible_; };

		void ChangeTexture(const TextureInfo& info) { modelData_.textureSrvHandlesGPU = info.textureSrvHandlesGPU; };

		void SetColor(Vector4 color) { materialData_->color = color; };

		Vector4 GetColor() { return materialData_->color; };

		void SetLightingType(LightingType type) { materialData_->lightingType = static_cast<uint32_t>(type); };

		LightingType GetLightingType() { return static_cast<LightingType>(materialData_->lightingType); };
	private:

		Material* materialData_ = nullptr;

		TransformationMatrix* wvpData_ = nullptr;

		bool isVisible_;

		ModelData modelData_;

		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;

		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

		Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

		Transform uvTransform_;

		BlendMode blendMode_;
	};

	class Sprite {
	public:
		void Initialize(TextureInfo info);
		void Initialize();

		void Draw(const Transform& transform);

		void Draw(const Transform2D& transform);

		void SetIsVisible(bool isVisible) { isVisible_ = isVisible; };

		bool GetIsVisible() { return isVisible_; };

		void ChangeTexture(const TextureInfo& info) { textureInfo_.textureSrvHandlesGPU = info.textureSrvHandlesGPU; };

		void SetColor(Vector4 color) { materialData_->color = color; };

		Vector4 GetColor() { return materialData_->color; };

		void SetUvTransform(const Transform& transform) { uvTransform_ = transform; }

		void SetSize(Vector2 size);
		void SetSize(WindowSize windowSize);

		Vector2 GetSize() { return size_; };
	private:
		void AdaptationSize();
	private:
		Vector2 size_;

		Transform uvTransform_;

		Material* materialData_ = nullptr;

		VertexData* vertexData = nullptr;

		TransformationMatrix* transformationMatrixData_ = nullptr;

		bool isVisible_;

		const uint32_t kSubdivision_ = 16;

		TextureInfo textureInfo_;

		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

		Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_ = nullptr;

		D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

		BlendMode blendMode_;
	};

	class Line {
	public:
		static Line* GetInstance();

		void Initialize();

		void Draw(const Vector3& startVector3, const Vector3& endVector3, const Vector4& color);

		void ClearDrawIndex() { currentDrawLineIndex_ = 0; };
	private:
		struct LineData {
			Material* materialData_ = nullptr;

			TransformationMatrix* wvpData_ = nullptr;

			VertexData* vertexData = nullptr;

			TextureInfo textureInfo_;

			Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;

			Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

			Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;

			D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

			Transform uvTransform_;

			BlendMode blendMode_;
		};

		static inline const uint32_t kLineMax = 500;

		uint32_t currentDrawLineIndex_;

		std::array<LineData*, kLineMax> lineDatas_;
	};

	class ModelTriangle {
	public:
		void Initialize(TextureInfo info);

		void Draw(const Transform& transform);

		void SetIsVisible(bool isVisible) { isVisible_ = isVisible; };

		bool GetIsVisible() { return isVisible_; };

		void ChangeTexture(const TextureInfo& info) { textureInfo_.textureSrvHandlesGPU = info.textureSrvHandlesGPU; };

		void SetColor(Vector4 color) { materialData_->color = color; };

		Vector4 GetColor() { return materialData_->color; };

		void SetLightingType(LightingType type) { materialData_->lightingType = static_cast<uint32_t>(type); };

		LightingType GetLightingType() { return static_cast<LightingType>(materialData_->lightingType); };
	private:

		Material* materialData_ = nullptr;

		TransformationMatrix* wvpData_ = nullptr;

		bool isVisible_;

		const uint32_t kSubdivision_ = 16;

		TextureInfo textureInfo_;

		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_ = nullptr;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

		VertexData* vertexData = nullptr;

		BlendMode blendMode_;
	};
}

