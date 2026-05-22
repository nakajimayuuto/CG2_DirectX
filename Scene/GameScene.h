#pragma once
//#include "../Engine/Renderer/Renderer.h"
//#include "../Managers/ModelManager.h"
#include "../Satlib.h"
#include "IScene.h"
#include <list>

class GameScene : public IScene{
public:
	enum class Phase{
		kTriangleSandBox,
		kTriangleMovie,
	};
	~GameScene();
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
	void CreateTriangle(const Vector3& position);

	void DeleteTriangle();
private:
	struct TestItem {
		const char* name;
		bool isSelect;

	};

	//Renderer::Model testModel_;
	//
	//Transform testTransform_;

	struct TriangleData {
		Renderer::ModelTriangle model;

		Transform transform;

		TestItem lightingType[3];

		TestItem textureType[3];

		uint32_t number;

		bool isDelete;
	};

	Renderer::ModelTriangle model_;
	
	Transform transform_;

	std::array<TriangleData,4> effectTriangleData_;

	TestItem lightingType[3];

	TestItem textureType[3];

	std::list<TriangleData> triangleDatas_;

	uint32_t triangleIndex_;

	bool isTriangleEffect_;

};

