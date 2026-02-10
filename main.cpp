#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>

// ウィンドウプロシージャ.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	// メッセージに応じてゲーム固有の処理を行う.
	switch (msg) {
		//ウィンドウが破棄された.
	case WM_DESTROY:
		// OSに対して、アプリの終了を伝える.
		PostQuitMessage(0);
		return 0;
	}

	// 標準のメッセージ処理を行う.
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

// ログを表示する.
void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
};

// stringからwstringへ(配布)
std::wstring ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

// wstringからstringへ(配布)
std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	WNDCLASS wc{};

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
	const int32_t kClientWidth = 1280;
	const int32_t kClinetHeight = 720;

	// ウィンドウサイズを表す構造体に九合アント領域を入れる.
	RECT wrc{ 0,0,kClientWidth,kClinetHeight };

	// クライアント領域をもとに実際のサイズにwrcを変更してもらう.
	AdjustWindowRect(&wrc,WS_OVERLAPPEDWINDOW,false);

	// ウィンドウの生成.
	HWND hwnd = CreateWindow(
		wc.lpszClassName,		// 利用するクラス名.
		L"CG2",					// タイトルバーの文字.
		WS_OVERLAPPEDWINDOW,	// よく見るウィンドウスタイル.
		CW_USEDEFAULT,			// 表示X座標(Windowsに任せる).
		CW_USEDEFAULT,			// 表示Y座標(WindowsOSに任せる).
		wrc.right - wrc.left,	// ウィンドウ横幅.
		wrc.bottom - wrc.top,	// ウィンドウ縦幅.
		nullptr,				// 親ウィンドウハンドル.
		nullptr,				// メニューウィンドウハンドル.
		wc.hInstance,			// インスタンスハンドル.
		nullptr);				// オプション.

	// ウィンドウを表示する.
	ShowWindow(hwnd,SW_SHOW);

	MSG msg{};

	// CG2_00_05.

	// DXGIファクトリーの生成.
	IDXGIFactory7* dxgiFactory = nullptr;
	
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));

	assert(SUCCEEDED(hr));

	// 使用するアダプタ用の変数、最初にnullptrを入れておく.
	IDXGIAdapter4* useAdapter = nullptr;

	// 一番いいのを頼む.
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND;i++) {

		// アダプターの情報を取得する.
		DXGI_ADAPTER_DESC3 adapterDesc{};

		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr)); // 取得できないのは一大事.

		// ソフトウェアアダプタでなければ採用!出なければ帰れ!
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// 採用したアダプタの情報をログに出力.
			Log(ConvertString((L"Use Adapter:{}\n",adapterDesc.Description)));
			break;
		}

		useAdapter = nullptr; // ソフトウェアアダプタの場合は見なかったこととする.
	}

	// 適切なアダプタが見つからなかったので起動できない.

	ID3D12Device* device = nullptr;

	// 機能レベルとログ出力用の文字列.
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};

	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };

	// 高い順に生成できるか試していく.
	for (size_t i = 0; i < _countof(featureLevels); i++) {
		hr = D3D12CreateDevice(useAdapter, featureLevels[i],IID_PPV_ARGS(&device));

		// 指定した昨日レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr)) {
			// 生成できたログ出力を行ってループを抜ける.
			Log(std::format("FeatureLevel : {}",featureLevelStrings[i]));
			break;
		}

	}

	assert(useAdapter != nullptr);
	Log("Complete create D3D12Device!!!\n");

	// ウィンドウのxボタンが押されるまでループ.
	while (msg.message != WM_QUIT){
		// Windowにメッセージが来てたら最優先で処理させる.
		if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			// ゲームの処理.
		}
	}
}
