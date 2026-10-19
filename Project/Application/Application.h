#pragma once
#include "Framework.h"
class Application : public Framework{
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void Finalize() override;
private:
	void LoadDatas();
};

