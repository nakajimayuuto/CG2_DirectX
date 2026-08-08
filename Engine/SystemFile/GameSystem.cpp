#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")
#pragma comment(lib,"Dbghelp.lib")

#include "GameSystem.h"
#include "../../Managers/SoundManager.h"
#include "../../Managers/InputManager.h"
#include "../../Managers/TextureManager.h"
#include "../../Managers/LightManager.h"
#include "../../Managers/ModelManager.h"
#include "../../Managers/ParticleManager.h"
#include "../../Managers/CollisionManager.h"
#include "../../Environment.h"
#include "../Math/Random.h"
#include "GlobalVariables.h"
#include <strsafe.h>
#include <filesystem>
#include <chrono>
#include "../Renderer/Renderer.h"
#include "../Renderer/Camera.h"
#include "../Renderer/PointLight.h"
#include "../Renderer/SpotLight.h"
#include "DeltaTime.h"

GameSystem* GameSystem::GetInstance() {
	static GameSystem gameSystem;
	return &gameSystem;
}

LONG __stdcall GameSystem::ExportDump(EXCEPTION_POINTERS* exception) {
	// 時刻を取得して、時刻を名前に入れたファイルを作成。Dumpsディレクトリ以下に出力.
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	// processId(このexeのId)とクラッシュ(例外)の発生したthreadIdを取得.
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();
	// 設定情報を入力.
	MINIDUMP_EXCEPTION_INFORMATION minidumpInfomation{ 0 };
	minidumpInfomation.ThreadId = threadId;
	minidumpInfomation.ExceptionPointers = exception;
	minidumpInfomation.ClientPointers = TRUE;
	// Dumpを出力する。MiniDumpNormalは最低限の情報を出力するフラグ
	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInfomation, nullptr, nullptr);
	// 他に関連付けられているSHE例外ハンドルがあれば実行。通常はプロセスを終了する.
	return EXCEPTION_EXECUTE_HANDLER;
}

void GameSystem::Initialize() {
	/*=============================================================
	CrashHandler系.
	=============================================================*/
	// 誰も捕捉しなかった場合に(Unhandled)、補足する関数を登録.
	// main関数始まってすぐに登録すると良い.
	SetUnhandledExceptionFilter(ExportDump);

	/*=============================================================
	COMの初期化.
	=============================================================*/
	CoInitializeEx(0, COINIT_MULTITHREADED);


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

	InputManager::GetInstance()->Initialize();

	logStream = CreateLogFile();

	// CG2_00_05.

	// DXGIファクトリーの生成.
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;

	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));

	assert(SUCCEEDED(hr));

	// 使用するアダプタ用の変数、最初にnullptrを入れておく.
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;

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
			ExportLog(Convert::ConvertString(std::format(L"Use Adapter : {}\n", adapterDesc.Description)));
			break;
		}

		useAdapter = nullptr; // ソフトウェアアダプタの場合は見なかったこととする.
	}

	// 適切なアダプタが見つからなかったので起動できない.

	// 機能レベルとログ出力用の文字列.
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};

	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };

	// 高い順に生成できるか試していく.
	for (size_t i = 0; i < _countof(featureLevels); i++) {
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));

		// 指定した昨日レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr)) {
			// 生成できたログ出力を行ってループを抜ける.
			ExportLog(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}

	}

	assert(useAdapter != nullptr);
	ExportLog("Complete create D3D12Device!!!\n");

#ifdef _DEBUG
	Microsoft::WRL::ComPtr <ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		// ヤバいエラー時に止まる.
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラー時に止まる.
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// 警告時に止まる.
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		// 解放.

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

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));

	// コマンドキューの生成がうまくいかなかったので起動できない.
	assert(SUCCEEDED(hr));

	// コマンドアロケータを生成する

	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	// コマンドアロケータの生成がうまくいかなかったので起動できない.
	assert(SUCCEEDED(hr));

	// コマンドリストを生成する

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	// コマンドリストの生成がうまくいかなかったので起動できない.
	assert(SUCCEEDED(hr));


	/*=============================================================
	スワップチェーン
	=============================================================*/
	// スワップチェーンを生成する.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = kClientWidth; //画面の幅。ウィンドウのクライアント領域を同じものにする.
	swapChainDesc.Height = kClientHeight; //画面の高さ。ウィンドウのクライアント領域を同じものにする.
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //色の形式.
	swapChainDesc.SampleDesc.Count = 1; // マルチサンプルしない.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 描画のターゲットとして利用する.
	swapChainDesc.BufferCount = 2; // ダブルバッファ.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //モニタに移したら中身を破棄.
	// コマンドキュー、ウィンドウハンドル、設定を渡して生成する.
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));


	/*=============================================================
	ディスクリプタ系
	=============================================================*/
	// RTV用のヒープでディスクリプタの数は2。RTVはShader内で触るものではないので、ShaderVisibleはfalse.
	rtvDescriptorHeap = CreateDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	srvDescriptorHeapNum_ = 1;
	// SRV用のヒープでディスクリプタの数は128。SRVはShader内で触るものなので、ShaderVisibleはtrue.
	srvDescriptorHeap = CreateDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kSrvDescriptorHeapNumMax, true);
	descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// SwapChainからResourceを引っ張ってくる.
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResource[0]));
	// 上手く取得出来なければ起動できない.
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResource[1]));
	assert(SUCCEEDED(hr));

	// RTVの設定.
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 出力結果をSRGBに変換して書き込む.
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2dテクスチャとして書き込む.
	// RTVを2つ作るのでディスクリプタを2つ用意.
	// まず1つ目を作る。1つ目は最初の所に作る。作る場所をこちらで指定して上げる必要がある.
	const uint32_t rtvHandleMax = 2;

	for (uint32_t dataNumber = 0; dataNumber < rtvHandleMax; dataNumber++) {
		rtvHandles[dataNumber] = GetCPUDescriptorHandle(rtvDescriptorHeap, descriptorSizeRTV, dataNumber);
	}
	device->CreateRenderTargetView(swapChainResource[0].Get(), &rtvDesc, rtvHandles[0]);

	// 2つ目を作る.
	device->CreateRenderTargetView(swapChainResource[1].Get(), &rtvDesc, rtvHandles[1]);


	/*=============================================================
	Fence、Event系
	=============================================================*/
	// 初期値0でFenceを作る.
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	// FenceのSignalを持つためのイベントを作成する.
	//HANDLE fenceEvent 
	fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);



	/*=============================================================
	DirectionalLightの初期化.
	=============================================================*/

	LightManager::GetInstance()->Initialize();

	//DirectionalLight::GetInstance()->Initialize();
	//
	//PointLight::GetInstance()->Initialize();
	//
	//SpotLight::GetInstance()->Initialize();

	/*=============================================================
	ImGuiの初期化.
	=============================================================*/
