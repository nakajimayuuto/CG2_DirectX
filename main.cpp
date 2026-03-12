#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <vector>
#include "Vector4.h"
#include "Vertex.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "Camera.h"
#include "Math.h"
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"


#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // USE_IMGUI


// ウィンドウプロシージャ.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
#ifdef USE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
#endif // USE_IMGUI

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

// CompileShader関数(どうやってファイル分けするかね).
IDxcBlob* CompileShader(
	// CompilerするShaderファイルへのパス.
	const std::wstring& filePath,
	// Compilerに使用するProfile.
	const wchar_t* profile,
	// 初期化で生成したものを3つ.
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler) {

	// ここの中身をこの後に書いていく.

	// 1. hlslファイルを読む.

	// これからシェーダーをコンパイルする旨をログに出す.
	Log(ConvertString(std::format(L"Begin CompileShader, path:{},profile:{}\n", filePath, profile)));
	// hlslファイルを読む.
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	// 読めなかったら止める.
	assert(SUCCEEDED(hr));
	// 読み込んだファイルの内容を確認する.
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8; // UTF8の文字コードであることを通知.


	// 2. Compileする.

	LPCWSTR argument[] = {
		filePath.c_str(), // コンパイル対象のhlslファイル名.
		L"-E",L"main", // エントリーポイントの指定。基本的にmain以外にはしない.
		L"-T",profile, // ShaderProfileの設定.
		L"-Zi",L"-Qembed_debug", // デバッグ用の情報を埋め込む.
		L"-Od",	// 最適化を外しておく.
		L"-Zpr", // メモリレイアウトは行優先.
	};
	// 実際にコンパイルする.
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer, // 読み込んだファイル.
		argument, // コンパイルオプション.
		_countof(argument), // コンパイルオプションの数.
		includeHandler, // includeが含まれた諸々.
		IID_PPV_ARGS(&shaderResult) // コンパイル結果.
	);
	// コンパイルエラーではなくdxcが起動できないなど致命的な状況.
	assert(SUCCEEDED(hr));


	// 3. 警告・エラーが出ていないかを確認する.

	// 警告・エラーが出てたらログに出して止める.
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		// 警告・エラー　ダメゼッタイ.
		assert(false);
	}


	// 4. Compile結果を受け取って返す.
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	// 成功したログを出す.
	Log(ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));
	// もう使わないリソースを解放.
	shaderSource->Release();
	shaderResult->Release();
	// 実行用のバイナリを返却.
	return shaderBlob;
}

// BufferResourceを作る関数.
ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
	// リソース用のヒープの設定.
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // UploadHeapを使う.
	// リソースの設定.
	D3D12_RESOURCE_DESC resourceDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする.
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	// バッファの場合はこれは1にする決まり.
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり.
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際にリソースを作る.
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

// DescriptorHeap関数(どうやってファイル分けするかね).
ID3D12DescriptorHeap* CreateDescriptorHeap(
	ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDiscriptors, bool shaderVisible) {
	// ディスクリプタヒープの生成.
	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType; // レンダーターゲットビュー用.
	descriptorHeapDesc.NumDescriptors = numDiscriptors; // ダブルバッファ用に2つ。多くてもかまわない.
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	// ディスクリプタヒープが作れなかったので起動できない.
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}



// Textureデータを読む(TextureManager的な奴に入れる).
DirectX::ScratchImage LoadTexture(const std::string& filePath) {
	// テクスチャファイルを読んでプログラムを扱えるようにする.
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミニマップの作成.
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// ミニマップ付きのデータを返す.
	return mipImages;
}

// DirectX12のTextureResourceを作る(TextureManager的な奴に入れる).
ID3D12Resource* CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metaData) {
	// 1. metadataを基にResourceの設定.
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metaData.width); // Textureの幅.
	resourceDesc.Height = UINT(metaData.height); // Textureの高さ.
	resourceDesc.MipLevels = UINT16(metaData.mipLevels); // mipmapの数.
	resourceDesc.DepthOrArraySize = UINT16(metaData.arraySize); // 奥行き or 配列Textureの配列数.
	resourceDesc.Format = metaData.format; // TextureのFormat.
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定.
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metaData.dimension); // Textureの次元数。普段使っているのは2次元.

	// 2. 利用するHeapの設定。非常に特殊な運用。02_04exで一般的なケース版がある(後々そっちに変えましょね).
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // 細かい設定を行う(03_00_exで変更した).
	//heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; // WriteBackポリシーでCPUアクセス可能.
	//heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0; // プロセッサの近くに配置.

	// 3. Resourceを生成する.

	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties, // Heapの設定.
		D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定。特になし.
		&resourceDesc, // Resource設定.
		D3D12_RESOURCE_STATE_COPY_DEST, // データ転送される設定(03_00_exで変更した).
		nullptr, // Clear最適値。使わないのでnullptr.
		IID_PPV_ARGS(&resource)); // 作成するResourceポインタへのポインタ.

	assert(SUCCEEDED(hr));

	return resource;
}

