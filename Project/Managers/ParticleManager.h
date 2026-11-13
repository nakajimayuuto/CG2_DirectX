#pragma once
#include <map>
#include <string>
#include <vector>
#include "../Engine/Renderer/Particles.h"
#include "../Engine/Math/Field.h"

/// <summary>
/// CreateNewParticles(全てのオーバーロード) と SetParticleSize に渡す内容をまとめたもの.
/// ParticleManager::ApplyParticleSettings に渡すとパーティクルが作られる(GUIのパーティクル管理で使う).
/// </summary>
struct ParticleSettings {
	// 何を描画するか(CreateNewParticles のどのオーバーロードにあたるか).
	enum class Shape {
		kTexture,   // テクスチャを貼った板ポリ(TextureInfo / テクスチャ名 を渡すもの).
		kModel,     // モデル(ModelInfo を渡すもの).
		kPrimitive, // Primitive図形(PrimitiveType を渡すもの).
	};

	std::string name;
	Shape shape = Shape::kTexture;
	// kTexture / kPrimitive で貼るテクスチャ(TextureManagerに登録した名前).
	std::string textureName = "white_template";
	// kModel で使うモデル(ModelManagerに登録した名前. メッシュが1つのもの).
	std::string modelName;
	// kPrimitive の図形と半径などのパラメータ.
	PrimitiveType primitiveType = PrimitiveType::kPlane;
	PrimitiveParam primitiveParam{};

	BillboardType billboardType = BillboardType::kAllAxis;
	// true ならノードで作った動き(nodeMoveName)、false なら moveType の動き.
	bool useNodeMove = false;
	Particles::Move moveType = Particles::Move::kNormal;
	std::string nodeMoveName;

	// SetParticleSize の値.
	Vector3 size{ 1.0f,1.0f,1.0f };
};

class ParticleManager{
public:
	// Primitive図形の種類(Rendererと共通の PrimitiveType をそのまま使う).
	// ParticleManager::Primitive::kSphere のように今まで通り書ける.
	using Primitive = PrimitiveType;
	static ParticleManager* GetInstance();

	void Initialize();

	void Update();

	void Draw();

	void ParticleClear();

	void SpawnParticles(const std::string& name,const Transform transform);
	void SpawnParticles(const std::string& name,const Vector3 vector3);
	/// <summary>
	/// 初速 / 寿命 / 目標地点(生成位置から見たローカル座標) を指定してパーティクルを出す.
	/// ノードで作った動きのパーティクルでは、"パーティクル"ノードの 初速 / 寿命 / 目標地点 になる.
	/// </summary>
	void SpawnParticles(const std::string& name, const Vector3& position, const Vector3& velocity, float lifeTime, const Vector3& targetPosition);
	void SpawnNumbers(float number, const Transform transform,const Vector3& color);

	void CreateNewParticles(const std::string& name ,const TextureInfo& info);
	void CreateNewParticles(const std::string& name ,const TextureInfo& info,BillboardType billType,Particles::Move moveType);
	void CreateNewParticles(const std::string& name, const ModelInfo& info);
	void CreateNewParticles(const std::string& name, const std::string& textureName, BillboardType billType, Particles::Move moveType) { CreateNewParticles(name, TextureManager::GetInstance()->GetTextureInfo(textureName), billType, moveType); };

	/// <summary>
	/// Primitive図形のパーティクルを作る.
	/// </summary>
	/// <param name="name">パーティクルの名前</param>
	/// <param name="primitive">図形の種類</param>
	/// <param name="info">貼るテクスチャ</param>
	/// <param name="param">半径などのパラメータ(Torus / Ring / Cylinder のみ使用)</param>
	void CreateNewParticles(const std::string& name, Primitive primitive, const TextureInfo& info, const PrimitiveParam& param = {});
	void CreateNewParticles(const std::string& name, Primitive primitive, const TextureInfo& info, BillboardType billType, Particles::Move moveType, const PrimitiveParam& param = {});
	void CreateNewParticles(const std::string& name, Primitive primitive, const std::string& textureName, const PrimitiveParam& param = {}) { CreateNewParticles(name, primitive, TextureManager::GetInstance()->GetTextureInfo(textureName), param); };
	void CreateNewParticles(const std::string& name, Primitive primitive, const std::string& textureName, BillboardType billType, Particles::Move moveType, const PrimitiveParam& param = {}) { CreateNewParticles(name, primitive, TextureManager::GetInstance()->GetTextureInfo(textureName), billType, moveType, param); };

