#pragma once
class Framework{
public:
	virtual ~Framework() = default;

	virtual void Initialize();

	virtual void Update();

	virtual void Draw();

	virtual void Finalize();

	void Run();
};

