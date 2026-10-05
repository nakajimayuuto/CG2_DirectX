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
enum class LightingType {
	kNone = 0,
	kHalfLambert = 1,
	kLambert = 2,
};

enum class ReflectionType {
	kNone = 0,
	kPhong = 1,
	kBlinnPhong = 2,
};

struct ModelElement {
	uint32_t modelMax_;

	Material* materialData_ = nullptr;

	TransformationMatrix* wvpData_ = nullptr;

	ModelData modelData_;

	VertexData* vertexData = nullptr;

	uint32_t* indexData = nullptr;

	VertexDataLine* vertexDataLine = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_ = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_ = nullptr;

	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	Transform uvTransform_;

	BlendMode blendMode_;

	uint32_t indexInstanceNum_;
};

struct ModelInstance{
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr; // 消すかも.
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_ = nullptr; // 消すかも.
};

struct SpriteInstance{
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr; // 消すかも.
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_ = nullptr; // 消すかも.
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr; // 消すかも.
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_ = nullptr; // 消すかも.
};

struct TorusInstance{
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr; // 消すかも.
	VertexData* vertexData = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr; // 消すかも.
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_ = nullptr; // 消すかも.
};

using ModelElements = std::vector<ModelElement>;

class Model {
public:
	~Model();
	void Initialize(const ModelInfo& info);
	void Initialize(const std::string& name);

	void Draw(const Transform& transform);

	void Draw(const Transform& transform,bool useTransparent);

	void SetIsVisible(bool isVisible);
	void SetIsVisible(bool isVisible, const std::string& meshName);

	bool GetIsVisible() const;
	bool GetIsVisible(const std::string& meshName) const;

	void ChangeTexture(const TextureInfo& info);
	void ChangeTexture(const TextureInfo& info, const std::string& meshName);
	void ChangeTexture(const TextureInfo& info, uint32_t index);

	void SetColor(Vector4 color);
	void SetColor(Vector4 color, const std::string& meshName);
	void SetColor(Vector4 color, uint32_t index);

	Vector4 GetColor();
	Vector4 GetColor(const std::string& meshName);
	Vector4 GetColor(uint32_t index);

	void SetUvTransform(const Transform& uvTransform);
	void SetUvTransform(const Transform& uvTransform, const std::string& meshName);
	void SetUvTransform(const Transform& uvTransform, uint32_t index);

	Transform GetUvTransform();
	Transform GetUvTransform(const std::string& meshName);
	Transform GetUvTransform(uint32_t index);

	void SetLightingType(LightingType type);
	void SetLightingType(LightingType type, const std::string& meshName);
	void SetLightingType(LightingType type, uint32_t index);

	LightingType GetLightingType();
	LightingType GetLightingType(const std::string& meshName);
	LightingType GetLightingType(uint32_t index);

	void SetBlendMode(BlendMode blendMode) { blendMode_ = blendMode; };

	uint32_t GetModelCountMax()const { return modelMax_; };

	ModelElements  GetModelElement()const;
private:
	uint32_t modelMax_;

	std::vector<Material*> materialData_;

	std::vector<TransformationMatrix*> wvpData_;

	std::vector<int> isVisible_;

	std::vector<ModelData> modelData_;

	float radius_;

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResource_;

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResource_;

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> wvpResource_;

	VertexData* vertexData = nullptr;

	std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferView_{};

	std::vector<Transform> uvTransform_;

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
	void SetAlpha(float alpha) { materialData_->color = {materialData_->color.x,materialData_->color.y,materialData_->color.z,alpha}; };

	Vector4 GetColor() { return materialData_->color; };
	float GetAlpha() { return materialData_->color.w; };

	void SetUvTransform(const Transform& transform) { uvTransform_ = transform; }
	Transform GetUvTransform() { return uvTransform_; };

	void SetSize(Vector2 size);
	void SetSize(WindowSize windowSize);

	Vector2 GetSize()const { return size_; };

	ModelElement* GetModelElement()const;

	TextureInfo GetTextureInfo() { return textureInfo_; };

	void SetBlendMode(BlendMode blendMode) { blendMode_ = blendMode; };
	void SetTranslateZ(float depth) { depth_ = depth; };
private:
	void AdaptationSize();
private:
	Vector2 size_;

	Transform uvTransform_;

	Material* materialData_ = nullptr;

	VertexData* vertexData = nullptr;

	TransformationMatrix* transformationMatrixData_ = nullptr;

	bool isVisible_;

	TextureInfo textureInfo_;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_ = nullptr;

	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	BlendMode blendMode_;

	float depth_;
};

class Renderer {
public:
	static Renderer* GetInstance();

	void Initialize();

	void SetBlendMode(BlendMode blendmode);

	void SetLightingType(LightingType lightingType);

	void SetReflectionType(ReflectionType reflectionType);

	void ClearDrawIndex();

	/// <summary>
	/// 線の描画.
	/// </summary>
	/// <param name="startVector3">始点の位置</param>
	/// <param name="endVector3">終点の位置</param>
	/// <param name="color">色</param>
	void DrawLine(const Vector3& startVector3, const Vector3& endVector3, const Vector4& color);