[[nodiscard]]
// 過去のやつ.
/*
void UploadTextureData(ID3D12Resource* texture,const DirectX::ScratchImage& mipImage) {
	// Meta情報を取得.
	const DirectX::TexMetadata& metadata = mipImage.GetMetadata();
	// 全MipMapについて.
	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels;++mipLevel) {
		// MipLevelを指定して各Imageを取得.
		const DirectX::Image* img = mipImage.GetImage(mipLevel,0,0);
		// Textureに転送.
		HRESULT hr = texture->WriteToSubresource(
			UINT(mipLevel),
			nullptr, // 全域へコピー
			img->pixels, // 元データアドレス.
			UINT(img->rowPitch), // 1ラインサイズ.
			UINT(img->slicePitch) // 1枚サイズ.
		);

		assert(SUCCEEDED(hr));
	}


}
*/

// にゅー！.
ID3D12Resource* UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList) {

	std::vector<D3D12_SUBRESOURCE_DATA> subresource;
	DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresource);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture, 0, UINT(subresource.size()));
	ID3D12Resource* intermediateResource = CreateBufferResource(device, intermediateSize);
	UpdateSubresources(commandList, texture, intermediateResource, 0, 0, UINT(subresource.size()), subresource.data());
	// Textureへの転用後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する.
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	return intermediateResource;
};

ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device,int32_t width, int32_t height){
	// 生成するResourceの設定.
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width; // Textureの幅.
	resourceDesc.Height = height; // Textureの高さ.
	resourceDesc.MipLevels = 1; // mipmapの数.
	resourceDesc.DepthOrArraySize = 1; // 奥行き or 配列Textureの配列数.
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // TextureのFormat.
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定.
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // Textureの次元数。普段使っているのは2次元.
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知.

	// 2. 利用するHeapの設定.
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る.

	// 深度値のクリア設定.
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f; // 1.0f(最大値)でクリア.
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット。Resourceと合わせる.

	// Resourceの作成.
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties, // Heapの設定.
		D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定。特になし.
		&resourceDesc, // Resource設定.
		D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値を書き込む状態にしておく.
		&depthClearValue, // Clear最適値.
		IID_PPV_ARGS(&resource)); // 作成するResourceポインタへのポインタ.

	assert(SUCCEEDED(hr));

	return resource;

};

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	/*=============================================================
	COMの初期化.
	=============================================================*/
	CoInitializeEx(0, COINIT_MULTITHREADED);


	/*=============================================================
	Window作成系
	=============================================================*/

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
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

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

#ifdef _DEBUG
	ID3D12Debug1* debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		// デバッグレイヤーを有効化する.
		debugController->EnableDebugLayer();
		// さらにGPU側でもチェックを行うようにする.
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif // _DEBUG


	// ウィンドウを表示する.
	ShowWindow(hwnd, SW_SHOW);

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
		DXGI_ERROR_NOT_FOUND; i++) {

		// アダプターの情報を取得する.
		DXGI_ADAPTER_DESC3 adapterDesc{};

		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr)); // 取得できないのは一大事.

		// ソフトウェアアダプタでなければ採用!出なければ帰れ!
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// 採用したアダプタの情報をログに出力.
			Log(ConvertString(std::format(L"Use Adapter : {}\n", adapterDesc.Description)));
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
		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));

		// 指定した昨日レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr)) {
			// 生成できたログ出力を行ってループを抜ける.
			Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}

	}

	assert(useAdapter != nullptr);
	Log("Complete create D3D12Device!!!\n");

#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		// ヤバいエラー時に止まる.
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラー時に止まる.
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// 警告時に止まる.
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		// 解放.
		infoQueue->Release();

		// 抑制するメッセージのID.
		D3D12_MESSAGE_ID denyIds[] = {
			// Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ.

			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		// 抑制するレベル.
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		// 指定したメッセージの表示を抑制する.
		infoQueue->PushStorageFilter(&filter);
	}
