#pragma once
#include <string>
#include <cstdint>
#include "../Math/BaseEntity.h"
#include "../../externals/nlohmann/json.hpp"

/// <summary>
/// GUIで設置するパーティクルのエミッター(EmitterManagerが管理する).
/// BaseEntityTransformの transform_ を範囲として使う.
/// ・translate : 範囲の中心 / rotate : 範囲の向き / scale : 範囲の大きさ(箱なら辺の長さ, 球なら直径).
/// ・particleName_ のパーティクル(ParticleManagerに作ってあるもの)を、範囲の中のランダムな位置に出す.
/// </summary>
class ParticleEmitterEntity : public BaseEntityTransform {
public:
	enum class Shape {
		kBox,
		kSphere,
	};

	void Initialize() override;

	// 再生中に呼ぶ. 間隔ごとにパーティクルを出す.
	void Update() override;

	// 範囲をワイヤーフレームで描画する(Debug版のみ).
	void Draw() override;

	// Inspectorの inspectorName のタブに Transform / パーティクル / 個数 / 間隔 / 削除ボタン を出す.
	void InspectorUpdate() override;

	// 1回分(count_個)のパーティクルを出す.
	void Emit();

	// Inspectorの削除ボタンが押されたか(EmitterManagerがフレームの最後に消す).
	bool IsDeleteRequested() const { return isDeleteRequested_; };

	const std::string& GetParticleName() const { return particleName_; };
	void SetParticleName(const std::string& name) { particleName_ = name; };

	// JSONに保存 / JSONから読み込み(inspectorNameも含む).
	nlohmann::json ToJson() const;
	void FromJson(const nlohmann::json& json);

private:
	// 範囲の中のランダムな位置(ワールド座標).
	Vector3 GetRandomPosition() const;

private:
	// 出すパーティクルの名前(ParticleManager::CreateNewParticlesで作った名前).
	std::string particleName_;
	// 1回に出す個数.
	uint32_t count_ = 3;
	// 出す間隔(秒).
	float frequency_ = 0.1f;
	Shape shape_ = Shape::kBox;
	// falseなら自動では出さない.
	bool isActive_ = true;

	// パーティクルに渡す値(ノードで作った動きのパーティクルでは"パーティクル"ノードの 初速 / 寿命 / 目標地点 になる).
	Vector3 startVelocity_{ 0.0f, 0.0f, 0.0f };
	float lifeTime_ = 1.0f;
	// 生成位置から見たローカル座標.
	Vector3 targetPosition_{ 0.0f, 0.0f, 0.0f };
	// 初速 / 目標地点 をエミッターの向き(rotate)に合わせて回すか.
	bool isRotateWithEmitter_ = true;

	float timer_ = 0.0f;
	bool isDeleteRequested_ = false;
	// これまでに出したパーティクルの数(Inspectorでの確認用).
	uint32_t emittedCount_ = 0;
};
