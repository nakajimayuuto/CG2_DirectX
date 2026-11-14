#pragma once
#include <Windows.h>
#include <cstdint>
#include "../../Managers/ModelManager.h"
#include "../../Managers/TextureManager.h"
#include "../Environment/Environment.h"
#include "../../externals/DirectXTex/DirectXTex.h"
#include "../../externals/DirectXTex/d3dx12.h"
#include "../SystemFile/GameSystem.h"
#include "../Math/Matrix4x4.h"
#include "../Math/Field.h"
#include "PrimitiveMesh.h"
#include <list>
#include <memory>

struct ParticleData {
	Transform transform;
	Vector3 posBlank;
	Transform targetTransform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;

	// ノードで作った動き(ParticleNodeMove)で使う値.
	Vector3 startPosition{};          // 生成位置.
	Vector3 startVelocity{};          // 初速(MakeNewParticleで渡したvelocity).
	float startLifeTime = 0.0f;       // MakeNewParticleで渡したlifeTime.
	Vector4 startColor{};             // 初期色.
	Vector3 baseScale{};              // 基準の大きさ(SetParticleSizeの値).
	Vector3 scaleRate{ 1.0f,1.0f,1.0f }; // 基準の大きさに掛ける倍率.
	float random = 0.0f;              // パーティクルごとに固定の乱数(0〜1).
	uint32_t randomSeed = 0;          // "生成時乱数"ノードの値を決める種(パーティクルごとに固定).

	// 出現時の色(生成した時の Particles::SetSpawnColor の値). 描画時に color に掛ける.
	Vector4 spawnColor{ 1.0f,1.0f,1.0f,1.0f };
};

class ParticleNodeMove;

struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

enum class BillboardType {
	kNone,
	kAllAxis, // 今はこれだけ対応.
	kOnlyX,
	kOnlyY,
	kOnlyZ,
};

class Particles{
public:
	enum class Move {
		kNormal,
		kFire,
		kSlash,
		kExplode,
		kExplodeMonochrome,
		kCharge,
		kNumber,


		kNodeMove, // ノードの時に使うやつです！それ以外の時には使わないで！
	};

	~Particles();
	void Initialize(const ModelInfo& info);

	void Initialize(const TextureInfo& info);

	/// <summary>
	/// Primitive図形(Rendererで描画できる図形と同じもの)をパーティクルにする.
	/// </summary>
	/// <param name="type">図形の種類</param>
	/// <param name="info">貼るテクスチャ</param>
	/// <param name="param">半径などのパラメータ(Torus / Ring / Cylinder のみ使用)</param>
	void Initialize(PrimitiveType type, const TextureInfo& info, const PrimitiveParam& param = {});

	void MakeNewParticle(const Vector3& position);
	void MakeNewParticle(const Vector3& position, const Vector3& blank);
	void MakeNewParticle(const Vector3& position,const Vector3& velocity,float lifeTime,const Vector3& targetPosition, const Vector3& blank);

	void MakeNewParticle(const Transform& transform);

	void Update();

	void Draw();

	void SetBillboardType(BillboardType billboardType);
	void SetMoveType(Move moveType);

	/// <summary>
	/// ノードで作った動きを使う(nullptrなら Move の動きに戻る).
	/// </summary>
	void SetNodeMove(const std::shared_ptr<ParticleNodeMove>& nodeMove) { nodeMove_ = nodeMove; if (nodeMove_) { moveType_ = Move::kNodeMove; } };
	const std::shared_ptr<ParticleNodeMove>& GetNodeMove() const { return nodeMove_; };

	BillboardType GetBillboardType() { return billboardType_; };
	
	void SetSize(const Vector3 size) { size_ = size; };

	void CheckCollision(const Field& field);

	void ClearParticle() { particleData_.clear(); };

	// 今出ているパーティクルの数.
	size_t GetParticleCount() const { return particleData_.size(); };

	void SetUvTransform(Transform transform) { uvTransform_ = transform; };

	void SetColor(const Vector3& color) { colorNoAlpha_ = color; };

	/// <summary>
	/// 出現時の色(これから出すパーティクルの色に掛ける色. 白なら今まで通り).
	/// 出した後に変えても、出ているパーティクルの色は変わらない.
	/// </summary>
	void SetSpawnColor(const Vector4& color) { spawnColor_ = color; };
	const Vector4& GetSpawnColor() const { return spawnColor_; };
private:
	/// <summary>
	/// 各Initializeで共通の処理(マテリアル・インスタンシング用バッファの生成など).
	/// </summary>
	void InitializeCommon(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, const Vector4& color, const Matrix4x4& uvTransform);

	void MoveNormal();

	void MoveFire();

	void MoveExplode();

	void MoveCharge();

	void MoveNumber();
private:
	// SetParticleSizeを呼ばなくても見えるよう等倍にしておく.
	Vector3 size_{ 1.0f,1.0f,1.0f };

	// 出現時の色(SetSpawnColor).
	Vector4 spawnColor_{ 1.0f,1.0f,1.0f,1.0f };

	Move moveType_;

	// ノードで作った動き(設定されていれば moveType_ より優先する).
	std::shared_ptr<ParticleNodeMove> nodeMove_;

	static inline const uint32_t kNumMaxInstance = 1000;

	Material* materialData_;
	bool isVisible_;
	std::list<ParticleData> particleData_;

	Matrix4x4 billboardMatrix_;


	// 描画する形(モデル / 板ポリ / Primitive図形).
	PrimitiveMesh mesh_;

	// 貼るテクスチャ.
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle_{};

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

	Transform uvTransform_;

	BlendMode blendMode_;

	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;

	ParticleForGPU* instancingData_ = nullptr;


	uint32_t numInstance_;

	TextureInfo textures_;

	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};

	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU;

	BillboardType billboardType_;

	Vector3 colorNoAlpha_;
	//std::list<ParticleData> particleData_;
};

enum class EmitterShape {
	kBox,
	kSphere,
};

class Emitter {
public:
	void Initialize(const Transform& transform,uint32_t count,float frequency);

	void SetParticle(Particles* particles) { particles_ = particles; };

	void SetTransform(const Transform& transform) { transform_ = transform; };

	void SetCount(uint32_t count) { count_ = count; };

	// 生成する間隔(秒). 0未満なら自動では生成しない(CreateParticleを呼んだ時だけ生成する).
	void SetFrequency(float frequency) { frequency_ = frequency; useTimer_ = frequency >= 0.0f; };

	// GUIで調整する用.
	Transform& GetTransform() { return transform_; };
	uint32_t GetCount() const { return count_; };
	float GetFrequency() const { return frequency_; };
	EmitterShape GetShape() const { return shape_; };
	Particles* GetParticles() const { return particles_; };

	void CreateParticle();

	void Update();

	void DebugDraw();

	void SetShape(EmitterShape shape) { shape_ = shape; };
private:
	EmitterShape shape_;

	Particles* particles_ = nullptr;

	Transform transform_;
	uint32_t count_;
	float frequency_;
	float frequencyTime_;

	bool useTimer_;
};