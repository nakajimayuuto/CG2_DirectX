#include "WinApp.h"


void WinApp::Initialize() {
	/*=============================================================
	COMの初期化.
	=============================================================*/
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	/*=============================================================
	Window作成系
	=============================================================*/

	// ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;

	// ウィンドウクラス名
	wc.lpszClassName = L"CG2WindowClass";

	// インスタンスハンドル.
	wc.hInstance = GetModuleHandle(nullptr);

	// カーソル.
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する.
	RegisterClass(&wc);

	// クライアント領域のサイズ.
	int32_t kClientWidth = Environment::GetInstance()->GetWindowSize().width;
	int32_t kClientHeight = Environment::GetInstance()->GetWindowSize().height;

	// ウィンドウサイズを表す構造体に九合アント領域を入れる.
	RECT wrc{ 0,0,kClientWidth,kClientHeight };

	// クライアント領域をもとに実際のサイズにwrcを変更してもらう.
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウの生成.
	hwnd = CreateWindow(
		wc.lpszClassName,		// 利用するクラス名.
		Environment::GetInstance()->GetWindowTitle(),					// タイトルバーの文字.
		WS_OVERLAPPEDWINDOW,	// よく見るウィンドウスタイル.
		CW_USEDEFAULT,			// 表示X座標(Windowsに任せる).
		CW_USEDEFAULT,			// 表示Y座標(WindowsOSに任せる).
		wrc.right - wrc.left,	// ウィンドウ横幅.
		wrc.bottom - wrc.top,	// ウィンドウ縦幅.
		nullptr,				// 親ウィンドウハンドル.
		nullptr,				// メニューウィンドウハンドル.
		wc.hInstance,			// インスタンスハンドル.
		nullptr);				// オプション.

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		// デバッグレイヤーを有効化する.
		debugController->EnableDebugLayer();
		// さらにGPU側でもチェックを行うようにする.
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif // _DEBUG

	// ウィンドウを表示する.
	ShowWindow(hwnd, SW_SHOW);
}

bool WinApp::ProcessMessage(){
	MSG msg{};
	if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_QUIT) {
		return true;
	}

	return false;
}

void WinApp::Update() {

}

void WinApp::Finalize() {
	CloseWindow(hwnd);

	CoUninitialize();
}

LRESULT CALLBACK WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	float aspect = Environment::GetInstance()->GetAspect();

#ifdef _DEBUG


	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
#endif // _DEBUG

	RECT* rect;
	int width;
	int height;
	int newHeight;
	int newWidth;

	DWORD style;
	DWORD exStyle;
	RECT borderRect;
	int borderWidth;
	int borderHeight;

	switch (msg) {
	case WM_SIZING:
		if (
			Environment::GetInstance()->GetAspectMode() == kAspectWindowFixed ||
			Environment::GetInstance()->GetAspectMode() == kAspectWindowAndFrameFixed
			) {

			if (
				Environment::GetInstance()->GetAspectMode() == kAspectWindowAndFrameFixed &&
				Environment::GetInstance()->GetWindowMode() == kFullscreen
				) {
				break;
			}

			rect = reinterpret_cast<RECT*>(lparam);
			width = rect->right - rect->left;
			height = rect->bottom - rect->top;

			style = static_cast<DWORD>(GetWindowLongPtr(hwnd, GWL_STYLE));
			exStyle = static_cast<DWORD>(GetWindowLongPtr(hwnd, GWL_EXSTYLE));

			borderRect = { 0,0,0,0 };

			AdjustWindowRectEx(
				&borderRect,
				style,
				FALSE,
				exStyle);

			borderWidth = borderRect.right - borderRect.left;

			borderHeight = borderRect.bottom - borderRect.top;

			//Log(std::format("Rect l:{},r:{},t:{},b:{}\n", rect->left, rect->right, rect->top, rect->bottom));
			//Log(std::format("Rect w:{},h:{}\n", width, height));

			width -= borderWidth;
			height -= borderHeight;

			switch (wparam) {
			case WMSZ_LEFT:
			case WMSZ_RIGHT:
				// 左右をドラッグ → 高さを補正
				newHeight = static_cast<int>(width / aspect);
				rect->bottom = rect->top + newHeight + borderHeight;
				break;
			case WMSZ_TOP:
			case WMSZ_BOTTOM:
				// 上下をドラッグ → 幅を補正
				newWidth = static_cast<int>(height * aspect);
				rect->right = rect->left + newWidth + borderWidth;
				break;
			case WMSZ_TOPLEFT:
			case WMSZ_TOPRIGHT:
			case WMSZ_BOTTOMLEFT:
			case WMSZ_BOTTOMRIGHT:
				// 四隅ドラッグ
				newHeight = static_cast<int>(width / aspect);
				rect->bottom = rect->top + newHeight + borderHeight;
				break;
			}
			width += borderWidth;
			height += borderHeight;
		}

		return TRUE;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}