	/// <summary>
	/// 球の描画.
	/// </summary>
	/// <param name="transform">トランスフォーム</param>
	/// <param name="textureInfo">テクスチャインフォ</param>
	/// <param name="color">色</param>
	void DrawSphere(const Transform& transform,const TextureInfo& textureInfo, const Vector4& color);

	void DrawSphere(const Transform& transform,const TextureInfo& textureInfo, const Vector4& color,const Transform& uvTransform);

	void DrawTorus(const Transform& transform, float majorRadius, float minorRadius, const std::string& name, const Vector4& color) { DrawTorus(transform,majorRadius,minorRadius, TextureManager::GetInstance()->GetTextureInfo(name), color); };
	void DrawTorus(const Transform& transform, float majorRadius, float minorRadius,const TextureInfo& textureInfo, const Vector4& color);

	/// <summary>
	/// 球の描画.
	/// </summary>
	/// <param name="transform">トランスフォーム</param>
	/// <param name="name">テクスチャネーム</param>
	/// <param name="color">色</param>
	void DrawSphere(const Transform& transform, const std::string& name, const Vector4& color) { DrawSphere(transform,TextureManager::GetInstance()->GetTextureInfo(name), color); };

	/// <summary>
	/// ワイヤーフレームの球の描画.(まだ使えないよ！)
	/// </summary>
	/// <param name="transform">トランスフォーム</param>
	/// <param name="color">色</param>
	void DrawSphereWireFrame(const Transform& transform, const Vector4& color);

	/// <summary>
	/// ボックスの描画.
	/// </summary>
	/// <param name="transform">トランスフォーム</param>
	/// <param name="textureInfo">テクスチャインフォ</param>
	/// <param name="color">色</param>
	void DrawBox(const Transform& transform, const TextureInfo& textureInfo, const Vector4& color);
	void DrawBox(const Transform& transform, const Transform& uvTransform, const TextureInfo& textureInfo, const Vector4& color);

	/// <summary>
	/// ボックスの描画.
	/// </summary>
	/// <param name="transform">トランスフォーム</param>
	/// <param name="name">テクスチャネーム</param>
	/// <param name="color">色</param>
	void DrawBox(const Transform& transform, const std::string& name, const Vector4& color) { DrawBox(transform, TextureManager::GetInstance()->GetTextureInfo(name), color); };

	/// <summary>
	/// ワイヤーフレームのボックスの描画.
	/// </summary>
	/// <param name="transform">トランスフォーム</param>
	/// <param name="color">色</param>
	void DrawBoxWireFrame(const Transform& transform, const Vector3& size, const Vector4& color);

	void DrawPlaneWireFrame(Plane& plane);

	/// <summary>
	/// ワイヤーフレームのボックスの描画.
	/// </summary>
	/// <param name="aabb">AABB</param>
	/// <param name="color">色</param>
	void DrawBoxWireFrame(const AABB& aabb, const Vector4& color);

	/// <summary>
	/// ワイヤーフレームのボックスの描画.
	/// </summary>
	/// <param name="aao">AABB</param>
	/// <param name="color">色</param>
	void DrawBoxWireFrame(const OBB& obb, const Vector4& color);



	/// <summary>
	/// モデルの描画.
	/// </summary>
	/// <param name="transform">トランスフォーム</param>
	/// <param name="modelInfo">モデルインフォ</param>
	/// <param name="color">色</param>
	void DrawModel(const Transform& transform, const ModelInfo& modelInfo, const Vector4& color,bool useTransparent);

	/// <summary>
	/// モデルの描画.
	/// </summary>
	/// <param name="transform">トランスフォーム</param>
	/// <param name="name">モデルネーム</param>
	/// <param name="color">色</param>
	void DrawModel(const Transform& transform, const std::string& name, const Vector4& color, bool useTransparent) { DrawModel(transform, ModelManager::GetInstance()->GetModelInfo(name), color,useTransparent); };

	/// <summary>
	/// モデルの描画.
	/// </summary>
	/// <param name="transform">トランスフォーム</param>
	/// <param name="model">モデル</param>
	void DrawModel(const Transform& transform, const Model* model, bool useTransparent);

	/// <summary>
	/// スプライトの描画.
	/// </summary>
	/// <param name="transform">トランスフォーム</param>
	/// <param name="textureInfo">テクスチャインフォ</param>
	/// <param name="color">色</param>
	void DrawSprite(const Transform& transform,const TextureInfo& textureInfo, const Vector4& color);

	/// <summary>
	/// スプライトの描画.
	/// </summary>
	/// <param name="transform">トランスフォーム</param>
	/// <param name="name">モデルネーム</param>
	/// <param name="color">色</param>
	void DrawSprite(const Transform& transform, const std::string& name, const Vector4& color) { DrawSprite(transform, TextureManager::GetInstance()->GetTextureInfo(name), color); };