#endif // _DEBUG



	// 01_00 ウィンドウの背景.

	/*=============================================================
	コマンド系
	=============================================================*/

	// コマンドキューを生成する
	ID3D12CommandQueue* commandQueue = nullptr;

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));

	// コマンドキューの生成がうまくいかなかったので起動できない.
	assert(SUCCEEDED(hr));

	// コマンドアロケータを生成する
	ID3D12CommandAllocator* commandAllocator = nullptr;

	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	// コマンドアロケータの生成がうまくいかなかったので起動できない.
	assert(SUCCEEDED(hr));

	// コマンドリストを生成する
	ID3D12GraphicsCommandList* commandList = nullptr;

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	// コマンドリストの生成がうまくいかなかったので起動できない.
	assert(SUCCEEDED(hr));


	/*=============================================================
	スワップチェーン
	=============================================================*/
	// スワップチェーンを生成する.
	IDXGISwapChain4* swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = kClientWidth; //画面の幅。ウィンドウのクライアント領域を同じものにする.
	swapChainDesc.Height = kClinetHeight; //画面の高さ。ウィンドウのクライアント領域を同じものにする.
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //色の形式.
	swapChainDesc.SampleDesc.Count = 1; // マルチサンプルしない.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 描画のターゲットとして利用する.
	swapChainDesc.BufferCount = 2; // ダブルバッファ.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //モニタに移したら中身を破棄.
	// コマンドキュー、ウィンドウハンドル、設定を渡して生成する.
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain));
	assert(SUCCEEDED(hr));


	/*=============================================================
	ディスクリプタ系
	=============================================================*/
	// RTV用のヒープでディスクリプタの数は2。RTVはShader内で触るものではないので、ShaderVisbleはfalse.
	ID3D12DescriptorHeap* rtvDiscriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

	// SRV用のヒープでディスクリプタの数は128。SRVはShader内で触るものなので、ShaderVisbleはtrue.
	ID3D12DescriptorHeap* srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

	// SwapChainからResourceを引っ張ってくる.
	ID3D12Resource* swapChainResource[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResource[0]));
	// 上手く取得出来なければ起動できない.
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResource[1]));
	assert(SUCCEEDED(hr));

	// RTVの設定.
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 出力結果をSRGBに変換して書き込む.
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2dテクスチャとして書き込む.
	// ディスクリプタの先頭を取得する.
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDiscriptorHeap->GetCPUDescriptorHandleForHeapStart();
	// RTVを2つ作るのでディスクリプタを2つ用意.
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	// まず1つ目を作る。1つ目は最初の所に作る。作る場所をこちらで指定して上げる必要がある.
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResource[0], &rtvDesc, rtvHandles[0]);
	// 2つ目のディスクリプタハンドルを得る(自力で).
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// 2つ目を作る.
	device->CreateRenderTargetView(swapChainResource[1], &rtvDesc, rtvHandles[1]);


	/*=============================================================
	Fence、Event系
	=============================================================*/
	// 初期値0でFenceを作る.
	ID3D12Fence* fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	// FenceのSignalを持つためのイベントを作成する.
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);


	/*=============================================================
	DXCの初期化.
	=============================================================*/
	// dxcCompilerを初期化.
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	// 現時点でincludeはしないが、includeに対応するための設定を行っておく.
	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

	//
	// もともとPSOがあった場所(03_01にて変更).
	//

	/*=============================================================
	ResourceとView.
	=============================================================*/
	// 【VertexResourceを生成する】
	// 実際に頂点リソースを作る.
	ID3D12Resource* vertexResource = CreateBufferResource(device, sizeof(VertexData) * 6);

	// 【MaterialResourceを生成する】
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
	ID3D12Resource* materialResource = CreateBufferResource(device, sizeof(Vector4));
	// マテリアルにデータを書き込む.
	Vector4* materialData = nullptr;
	// 書き込むためのアドレスを取得.
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// 今回は赤を書き込んでみる
	*materialData = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	// 【TransformationMatrix】
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する.
	ID3D12Resource* wvpResource = CreateBufferResource(device, sizeof(Matrix4x4));
	// データを書き込む.
	Matrix4x4* wvpData = nullptr;
	// 書き込むためのアドレスを取得.
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 単位行列を書き込んでおく.
	*wvpData = Matrix4x4::Identity();


	// 【VertexBufferViewを作成する】

	// 頂点バッファビューを作成する.
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// リソースの先頭のアドレスから使う.
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ.
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	// 1頂点あたりのサイズ.
	vertexBufferView.StrideInBytes = sizeof(VertexData);


	// 【Resourceにデータを書き込む】

	// 頂点リソースにデータを書き込む.
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得.
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 左下.
	vertexData[0].position = { -0.5f,-0.5f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	// 上.
	vertexData[1].position = { 0.0f,0.5f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.5f,0.0f };
	// 左下.
	vertexData[2].position = { 0.5f,-0.5f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };

	// 左下2.
	vertexData[3].position = { -0.5f,-0.5f,0.5f,1.0f };
	vertexData[3].texcoord = { 0.0f,1.0f };
	// 上2.
	vertexData[4].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexData[4].texcoord = { 0.5f,0.0f };
	// 左下2.
	vertexData[5].position = { 0.5f,-0.5f,-0.5f,1.0f };
	vertexData[5].texcoord = { 1.0f,1.0f };

	// 【ビューポート】
	D3D12_VIEWPORT viewport{};
	// クライアント領域のサイズと一緒にして画面全体に表示.
	viewport.Width = kClientWidth;
	viewport.Height = kClinetHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// 【シザー矩形】
	D3D12_RECT scissorRect{};
	// 基本的にビューポートと同じ矩形が構成されるようにする.
	scissorRect.left = 0;
	scissorRect.right = kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = kClinetHeight;


	/*=============================================================
	ImGuiの初期化.
	=============================================================*/
#ifdef USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(device,
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap,
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Build();
#endif // USE_IMGUI


	/*=============================================================
	Texture読み込み.
	=============================================================*/
	// Textureを読んで転送する.
	DirectX::ScratchImage mipImage = LoadTexture("Resource/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImage.GetMetadata();
	ID3D12Resource* textureResource = CreateTextureResource(device, metadata);
	ID3D12Resource* intermediateResource = UploadTextureData(textureResource, mipImage, device, commandList);

	// commandListをCloseし、キックしたりする(スワップチェーン無しのフレーム更新みたいなもの).
	hr = commandList->Close();
	assert(SUCCEEDED(hr));

	//GPUにコマンドリストの実行を行わせる.
	ID3D12CommandList* commandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(1, commandLists);

	// Fanceの値を更新.
	fenceValue++;
	// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る.
	commandQueue->Signal(fence, fenceValue);

	// Fenceの値が指定したSignal値にたどり着いているか確認する.
	// GetCompletedValueの初期値はFence作成時に渡した初期値.
	if (fence->GetCompletedValue() < fenceValue) {
		// 指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する.
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		// イベント待つ.
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator, nullptr);
	assert(SUCCEEDED(hr));

	intermediateResource->Release();

	/*=============================================================
	DepthStencilTextureをつくる
	=============================================================*/
	ID3D12Resource* depthStencilResource = CreateDepthStencilTextureResource(device,kClientWidth,kClinetHeight);

	// DSV用のヒープでディスクリプタ数は1。DSVはShader内で触れるものではないので、ShaderVisibleはfalse.
	ID3D12DescriptorHeap* dsvDescriptorHeap = CreateDescriptorHeap(device,D3D12_DESCRIPTOR_HEAP_TYPE_DSV,1,false);

	// DSVの設定.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Format。基本的にResourceに合わせる.
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2dTexture.
	// DSVHeapの先頭にDSVをつくる.
	device->CreateDepthStencilView(depthStencilResource,&dsvDesc,dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// DepthStencilStateの設定.
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Depthの機能を有効化する.
	depthStencilDesc.DepthEnable = true;
	// 書き込みします.
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される.
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;


	/*=============================================================
	ShaderResourceViewを作る.
	=============================================================*/
	// metaDataを基にSRVの設定.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ.
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	// srvを作成するDescriptHeapの場所を決める.
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	// 先頭はImGuiが使っているのでその次を使う.
	textureSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// SRVの生成.
	device->CreateShaderResourceView(textureResource, &srvDesc, textureSrvHandleCPU);

	/*=============================================================
	PSO(どこに書けばいいかわからぬ).
	=============================================================*/
	// 【RootSignature作成】
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // 0から始まる.
	descriptorRange[0].NumDescriptors = 1; // 数は1つ.
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う.
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetの自動計算.

	// RootParameter作成。複数設定出来るので配列。
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う.
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
	rootParameters[0].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド.
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う.
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VertexShaderで使う.
	rootParameters[1].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド.
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //DescriptorTableを使う.
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定.
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数.

	descriptionRootSignature.pParameters = rootParameters; // ルートパラメータ配列へのポインタ.
	descriptionRootSignature.NumParameters = _countof(rootParameters); // 配列の長さ.

	// Samplerの設定.
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ.
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲外をリピート.
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない.
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipmapを使う.
	staticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う.
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// シリアライズしてバイナリにする.
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成.
	ID3D12RootSignature* rootSignature = nullptr;
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	// 【InputLayout】
	D3D12_INPUT_ELEMENT_DESC inputElementalDescs[2] = {};
	inputElementalDescs[0].SemanticName = "POSITION";
	inputElementalDescs[0].SemanticIndex = 0;
	inputElementalDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementalDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementalDescs[1].SemanticName = "TEXCOORD";
	inputElementalDescs[1].SemanticIndex = 0;
	inputElementalDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementalDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementalDescs;
	inputLayoutDesc.NumElements = _countof(inputElementalDescs);

	// 【BlendState設定】
	D3D12_BLEND_DESC blendDesc{};
	// 全ての色要素を書き込む.
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// 【RasterizerStateの設定を行う】
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない.
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす.
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする.
	// 【VertexShader】
	IDxcBlob* vertexShaderBlob = CompileShader(L"Object3d.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(vertexShaderBlob != nullptr);
	// 【PixelShader】
	IDxcBlob* pixelShaderBlob = CompileShader(L"Object3d.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(pixelShaderBlob != nullptr);

	// 【PSO】
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature; // RootSignature.
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc; // InputLayout.
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() }; // VertexShader.
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() }; // PixelShader.
	graphicsPipelineStateDesc.BlendState = blendDesc; // BlendState.
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState.
	// 書き込むRTVの情報.
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するとトポロジ(形状)のタイプ。三角形.
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定 (気にしなくていい)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// DepthStencilの設定.
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 実際に生成.
	ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	/*=============================================================
	ここから下がゲームの変数.
	=============================================================*/



	bool isTriangleAutoMove = false;

	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Camera::GetInstance()->Initialize(kClientWidth, kClinetHeight);

	// ウィンドウのxボタンが押されるまでループ.
	while (msg.message != WM_QUIT) {




		// Windowにメッセージが来てたら最優先で処理させる.
		if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {

#ifdef USE_IMGUI
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
#endif // USE_IMGUI

			// 指定した深度で画面全体をクリアする.
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			/*=============================================================
			以下にゲームの更新処理を記述.
			=============================================================*/

			// 三角形のもろもろ.
#ifdef USE_IMGUI
			ImGui::Begin("Triangle");
			ImGui::SliderFloat3("scale", reinterpret_cast<float*>(&transform.scale), 0.0f, 2.0f);
			ImGui::SliderFloat3("rotate", reinterpret_cast<float*>(&transform.rotate), 0.0f, Radian(360.0f));
			ImGui::SliderFloat3("translate", reinterpret_cast<float*>(&transform.translate), -5.0f, 5.0f);

			Vector4 imColor = *materialData;

			ImGui::ColorEdit4("color", reinterpret_cast<float*>(&imColor));

			*materialData = imColor;

			if (ImGui::Button("AutoMove")) {
				if (isTriangleAutoMove) {
					isTriangleAutoMove = false;
				}
				else {
					isTriangleAutoMove = true;
				}

				transform.rotate = { 0.0f,0.0f,0.0f };
			}

			ImGui::Text("AutoMove : %s", isTriangleAutoMove ? "true" : "false");

			ImGui::End();
#endif // USE_IMGUI

			Camera::GetInstance()->Update();

			if (isTriangleAutoMove) {
				transform.rotate.y += 0.03f;

				if (transform.rotate.y >= Radian(360.0f)) {
					transform.rotate.y -= Radian(360.0f);
				}
			}

			Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform);

			*wvpData = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);

			/*=============================================================
			以下にゲームの描画処理を記述.
			=============================================================*/
#ifdef USE_IMGUI
			// ImGuiの内部コマンドを生成する.
			ImGui::Render();
#endif // USE_IMGUI


			/*=============================================================
			コマンドを積む
			=============================================================*/
			// これから書き込むバックバッファのインデックスを取得.
			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();


			// TransitionBarrierの設定.
			D3D12_RESOURCE_BARRIER barrier{};
			// 今回のバリアはTransition.
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			// Noneにしておく.
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			// バリアを張る対象のリソース。現在のバックバッファに対して行う.
			barrier.Transition.pResource = swapChainResource[backBufferIndex];
			// 遷移前(現在)のResourceState.
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			// 遷移後のResourceState.
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			// TransitionBarrierを張る.
			commandList->ResourceBarrier(1, &barrier);


			// 描画先のRTVとDSVを設定する.
			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
			// 指定した色で画面全体をクリアする.
			float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };// 青っぽい色。RGBAの順.
			commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);



			// 描画用のDescriptorHeapの設定.
			ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap };
			commandList->SetDescriptorHeaps(1, descriptorHeaps);


			/*=============================================================
			三角形の描画のコマンド.
			=============================================================*/
			commandList->RSSetViewports(1, &viewport); // Viewportを設定.
			commandList->RSSetScissorRects(1, &scissorRect); // Scissorを設定.
			// RootSignatureを設定。PS0に設定しているけど別途設定が必要.
			commandList->SetGraphicsRootSignature(rootSignature);
			commandList->SetPipelineState(graphicsPipelineState); // PS0を設定.
			commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定.
			// 形状を設定。PS0に設定しているものとはまた別。同じものを設定すると考えておけば良い.
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			// CBufferの場所を設定.
			// マテリアル用のCBufferの場所.
			commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
			// WVP用のCBufferの場所.
			commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
			// SRVのDescriptorTableの先頭の設定。2はrootParameter[2]である.
			commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
			// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後.
			commandList->DrawInstanced(6, 1, 0, 0);