	/*=============================================================
	ノードで作った動きを使うパーティクル(Particles::Moveの代わりに動きの名前を指定する)
	動きの名前は Resources/Node/Particle に保存されている .json を抜いたファイル名.
	=============================================================*/
	void CreateNewParticles(const std::string& name, const TextureInfo& info, BillboardType billType, const std::string& moveName);
	void CreateNewParticles(const std::string& name, const std::string& textureName, BillboardType billType, const std::string& moveName) { CreateNewParticles(name, TextureManager::GetInstance()->GetTextureInfo(textureName), billType, moveName); };
	void CreateNewParticles(const std::string& name, Primitive primitive, const TextureInfo& info, BillboardType billType, const std::string& moveName, const PrimitiveParam& param = {});
	void CreateNewParticles(const std::string& name, Primitive primitive, const std::string& textureName, BillboardType billType, const std::string& moveName, const PrimitiveParam& param = {}) { CreateNewParticles(name, primitive, TextureManager::GetInstance()->GetTextureInfo(textureName), billType, moveName, param); };

	// name のパーティクルが作られているか.
	bool HasParticles(const std::string& name) const;
	// 作られているパーティクルの名前の一覧(エディター用の "__" から始まるものは除く).
	std::vector<std::string> GetParticleNames() const;
	/// <summary>
	/// settings の内容でパーティクルを作る. 同じ名前のパーティクルがあれば同じオブジェクトのまま作り直す
	/// (GetParticlesで受け取ったポインタやEmitterに設定したものはそのまま使える).
	/// 描画に使っている頂点バッファを作り直すことがあるので、描画コマンドを積んでいる途中では呼ばないこと.
	/// </summary>
	/// <param name="settings">作る内容</param>
	/// <param name="isRebuildShape">falseなら形とテクスチャは作り直さず、ビルボード / 動き / 大きさだけ変える(作られていない時は作る)</param>
	void ApplyParticleSettings(const ParticleSettings& settings, bool isRebuildShape = true);

	// name のパーティクルを消す(CreateNewEmitterで作ったEmitterに設定されていれば、そのEmitterは何も出さなくなる).
	void RemoveParticles(const std::string& name);

	void CreateNewEmitter(const std::string& name, const std::string& setParticleName, const Transform& transform, uint32_t count, float frequency);
	void CreateNewEmitter(const std::string& name, const std::string& setParticleName);

	void CreateNewField(const std::string& name, const AABB& aabb, const Vector3& acceleration);

	void SetBillboardType(BillboardType type);

	void SetBillboardType(const std::string& name, BillboardType type);

	void SetParticleSize(const std::string& name,const Vector3 size);
	void SetParticleUVTransform(const std::string& name,const Transform& transform);

	void SetEmitterTransform(const std::string& name, const Transform& transform);

	void SetEmitterCount(const std::string& name, uint32_t count);

	void SetEmitterFrequency(const std::string& name, float frequency);

	void SetEmitterShape(const std::string& name, EmitterShape shape);

	void SetFieldArea(const std::string& name, const AABB& area);

	void SetFieldAcceleration(const std::string& name, const Vector3& acceleration);

	void SetFieldIsActive(const std::string& name, bool isActive);

	Particles* GetParticles(const std::string& name);

	void ClearParticles();

	/// <summary>
	/// InspectorのEmitterタブ(位置・大きさ・個数・間隔などの調整とギズモ)を描画する.
	/// ImGuiManager::EndFrameで呼ばれる. Release版では何もしない.
	/// </summary>
	void DrawEmitterGui();
private:
	void CheckCollision();

	// moveName のノードの動きを読み込んで particles に設定する.
	void SetNodeMove(Particles& particles, const std::string& moveName);
private:
	std::map<std::string, std::unique_ptr<Particles>> particles_;
	std::map<std::string, std::unique_ptr<Emitter>> emitters_;
	std::map<std::string, std::unique_ptr<Field>> fields_;

	// EmitterタブのGUI.
	std::string selectedEmitterName_;
	bool isEmitterGizmoVisible_ = true;
};