	/// <summary>
	/// スプライトの描画.
	/// </summary>
	/// <param name="transform">トランスフォーム</param>
	/// <param name="textureInfo">テクスチャインフォ</param>
	/// <param name="color">色</param>
	void DrawSprite(const Transform& transform,const Vector2& size,const TextureInfo& textureInfo, const Vector4& color);

	/// <summary>
	/// スプライトの描画.
	/// </summary>
	/// <param name="transform">トランスフォーム</param>
	/// <param name="name">モデルネーム</param>
	/// <param name="color">色</param>
	void DrawSprite(const Transform& transform, const Vector2& size, const std::string& name, const Vector4& color) { DrawSprite(transform,size, TextureManager::GetInstance()->GetTextureInfo(name), color); };

	/// <summary>
	/// スプライトの描画.
	/// </summary>
	/// <param name="transform">トランスフォーム</param>
	/// <param name="sprite">スプライト</param>
	void DrawSprite(const Transform& transform,const Sprite& sprite);

	void DrawShadow(const Transform& transform, const Model* model);
	void DrawShadow(const Transform& transform, const Model* model,const Vector4& color);

	void DrawShadow(const Transform& transform, const std::string& name, const Vector4& color) { DrawShadow(transform, ModelManager::GetInstance()->GetModelInfo(name), color); };
	void DrawShadow(const Transform& transform, const ModelInfo& modelInfo, const Vector4& color);

	void DrawLineAll();

	void ChangeUseDebugLine();
private:
	void CreateSphereResource();
	void CreateTorusResource();

	void CreateLine(ModelElement* newElement);

	void CreateSphere(ModelElement* newElement);

	void CreateTorus(ModelElement* newElement,float majorRadius,float minorRadius);

	void CreateBox(ModelElement* newElement);

	void CreateNewModel(ModelElements* newElements, const uint32_t modelMax);

	void CreateModel(ModelElements* newElements, const ModelElements& targetElements, const uint32_t modelMax);

	void CreateNewSprite(ModelElement* newElement,float width,float height);

	void CreateSprite(ModelElement* newElement, float width, float height);
private:
	// SphereResource.
	const uint32_t kSphereSubdivision_ = 16;
	Microsoft::WRL::ComPtr<ID3D12Resource> sphereVertexResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> sphereIndexResource_ = nullptr;
	VertexData* sphereVertexData = nullptr;
	uint32_t* sphereIndexData = nullptr;
	D3D12_VERTEX_BUFFER_VIEW sphereVertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW sphereIndexBufferView_{};

	// TorusResource.
	const uint32_t kTorusSubdivision_ = 32;
	std::vector<std::unique_ptr<TorusInstance>> torusInstances;
	Microsoft::WRL::ComPtr<ID3D12Resource> torusIndexResource_ = nullptr;
	uint32_t currentDrawTorusIndex_;
	uint32_t* torusIndexData = nullptr;
	const uint32_t maxTorusNum = 100;

	std::vector<std::unique_ptr<ModelInstance>> modelInstances;
	uint32_t currentDrawModelIndex_;
	const uint32_t maxModelNum = 300;

	std::vector<std::unique_ptr<SpriteInstance>> spriteInstances;
	uint32_t currentDrawSpriteIndex_;
	const uint32_t maxSpriteNum = 300;



	ModelElement* lineElement_;

	BlendMode blendMode_;

	LightingType lightingType_;

	ReflectionType reflectionType_;

	uint32_t currentDrawLineIndex_;
	const uint32_t maxLineNum_ = 100000;

	bool useDebugLine_ = false;
};

class TestParticle {
public:
	void Initialize(const ModelInfo& info, uint32_t numInstanced);

	void Draw(const Transform& transform) const;
private:
	uint32_t modelMax_;

	Material* materialData_;

	//TransformationMatrix* wvpData_;

	bool isVisible_;

	ModelData modelData_;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

	//Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;

	VertexData* vertexData = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	Transform uvTransform_;

	BlendMode blendMode_;


	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;

	TransformationMatrix* instancingData_ = nullptr;

	//static inline const uint32_t kNumInstance = 10;

	uint32_t numInstance_ = 10;

	TextureInfo textures_;

	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};

	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU;
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

/*
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
*/

class ModelTriangle {
public:
	void Initialize(TextureInfo info);

	void Draw(const Transform& transform);

	void SetIsVisible(bool isVisible) { isVisible_ = isVisible; };

	bool GetIsVisible() { return isVisible_; };

	void ChangeTexture(const TextureInfo& info) { textureInfo_.textureSrvHandlesGPU = info.textureSrvHandlesGPU; };

	void SetVertexPosition(const Vector3& topVertex, const Vector3& leftVertex, const Vector3& rightVertex);
	Vector3* GetVertexPosition();

	void SetColor(Vector4 color) { materialData_->color = color; };

	Vector4 GetColor() { return materialData_->color; };

	void SetLightingType(LightingType type) { materialData_->lightingType = static_cast<uint32_t>(type); };

	LightingType GetLightingType() { return static_cast<LightingType>(materialData_->lightingType); };

	void SetBlendMode(BlendMode blendMode) { blendMode_ = blendMode; };
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

