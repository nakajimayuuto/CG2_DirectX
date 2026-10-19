#include "Satlib.h"
#include "Application.h"
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	std::unique_ptr<Framework> application = std::make_unique<Application>();
	application->Run();
	return 0;
}