#ifdef USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(device.Get(),
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap.Get(),
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Build();
#endif // USE_IMGUI

	// 【ビューポート】
	// クライアント領域のサイズと一緒にして画面全体に表示.
	viewport.Width = static_cast<FLOAT>(Environment::GetInstance()->GetWindowSize().width);
	viewport.Height = static_cast<FLOAT>(Environment::GetInstance()->GetWindowSize().height);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// 【シザー矩形】
	// 基本的にビューポートと同じ矩形が構成されるようにする.
	scissorRect.left = 0;
	scissorRect.right = static_cast<int32_t>(Environment::GetInstance()->GetWindowSize().width);
	scissorRect.top = 0;
	scissorRect.bottom = static_cast<int32_t>(Environment::GetInstance()->GetWindowSize().height);/*=============================================================
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

	vertexShaderBlobParticle = CompileShader(L"./hlsl/Particle.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	pixelShaderBlobParticle = CompileShader(L"./hlsl/Particle.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);

	vertexShaderBlobLine = CompileShader(L"./hlsl/Line.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	pixelShaderBlobLine = CompileShader(L"./hlsl/Line.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);

	vertexShaderBlob3dObject = CompileShader(L"./hlsl/Object3d.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	pixelShaderBlob3dObject = CompileShader(L"./hlsl/Object3d.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);

	vertexShaderBlobNoTexture = CompileShader(L"./hlsl/NoTexture.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	pixelShaderBlobNoTexture = CompileShader(L"./hlsl/NoTexture.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);

	for (uint32_t j = 0; j < static_cast<uint32_t>(ShaderType::kCount); j++) {
		for (uint32_t i = 0; i < static_cast<uint32_t>(BlendMode::kCount); i++) {
			CreatePipeline(static_cast<BlendMode>(i), static_cast<ShaderType>(j));
		}
	}

	Camera::GetInstance()->CreateResource();

	SoundManager::GetInstance()->Initialize();

	Random::GetInstance()->Initialize();

	ModelManager::GetInstance()->RegisterObj("block_template", "Resource/block", "block.obj");

	ModelManager::GetInstance()->RegisterObj("effect_plane", "Resource/effects", "effect_plane.obj");

	TextureManager::GetInstance()->RegisterTexture("white_template", "Resource/white_template.png");

	//Renderer::Line::GetInstance()->Initialize();

	Renderer::GetInstance()->Initialize();

	GlobalVariables::GetInstance()->LoadFiles();

	DeltaTime::GetInstance()->Initialize();

	Environment::GetInstance()->Initialize();

	ParticleManager::GetInstance()->Initialize();

	RegisterGlobalVariables();

	dxcCompiler->Release();
	dxcUtils->Release();
}

void GameSystem::CreatePipeline(BlendMode blendMode, ShaderType shaderType) {
	uint32_t pipeLineIndex = static_cast<uint32_t>(blendMode) + (static_cast<uint32_t>(shaderType) * static_cast<uint32_t>(BlendMode::kCount));
	if (shaderType != ShaderType::kObject3d) {
		if (blendMode == BlendMode::kLine) {
			return;
		}
	}


	//
	// もともとPSOがあった場所(03_01にて変更).
	//

	/*=============================================================
	DepthStencilTextureをつくる
	=============================================================*/
	depthStencilResource = CreateDepthStencilTextureResource(device, static_cast<int32_t>(Environment::GetInstance()->GetWindowSize().width), static_cast<int32_t>(Environment::GetInstance()->GetWindowSize().height));

	// DSV用のヒープでディスクリプタ数は1。DSVはShader内で触れるものではないので、ShaderVisibleはfalse.
	dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	// DSVの設定.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Format。基本的にResourceに合わせる.
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2dTexture.
	// DSVHeapの先頭にDSVをつくる.
	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// DepthStencilStateの設定.
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Depthの機能を有効化する.
	depthStencilDesc.DepthEnable = true;
	// 書き込みします.
	if (shaderType == ShaderType::kParticle) {
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	} else {
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	}
	// 比較関数はLessEqual。つまり、近ければ描画される.
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	/*=============================================================
	PSO(どこに書けばいいかわからぬ).
	=============================================================*/
	// 【RootSignature作成】
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
	descriptorRangeForInstancing[0].BaseShaderRegister = 0; // 0から始まる.
	descriptorRangeForInstancing[0].NumDescriptors = 1; // 数は1つ.
	descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う.
	descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetの自動計算.

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // 0から始まる.
	descriptorRange[0].NumDescriptors = 1; // 数は1つ.
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う.
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetの自動計算.

	D3D12_DESCRIPTOR_RANGE pointLightDescriptorRange[1] = {};
	pointLightDescriptorRange[0].BaseShaderRegister = 1; // 1から始まる.
	pointLightDescriptorRange[0].NumDescriptors = 1; // 数は1つ.
	pointLightDescriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う.
	pointLightDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetの自動計算.

	D3D12_DESCRIPTOR_RANGE spotLightDescriptorRange[1] = {};
	spotLightDescriptorRange[0].BaseShaderRegister = 2; // 2から始まる.
	spotLightDescriptorRange[0].NumDescriptors = 1; // 数は1つ.
	spotLightDescriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う.
	spotLightDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetの自動計算.

	// RootParameter作成。複数設定出来るので配列。
	D3D12_ROOT_PARAMETER rootParameters[8] = {};
	D3D12_ROOT_PARAMETER rootParametersNoTexture[7] = {};
	D3D12_ROOT_PARAMETER rootParametersParticle[3] = {};
	D3D12_ROOT_PARAMETER rootParametersLine[1] = {};

	switch (shaderType) {
	case ShaderType::kParticle:
		rootParametersParticle[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う.
		rootParametersParticle[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
		rootParametersParticle[0].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド.

		rootParametersParticle[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // CBVを使う.
		rootParametersParticle[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VertexShaderで使う.
		rootParametersParticle[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing; // レジスタ番号0とバインド.
		rootParametersParticle[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);

		rootParametersParticle[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //DescriptorTableを使う.
		rootParametersParticle[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
		rootParametersParticle[2].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定.
		rootParametersParticle[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数.

		descriptionRootSignature.pParameters = rootParametersParticle; // ルートパラメータ配列へのポインタ.
		descriptionRootSignature.NumParameters = _countof(rootParametersParticle); // 配列の長さ.
		break;
	case ShaderType::kNoTexture:
		rootParametersNoTexture[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う.
		rootParametersNoTexture[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
		rootParametersNoTexture[0].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド.

		rootParametersNoTexture[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う.
		rootParametersNoTexture[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VertexShaderで使う.
		rootParametersNoTexture[1].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド.

		// DirectionalLightData.
		rootParametersNoTexture[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う.
		rootParametersNoTexture[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
		rootParametersNoTexture[2].Descriptor.ShaderRegister = 1; // レジスタ番号1を使う.

		// CameraData.
		rootParametersNoTexture[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う.
		rootParametersNoTexture[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
		rootParametersNoTexture[3].Descriptor.ShaderRegister = 2; // レジスタ番号2を使う.

		// PointLightData.
		rootParametersNoTexture[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //DescriptorTableを使う.
		rootParametersNoTexture[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
		rootParametersNoTexture[4].DescriptorTable.pDescriptorRanges = pointLightDescriptorRange; // Tableの中身の配列を指定.
		rootParametersNoTexture[4].DescriptorTable.NumDescriptorRanges = _countof(pointLightDescriptorRange); // Tableで利用する数.

		// SpotLightData.
		rootParametersNoTexture[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //DescriptorTableを使う.
		rootParametersNoTexture[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
		rootParametersNoTexture[5].DescriptorTable.pDescriptorRanges = spotLightDescriptorRange; // Tableの中身の配列を指定.
		rootParametersNoTexture[5].DescriptorTable.NumDescriptorRanges = _countof(spotLightDescriptorRange); // Tableで利用する数.
		// LightNumData.
		rootParametersNoTexture[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う.
		rootParametersNoTexture[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
		rootParametersNoTexture[6].Descriptor.ShaderRegister = 3; // レジスタ番号3を使う.

		descriptionRootSignature.pParameters = rootParametersNoTexture; // ルートパラメータ配列へのポインタ.
		descriptionRootSignature.NumParameters = _countof(rootParametersNoTexture); // 配列の長さ.
		break;
	default:
		if (blendMode == BlendMode::kLine) {
			rootParametersLine[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う.
			rootParametersLine[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VertexShaderで使う.
			rootParametersLine[0].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド.

			descriptionRootSignature.pParameters = rootParametersLine; // ルートパラメータ配列へのポインタ.
			descriptionRootSignature.NumParameters = _countof(rootParametersLine); // 配列の長さ.
			break;
		}

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

		// DirectionalLightData.
		rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う.
		rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
		rootParameters[3].Descriptor.ShaderRegister = 1; // レジスタ番号1を使う.

		// CameraData.
		rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う.
		rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
		rootParameters[4].Descriptor.ShaderRegister = 2; // レジスタ番号2を使う.

		// PointLightData.
		rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //DescriptorTableを使う.
		rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
		rootParameters[5].DescriptorTable.pDescriptorRanges = pointLightDescriptorRange; // Tableの中身の配列を指定.
		rootParameters[5].DescriptorTable.NumDescriptorRanges = _countof(pointLightDescriptorRange); // Tableで利用する数.

		// SpotLightData.
		rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //DescriptorTableを使う.
		rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
		rootParameters[6].DescriptorTable.pDescriptorRanges = spotLightDescriptorRange; // Tableの中身の配列を指定.
		rootParameters[6].DescriptorTable.NumDescriptorRanges = _countof(spotLightDescriptorRange); // Tableで利用する数.
		// LightNumData.
		rootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う.
		rootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
		rootParameters[7].Descriptor.ShaderRegister = 3; // レジスタ番号2を使う.

		descriptionRootSignature.pParameters = rootParameters; // ルートパラメータ配列へのポインタ.
		descriptionRootSignature.NumParameters = _countof(rootParameters); // 配列の長さ.
		break;
	}

	// Samplerの設定.
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	if (blendMode != BlendMode::kLine && shaderType != ShaderType::kNoTexture) {
		staticSamplers[0].Filter = kUsingFillter_; // バイリニアフィルタ.(重要!ゲーム的にここをLINEARからPOINTに切り替えています)
		staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲外をリピート.
		staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない.
		staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipmapを使う.
		staticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う.
		staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
		descriptionRootSignature.pStaticSamplers = staticSamplers;
		descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);
	}

	// シリアライズしてバイナリにする.
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		ExportLog(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成.
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pipeline_[pipeLineIndex].rootSignature));
	assert(SUCCEEDED(hr));

	D3D12_INPUT_ELEMENT_DESC inputElementalDescs[3] = {};
	D3D12_INPUT_ELEMENT_DESC inputElementalDescsLine[2] = {};
	D3D12_INPUT_ELEMENT_DESC inputElementalDescsNoTexture[2] = {};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	if (blendMode == BlendMode::kLine) {
		// 【InputLayout】
		inputElementalDescsLine[0].SemanticName = "POSITION";
		inputElementalDescsLine[0].SemanticIndex = 0;
		inputElementalDescsLine[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		inputElementalDescsLine[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElementalDescsLine[1].SemanticName = "COLOR";
		inputElementalDescsLine[1].SemanticIndex = 0;
		inputElementalDescsLine[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		inputElementalDescsLine[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

		inputLayoutDesc.pInputElementDescs = inputElementalDescsLine;
		inputLayoutDesc.NumElements = _countof(inputElementalDescsLine);

	}else if(shaderType == ShaderType::kNoTexture){
		// 【InputLayout】
		inputElementalDescsNoTexture[0].SemanticName = "POSITION";
		inputElementalDescsNoTexture[0].SemanticIndex = 0;
		inputElementalDescsNoTexture[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		inputElementalDescsNoTexture[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElementalDescsNoTexture[1].SemanticName = "NORMAL";
		inputElementalDescsNoTexture[1].SemanticIndex = 0;
		inputElementalDescsNoTexture[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputElementalDescsNoTexture[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

		inputLayoutDesc.pInputElementDescs = inputElementalDescsNoTexture;
		inputLayoutDesc.NumElements = _countof(inputElementalDescsNoTexture);
	} else {
		// 【InputLayout】
		inputElementalDescs[0].SemanticName = "POSITION";
		inputElementalDescs[0].SemanticIndex = 0;
		inputElementalDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		inputElementalDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElementalDescs[1].SemanticName = "TEXCOORD";
		inputElementalDescs[1].SemanticIndex = 0;
		inputElementalDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputElementalDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElementalDescs[2].SemanticName = "NORMAL";
		inputElementalDescs[2].SemanticIndex = 0;
		inputElementalDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputElementalDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

		inputLayoutDesc.pInputElementDescs = inputElementalDescs;
		inputLayoutDesc.NumElements = _countof(inputElementalDescs);
	}

	// 【BlendState設定】
	D3D12_BLEND_DESC blendDesc{};
	// 全ての色要素を書き込む.
	// AL3_05_10にて透明度が反映さえる変更を加えた.
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendEnable = true;

	switch (blendMode) {
	case BlendMode::kNormal:
	case BlendMode::kNormalCullNone:
	case BlendMode::kLine:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		break;
	case BlendMode::kAdd:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	case BlendMode::kSubtract:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	case BlendMode::kMultily:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
		break;
	case BlendMode::kScreen:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	default:
		blendDesc.RenderTarget[0].BlendEnable = false;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		break;
	}


	// 【RasterizerStateの設定を行う】
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない.
	if (blendMode == BlendMode::kNormalCullNone) {
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	} else {
		rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	}
	// 三角形の中を塗りつぶす.
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする.
	// 【VertexShader】
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob;
	// 【PixelShader】
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob;

	if (shaderType == ShaderType::kParticle) {
		vertexShaderBlob = vertexShaderBlobParticle;
		pixelShaderBlob = pixelShaderBlobParticle;
	} else if (shaderType == ShaderType::kNoTexture) {
		vertexShaderBlob = vertexShaderBlobNoTexture;
		pixelShaderBlob = pixelShaderBlobNoTexture;
	} else if(blendMode == BlendMode::kLine){
		vertexShaderBlob = vertexShaderBlobLine;
		pixelShaderBlob = pixelShaderBlobLine;
	} else {
		vertexShaderBlob = vertexShaderBlob3dObject;
		pixelShaderBlob = pixelShaderBlob3dObject;
	}

	assert(vertexShaderBlob != nullptr);
	assert(pixelShaderBlob != nullptr);

	// 【PSO】
	graphicsPipelineStateDesc.pRootSignature = pipeline_[pipeLineIndex].rootSignature.Get(); // RootSignature.
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc; // InputLayout.
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() }; // VertexShader.
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() }; // PixelShader.
	graphicsPipelineStateDesc.BlendState = blendDesc; // BlendState.
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState.
	// 書き込むRTVの情報.
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するとトポロジ(形状)のタイプ。三角形.
	if (blendMode == BlendMode::kLine) {
		graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	} else {
		graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	}
	// どのように画面に色を打ち込むかの設定 (気にしなくていい)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// DepthStencilの設定.
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 実際に生成.
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipeline_[pipeLineIndex].graphicsPipelineState));
	assert(SUCCEEDED(hr));

	commandList->SetGraphicsRootSignature(pipeline_[pipeLineIndex].rootSignature.Get());
	commandList->SetPipelineState(pipeline_[pipeLineIndex].graphicsPipelineState.Get()); // PS0を設定.
}

void GameSystem::SetPipeline(BlendMode blendMode) {
	commandList->RSSetViewports(1, &viewport); // Viewportを設定.
	commandList->RSSetScissorRects(1, &scissorRect); // Scissorを設定.
	// RootSignatureを設定。PS0に設定しているけど別途設定が必要.
	commandList->SetGraphicsRootSignature(pipeline_[static_cast<uint32_t>(blendMode)].rootSignature.Get());
	commandList->SetPipelineState(pipeline_[static_cast<uint32_t>(blendMode)].graphicsPipelineState.Get()); // PS0を設定.
}

void GameSystem::SetParticlePipeline(BlendMode blendMode) {
	commandList->RSSetViewports(1, &viewport); // Viewportを設定.
	commandList->RSSetScissorRects(1, &scissorRect); // Scissorを設定.
	// RootSignatureを設定。PS0に設定しているけど別途設定が必要.
	commandList->SetGraphicsRootSignature(pipeline_[static_cast<uint32_t>(blendMode) + (static_cast<uint32_t>(ShaderType::kParticle) * static_cast<uint32_t>(BlendMode::kCount))].rootSignature.Get());
	commandList->SetPipelineState(pipeline_[static_cast<uint32_t>(blendMode) + (static_cast<uint32_t>(ShaderType::kParticle) * static_cast<uint32_t>(BlendMode::kCount))].graphicsPipelineState.Get()); // PS0を設定.
}

void GameSystem::DrawCommand(
	BlendMode blendMode,
	D3D12_VERTEX_BUFFER_VIEW* vertexBufferView,
	D3D12_INDEX_BUFFER_VIEW* indexBufferView,
	D3D_PRIMITIVE_TOPOLOGY topology,
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource,
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource,
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU,
	uint32_t indexInstancedNum
) {
	SetPipeline(blendMode);

	commandList->IASetVertexBuffers(0, 1, vertexBufferView); // VBVを設定.
	if (indexBufferView != nullptr) {
		commandList->IASetIndexBuffer(indexBufferView); // IBVを設定.
	}
	// 形状を設定。PS0に設定しているものとはまた別。同じものを設定すると考えておけば良い.
	commandList->IASetPrimitiveTopology(topology);

	if (blendMode == BlendMode::kLine) {
		// transformationMatrixCBufferの場所.
		commandList->SetGraphicsRootConstantBufferView(0, wvpResource->GetGPUVirtualAddress());
	} else {
		// マテリアル用のCBufferの場所.
		commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		// transformationMatrixCBufferの場所.
		commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
		// SRVのDescriptorTableの先頭の設定。2はrootParameter[2]である.
		commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
		commandList->SetGraphicsRootConstantBufferView(3, LightManager::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(4, Camera::GetInstance()->GetCameraForGPUResource()->GetGPUVirtualAddress());
		//commandList->SetGraphicsRootConstantBufferView(5, PointLight::GetInstance()->GetPointLightResource()->GetGPUVirtualAddress());
		//commandList->SetGraphicsRootConstantBufferView(6, SpotLight::GetInstance()->GetSpotLightResource()->GetGPUVirtualAddress());
		commandList->SetGraphicsRootDescriptorTable(5, LightManager::GetInstance()->GetPointLightSrvHandleGPU());
		commandList->SetGraphicsRootDescriptorTable(6, LightManager::GetInstance()->GetSpotLightSrvHandleGPU());
		commandList->SetGraphicsRootConstantBufferView(7, LightManager::GetInstance()->GetLightNumResource()->GetGPUVirtualAddress());
	}
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後.
	if (indexBufferView == nullptr) {
		commandList->DrawInstanced(indexInstancedNum, 1, 0, 0);
	} else {
		commandList->DrawIndexedInstanced(indexInstancedNum, 1, 0, 0, 0);
	}

	drawCount_++;
}

bool GameSystem::ProcessMessage() {
	if (Environment::GetInstance()->GetIsGameFinished()) {
		return false;
	}

	return msg.message != WM_QUIT;
}

bool GameSystem::BeginFrame() {
	drawCount_ = 0;
	// Windowにメッセージが来てたら最優先で処理させる.
	if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		return false;
	}

	InputManager::GetInstance()->Update();

	DeltaTime::GetInstance()->GetStartDebugTime();

	SoundManager::GetInstance()->Update();
	//Renderer::Line::GetInstance()->ClearDrawIndex();
	Renderer::GetInstance()->ClearDrawIndex();

#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif // USE_IMGUI

	// 指定した深度で画面全体をクリアする.
	dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	ApplyGlobalVariables();

	DeltaTime::GetInstance()->Update();

	/*=============================================================
	コマンドを積む
	=============================================================*/
	// これから書き込むバックバッファのインデックスを取得.
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();


	// TransitionBarrierの設定.
	// 今回のバリアはTransition.
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく.
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを張る対象のリソース。現在のバックバッファに対して行う.
	barrier.Transition.pResource = swapChainResource[backBufferIndex].Get();
	// 遷移前(現在)のResourceState.
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	// 遷移後のResourceState.
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// TransitionBarrierを張る.
	commandList->ResourceBarrier(1, &barrier);


	// 描画先のRTVとDSVを設定する.
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
	// 指定した色で画面全体をクリアする.
	//float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };// 青っぽい色。RGBAの順.
	float clearColor[] = { 0.0f,0.0f,0.0f,1.0f };// 青っぽい色。RGBAの順.
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);



	// 描画用のDescriptorHeapの設定.
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { srvDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());


	commandList->RSSetViewports(1, &viewport); // Viewportを設定.
	commandList->RSSetScissorRects(1, &scissorRect); // Scissorを設定.
	// RootSignatureを設定。PS0に設定しているけど別途設定が必要.
	commandList->SetGraphicsRootSignature(pipeline_[static_cast<uint32_t>(BlendMode::kNormal)].rootSignature.Get());
	commandList->SetPipelineState(pipeline_[static_cast<uint32_t>(BlendMode::kNormal)].graphicsPipelineState.Get()); // PS0を設定.

	return true;
}

void GameSystem::DrawSetup() {

}

void GameSystem::EndFrame() {
	WindowSizeUpdate();

	LightManager::GetInstance()->Update();

	Camera::GetInstance()->Draw();

	Renderer::GetInstance()->DrawLineAll();

	ImGui::Begin("drawCount");
	ImGui::Text("%d", drawCount_);
	ImGui::End();

#ifdef USE_IMGUI
	// ImGuiの内部コマンドを生成する.
	ImGui::Render();
#endif // USE_IMGUI
#ifdef USE_IMGUI
	// ImGuiの描画.
	// 実際のcommandListのImGuiの描画コマンドを積む.
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
#endif // USE_IMGUI


	// 画面に描く処理は全て終わり、画面に映すので状態を遷移.
	// 今回はRenderTargetからPresentにする.
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	// TransitionBarrierを張る.
	commandList->ResourceBarrier(1, &barrier);


	// コマンドリストの内容を確定させる。全てのコマンドを積んでからCloseすること.
	HRESULT hr = commandList->Close();
	assert(SUCCEEDED(hr));

	/*=============================================================
	コマンドをキックする.
	=============================================================*/
	//GPUにコマンドリストの実行を行わせる.
	Microsoft::WRL::ComPtr<ID3D12CommandList> commandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(1, commandLists->GetAddressOf());
	//GPUとOSに画面の交換を行うよう通知する.
	swapChain->Present(1, 0);

	// Fenceの値を更新.
	fenceValue++;
	// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る.
	commandQueue->Signal(fence.Get(), fenceValue);

	DeltaTime::GetInstance()->GetEndDebugTime();

	// Fenceの値が指定したSignal値にたどり着いているか確認する.
	// GetCompletedValueの初期値はFence作成時に渡した初期値.
	auto start = std::chrono::high_resolution_clock::now();

	if (fence->GetCompletedValue() < fenceValue) {

		fence->SetEventOnCompletion(fenceValue, fenceEvent);

		WaitForSingleObject(fenceEvent, INFINITE);
	}

	auto end = std::chrono::high_resolution_clock::now();

	double waitTime =
		std::chrono::duration<double, std::milli>(end - start).count();

	OutputDebugStringA(static_cast<LPCSTR>(std::format("{}ms\n",waitTime).c_str()));


	// 次のフレーム用のコマンドリストを準備.
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

void GameSystem::Finalize() {
	SoundManager::GetInstance()->Finalize();

	/*=============================================================
	メモリ解放系.
	=============================================================*/
	CloseHandle(fenceEvent);

	/*=============================================================
	 ImGuiの終了処理.
	=============================================================*/
	// 初期化と逆順に行う.
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif // USE_IMGUI

	CloseWindow(hwnd);

	CoUninitialize();
}

void GameSystem::WindowSizeUpdate() {
	if (
		Environment::GetInstance()->GetAspectMode() == kAspectNone ||
		Environment::GetInstance()->GetAspectMode() == kAspectWindowFixed ||
		Environment::GetInstance()->GetAspectMode() == kAspectNoChange
		) {
		return;
	}

	RECT clientRect{};
	GetClientRect(GameSystem::GetInstance()->GetHWND(), &clientRect);
	float windowWidth = static_cast<float>(clientRect.right - clientRect.left);
	float windowHeight = static_cast<float>(clientRect.bottom - clientRect.top);

	float targetAspect = Environment::GetInstance()->GetAspect();

	float windowAspect = windowWidth / windowHeight;

	float viewportWidth;
	float viewportHeight;
	float viewportX = 0.0f;
	float viewportY = 0.0f;

	if (windowAspect > targetAspect) {
		viewportHeight = windowHeight;
		viewportWidth = viewportHeight * targetAspect * (Environment::GetInstance()->GetWindowSize().width / windowWidth);
		if (windowWidth > Environment::GetInstance()->GetWindowSize().width) {
			viewportWidth = Environment::GetInstance()->GetWindowSize().width * (targetAspect / windowAspect);
			viewportX = (Environment::GetInstance()->GetWindowSize().width - viewportWidth) * 0.5f;
		} else {
			viewportX = (windowWidth - (viewportHeight * targetAspect)) * 0.5f * (Environment::GetInstance()->GetWindowSize().width / windowWidth);
		}

		viewportHeight =static_cast<float>(Environment::GetInstance()->GetWindowSize().height);
	} else {
		viewportWidth = windowWidth;
		viewportHeight = viewportWidth / targetAspect * (Environment::GetInstance()->GetWindowSize().height / windowHeight);
		if (windowHeight > Environment::GetInstance()->GetWindowSize().height) {
			viewportHeight = Environment::GetInstance()->GetWindowSize().height / (targetAspect / windowAspect);
			viewportY = (Environment::GetInstance()->GetWindowSize().height - viewportHeight) * 0.5f;
		} else {
			viewportY = (windowHeight - (viewportWidth / targetAspect)) * 0.5f * (Environment::GetInstance()->GetWindowSize().height / windowHeight);
		}


		viewportWidth = static_cast<float>(Environment::GetInstance()->GetWindowSize().width);
	}

	// こっちがカメラの位置みたいなやつ.
	viewport = {};
	viewport.TopLeftX = viewportX;
	viewport.TopLeftY = viewportY;
	viewport.Width = viewportWidth;
	viewport.Height = viewportHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// こっちがカメラ描画する範囲みたいなやつ.
	scissorRect = {};
	scissorRect.left = static_cast<LONG>(viewportX);
	scissorRect.top = static_cast<LONG>(viewportY);
	scissorRect.right = static_cast<LONG>(viewportX + viewportWidth);
	scissorRect.bottom = static_cast<LONG>(viewportY + viewportHeight);
}


void GameSystem::RegisterGlobalVariables() {
	Camera::GetInstance()->RegisterGlobalVariables();
	CollisionManager::RegisterGlobalVariables();
	//DirectionalLight::GetInstance()->RegisterGlobalVariables();
};

void GameSystem::ApplyGlobalVariables() {
	Camera::GetInstance()->ApplyGlobalVariables();
	CollisionManager::ApplyGlobalVariables();
	//DirectionalLight::GetInstance()->ApplyGlobalVariables();
};

LRESULT CALLBACK GameSystem::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
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

/*
LRESULT CALLBACK GameSystem::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}

	float aspect = Environment::GetInstance()->GetAspect();
	switch (msg)
	{
	case WM_SIZING:
	{
		RECT* rect = reinterpret_cast<RECT*>(lparam);

		DWORD style =
			static_cast<DWORD>(
				GetWindowLongPtr(hwnd, GWL_STYLE));

		DWORD exStyle =
			static_cast<DWORD>(
				GetWindowLongPtr(hwnd, GWL_EXSTYLE));

		RECT borderRect = { 0,0,0,0 };

		AdjustWindowRectEx(
			&borderRect,
			style,
			FALSE,
			exStyle);

		int borderWidth =
			borderRect.right - borderRect.left;

		int borderHeight =
			borderRect.bottom - borderRect.top;

		int width = rect->right - rect->left;
		int height = rect->bottom - rect->top;
		//--------------------------------------
		// クライアントサイズへ変換
		//--------------------------------------
		int clientWidth = width - borderWidth;

		int clientHeight = height - borderHeight;


		rect->bottom = clientHeight;
		rect->right = clientWidth;
		int newWidth;
		int newHeight;

		int currentPosX = rect->left;
		int currentPosY = rect->top;

		Log(std::format("Rect l:{},r:{},t:{},b:{}\n", rect->left, rect->right, rect->top, rect->bottom));
		Log(std::format("Rect w:{},h:{}\n", width,height));

		switch (wparam) {
		case WMSZ_LEFT:
		case WMSZ_RIGHT:
			// 左右をドラッグ → 高さを補正
			newHeight = static_cast<int>(clientWidth / aspect);
			rect->bottom = newHeight;
			currentPosX = rect->left;
			break;
		case WMSZ_TOP:
		case WMSZ_BOTTOM:
			// 上下をドラッグ → 幅を補正
			newWidth = static_cast<int>(clientHeight * aspect);
			rect->right = newWidth;
			currentPosY = rect->top;
			break;
		case WMSZ_TOPLEFT:
		case WMSZ_TOPRIGHT:
		case WMSZ_BOTTOMLEFT:
		case WMSZ_BOTTOMRIGHT:
			// 四隅ドラッグ
			newHeight = static_cast<int>(clientWidth / aspect);
			rect->bottom = newHeight;
			currentPosX = rect->left;
			break;
		}

		rect->left = 0;
		rect->top = 0;

		AdjustWindowRectEx(
			rect,
			style,
			FALSE,
			exStyle);

		rect->left = currentPosX;
		rect->top = currentPosY;
		rect->right = rect->right + currentPosX + borderWidth;
		rect->bottom = rect->bottom + currentPosY + borderHeight;

		return TRUE;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}
*/

std::ofstream GameSystem::CreateLogFile() {
	// ログのディレクトリを用意.
	std::filesystem::create_directory("logs");
	// 現在時刻の取得.
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	// ログファイルの名前にコンマ何秒はいらないので、削って秒にする.
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	// 日本時間(PCの設定時間)に変換.
	std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };
	// formatを使って年月日_時分秒の文字列に変換.
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	// 時刻を使ってファイル名を決定.
	std::string logFilePath = std::string("logs/") + dateString + ".log";
	// ファイルを作って書き込み準備.
	std::ofstream logStream(logFilePath);

	return logStream;
}

void GameSystem::Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}

void GameSystem::ExportLog(const std::string& message) {
	GameSystem::GetInstance()->GetLogStream() << message << std::endl;
	OutputDebugStringA(message.c_str());
}

IDxcBlob* GameSystem::CompileShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler) {

	// ここの中身をこの後に書いていく.

	// 1. hlslファイルを読む.

	// これからシェーダーをコンパイルする旨をログに出す.
	ExportLog(Convert::ConvertString(std::format(L"Begin CompileShader, path:{},profile:{}\n", filePath, profile)));
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
		ExportLog(shaderError->GetStringPointer());
		// 警告・エラー　ダメゼッタイ.
		assert(false);
	}


	// 4. Compile結果を受け取って返す.
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	// 成功したログを出す.
	ExportLog(Convert::ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));
	// もう使わないリソースを解放.
	shaderSource->Release();
	shaderResult->Release();
	// 実行用のバイナリを返却.
	return shaderBlob;
}

Microsoft::WRL::ComPtr<ID3D12Resource> GameSystem::CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes) {
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
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> GameSystem::CreateBufferResource(size_t sizeInBytes) {
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
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = GameSystem::GetInstance()->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GameSystem::CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDiscriptors, bool shaderVisible) {
	// ディスクリプタヒープの生成.
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType; // レンダーターゲットビュー用.
	descriptorHeapDesc.NumDescriptors = numDiscriptors; // ダブルバッファ用に2つ。多くてもかまわない.
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	// ディスクリプタヒープが作れなかったので起動できない.
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}

void GameSystem::SrvDescriptorHeapNumIncrement() {
	if (srvDescriptorHeapNum_ >= kSrvDescriptorHeapNumMax) {
#ifdef _DEBUG

		assert(false, "現在使えるsrvDescriptorHeapのサイズ(128)を使い切りました");

#endif // _DEBUG
		return;
	}
	srvDescriptorHeapNum_++;
}

DirectX::ScratchImage GameSystem::LoadTexture(const std::string& filePath) {
	// テクスチャファイルを読んでプログラムを扱えるようにする.
	DirectX::ScratchImage image{};
	std::wstring filePathW = Convert::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミニマップの作成.
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// ミニマップ付きのデータを返す.
	return mipImages;
}

Microsoft::WRL::ComPtr<ID3D12Resource> GameSystem::CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metaData) {
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

	// 3. Resourceを生成する.

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
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

Microsoft::WRL::ComPtr<ID3D12Resource> GameSystem::UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages, Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList) {

	std::vector<D3D12_SUBRESOURCE_DATA> subresource;
	DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresource);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresource.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(device.Get(), intermediateSize);
	UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, UINT(subresource.size()), subresource.data());
	// Textureへの転用後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する.
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	return intermediateResource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> GameSystem::CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height) {
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
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties, // Heapの設定.
		D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定。特になし.
		&resourceDesc, // Resource設定.
		D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値を書き込む状態にしておく.
		&depthClearValue, // Clear最適値.
		IID_PPV_ARGS(&resource)); // 作成するResourceポインタへのポインタ.

	assert(SUCCEEDED(hr));

	return resource;

}

D3D12_CPU_DESCRIPTOR_HANDLE GameSystem::GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

//D3D12_CPU_DESCRIPTOR_HANDLE GameSystem::GetCPUDescriptorHandle(){
//	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
//	handleCPU.ptr += (descriptorSizeSRV * srvDescriptorHeapNum_);
//	return handleCPU;
//}

D3D12_GPU_DESCRIPTOR_HANDLE GameSystem::GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

//D3D12_GPU_DESCRIPTOR_HANDLE GameSystem::GetGPUDescriptorHandle(){
//	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
//	handleGPU.ptr += (descriptorSizeSRV * srvDescriptorHeapNum_);
//	return handleGPU;
//}