#ifdef USE_IMGUI
			// ImGuiの描画.
			// 実際のcommandListのImGuiの描画コマンドを積む.
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#endif // USE_IMGUI


			// 画面に描く処理は全て終わり、画面に映すので状態を遷移.
			// 今回はRenderTargetからPresentにする.
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			// TransitionBarrierを張る.
			commandList->ResourceBarrier(1, &barrier);


			// コマンドリストの内容を確定させる。全てのコマンドを積んでからCloseすること.
			hr = commandList->Close();
			assert(SUCCEEDED(hr));

			/*=============================================================
			コマンドをキックする.
			=============================================================*/
			//GPUにコマンドリストの実行を行わせる.
			ID3D12CommandList* commandLists[] = { commandList };
			commandQueue->ExecuteCommandLists(1, commandLists);
			//GPUとOSに画面の交換を行うよう通知する.
			swapChain->Present(1, 0);


			// Fanceの値を更新.
			fenceValue++;
			// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る.
			commandQueue->Signal(fence, fenceValue);


			// Fenceの値が指定したSignal値にたどり着いているか確認する.
			// GetCompletedValueの初期値はFence作成時に渡した初期値.
			if (fence->GetCompletedValue() < fenceValue) {
				// 指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する.
				fence->SetEventOnCompletion(fenceValue, fenceEvent);
				// イベント待つ.
				WaitForSingleObject(fenceEvent, INFINITE);
			}


			// 次のフレーム用のコマンドリストを準備.
			hr = commandAllocator->Reset();
			assert(SUCCEEDED(hr));
			hr = commandList->Reset(commandAllocator, nullptr);
			assert(SUCCEEDED(hr));
		}
	}

	/*=============================================================
	メモリ解放系.
	=============================================================*/
	CloseHandle(fenceEvent);
	fence->Release();
	rtvDiscriptorHeap->Release();
	swapChainResource[0]->Release();
	swapChainResource[1]->Release();
	swapChain->Release();
	commandList->Release();
	commandAllocator->Release();
	commandQueue->Release();
	device->Release();
	useAdapter->Release();
	dxgiFactory->Release();

	// 三角形の解放開始.
	vertexResource->Release();
	graphicsPipelineState->Release();
	signatureBlob->Release();

	if (errorBlob) {
		errorBlob->Release();
	}

	rootSignature->Release();
	pixelShaderBlob->Release();
	vertexShaderBlob->Release();
	materialResource->Release();
	wvpResource->Release();
	// 三角形の解放終了.


	/*=============================================================
	 ImGuiの終了処理.
	=============================================================*/
	// 初期化と逆順に行う.
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif // USE_IMGUI



	srvDescriptorHeap->Release();


#ifdef _DEBUG
	debugController->Release();
#endif // _DEBUG
	CloseWindow(hwnd);

	/*=============================================================
	Texture系の解放.
	=============================================================*/
	textureResource->Release();
	depthStencilResource->Release();
	dsvDescriptorHeap->Release();

	/*=============================================================
	メモリ解放チェック.
	=============================================================*/
	// リソースリークチェック.
	IDXGIDebug1* debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}

	CoUninitialize();

	return 0;
}
