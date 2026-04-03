#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

#include <wrl.h>

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
#include <numbers>
#include "Vector4.h"
#include "Vertex.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "Camera.h"
#include "Math.h"

#include "Material.h"
#include "DirectionalLight.h"

#include "ModelData.h"
#include <fstream>
#include <sstream>

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"


#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // USE_IMGUI

struct D3DResourceLeakChecker {
	~D3DResourceLeakChecker()
	{
		// リソースリークチェック.
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
			//debug->Release();
		}
	}
};

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
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes) {
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

// DescriptorHeap関数(どうやってファイル分けするかね).
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
	Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDiscriptors, bool shaderVisible) {
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
Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metaData) {
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
Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages, Microsoft::WRL::ComPtr<ID3D12Device> device,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList) {

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
};

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device,int32_t width, int32_t height){
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

};

D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDiscriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap,uint32_t descriptorSize,uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDiscriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap,uint32_t descriptorSize,uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

// ModelManager的n(以下略.
MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& fileName) {
	// 1. 中で必要となる変数の宣言.
	MaterialData materialData; // 構築するModelData.
	std::string line; // ファイルから読んだ1行を格納するもの.


	// 2. ファイルを開く.
	std::ifstream file(directoryPath + "/" + fileName); // ファイルを開く.
	assert(file.is_open()); // とりあえず開けなかったら止める.
	

	// 3. 実際にファイルを読み、MaterialDataを構築していく.
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を読む.

		// identifierに応じた処理.

		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする.
			materialData.textureFilePath = directoryPath+"/"+textureFilename;
		}
	}


	// 4. MaterialDataを返す.

	return materialData;
}

// ModelManager的な奴に入れる.
ModelData LoadObjFile(const std::string& directoryPath, const std::string& fileName) {
	// 1. 中で必要となる変数の宣言.
	ModelData modelData; // 構築するModelData.
	std::vector<Vector4> positions; // 位置.
	std::vector<Vector3> normals; // 法線.
	std::vector<Vector2> texcoords; // テクスチャ座標.
	std::string line; // ファイルから読んだ1行を格納するもの.


	// 2. ファイルを開く.
	std::ifstream file(directoryPath + "/" + fileName); // ファイルを開く.
	assert(file.is_open()); // とりあえず開けなかったら止める.


	// 3. 実際にファイルを読み、ModelDataを構築していく.
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を読む.

		// identifierに応じた処理.

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.x *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			VertexData triangle[3];
			// 面は三角形限定。その他は未対応.
			for (uint32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する.
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (uint32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');// 区切りでインデックスを読んでいく.
					elementIndices[element] = std::stoi(index);
				}
				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築する.
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				//VertexData vertex = {position,texcoord,normal};
				//modelData.vertices.push_back(vertex);
				triangle[faceVertex] = { position,texcoord,normal };
			}

			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {
			// MaterialTemplateLiblaryファイルの名前を取得する.
			std::string materialFilename;
			s >> materialFilename;
			// 基本的にObjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す.
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	// 4. ModelDataを返す.


	return modelData;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	

	/*=============================================================
	ここから下がゲームの変数.
	=============================================================*/



	bool isTriangleAutoMove = false;

	//bool useMonsterBall = true;
	uint32_t textureNumber = 2;

	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Transform uvTransformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };


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

			// 球のもろもろ.
#ifdef USE_IMGUI
			ImGui::Begin("Triangle");
			ImGui::SliderFloat3("scale", reinterpret_cast<float*>(&transform.scale), 0.0f, 2.0f);
			ImGui::SliderFloat3("rotate", reinterpret_cast<float*>(&transform.rotate), 0.0f, Radian(360.0f));
			ImGui::SliderFloat3("translate", reinterpret_cast<float*>(&transform.translate), -5.0f, 5.0f);

			Vector4 imColor = materialData->color;

			ImGui::ColorEdit4("color", reinterpret_cast<float*>(&imColor));

			materialData->color = imColor;

			//ImGui::Checkbox("useMonsterBall",&useMonsterBall);
			//ImGui::SliderInt("texture", reinterpret_cast<int*>(textureNumber),0,2);

			ImGui::Checkbox("enableLighting", reinterpret_cast<bool*>(&materialData->enableLighting));

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

			ImGui::Begin("Sprite");
			ImGui::SliderFloat2("scale", reinterpret_cast<float*>(&transformSprite.scale), 0.0f, 2.0f);
			ImGui::SliderFloat("rotate", reinterpret_cast<float*>(&transformSprite.rotate.z), 0.0f, Radian(360.0f));
			ImGui::SliderFloat2("translate", reinterpret_cast<float*>(&transformSprite.translate), -640.0f, 1280.0f);

			ImGui::SliderFloat2("UVScale", reinterpret_cast<float*>(&uvTransformSprite.scale), 0.0f, 2.0f);
			ImGui::SliderFloat("UVRotate", reinterpret_cast<float*>(&uvTransformSprite.rotate.z), 0.0f, Radian(360.0f));
			ImGui::SliderFloat2("UVTranslate", reinterpret_cast<float*>(&uvTransformSprite.translate), -640.0f, 1280.0f);

			ImGui::End();


			ImGui::Begin("DirectionalLight");
			imColor = directionalLightData->color;

			ImGui::ColorEdit4("color", reinterpret_cast<float*>(&imColor));

			directionalLightData->color = imColor;

			Vector3 imDirection = directionalLightData->direction;

			ImGui::SliderFloat3("direction", reinterpret_cast<float*>(&imDirection), -1.0f, 1.0f);

			directionalLightData->direction = imDirection.Normalize();

			ImGui::SliderFloat("intensity", &directionalLightData->intensity, 0.0f, 1.0f);

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

			wvpData->World = worldMatrix;
			wvpData->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);

			worldMatrix = Matrix4x4::MakeAffineMatrix(transformSprite);

			transformationMatrixDataSprite->World = worldMatrix;
			transformationMatrixDataSprite->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrixSprite(worldMatrix);

			materialDataSprite->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransformSprite);

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
			float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };// 青っぽい色。RGBAの順.
			commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);



			// 描画用のDescriptorHeapの設定.
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { srvDescriptorHeap.Get()};
			commandList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());


			/*=============================================================
			三角形の描画のコマンド.
			=============================================================*/
			commandList->RSSetViewports(1, &viewport); // Viewportを設定.
			commandList->RSSetScissorRects(1, &scissorRect); // Scissorを設定.
			// RootSignatureを設定。PS0に設定しているけど別途設定が必要.
			commandList->SetGraphicsRootSignature(rootSignature.Get());
			commandList->SetPipelineState(graphicsPipelineState.Get()); // PS0を設定.
			commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定.
			//commandList->IASetIndexBuffer(&indexBufferView); // IBVを設定.
			// 形状を設定。PS0に設定しているものとはまた別。同じものを設定すると考えておけば良い.
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			// CBufferの場所を設定.
			// マテリアル用のCBufferの場所.
			commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
			// WVP用のCBufferの場所.
			commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
			// SRVのDescriptorTableの先頭の設定。2はrootParameter[2]である.
			commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandlesGPU[textureNumber]);
			// DirectionalLight用のCBufferの場所.
			commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
			// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後.
			//commandList->DrawIndexedInstanced(kSubdivision* kSubdivision * 6, 1, 0, 0,0);
			commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

			/*=============================================================
			三角形のSpriteの描画のコマンド.
			=============================================================*/
			// Spriteの描画。変更が必要なものだけ変更する.
			commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite); // VBVを設定.
			commandList->IASetIndexBuffer(&indexBufferViewSprite); // IBVを設定.
			// マテリアル用のCBufferの場所.
			commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
			// transformationMatrixCBufferの場所.
			commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
			// SRVのDescriptorTableの先頭の設定。2はrootParameter[2]である.
			commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandlesGPU[0]);
			commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
			// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後.
			//commandList->DrawIndexedInstanced(6, 1, 0, 0,0);


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
			hr = commandList->Close();
			assert(SUCCEEDED(hr));

			/*=============================================================
			コマンドをキックする.
			=============================================================*/
			//GPUにコマンドリストの実行を行わせる.
			Microsoft::WRL::ComPtr<ID3D12CommandList> commandLists[] = { commandList };
			commandQueue->ExecuteCommandLists(1, commandLists->GetAddressOf());
			//GPUとOSに画面の交換を行うよう通知する.
			swapChain->Present(1, 0);


			// Fanceの値を更新.
			fenceValue++;
			// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る.
			commandQueue->Signal(fence.Get(), fenceValue);


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
			hr = commandList->Reset(commandAllocator.Get(), nullptr);
			assert(SUCCEEDED(hr));
		}
	}

	/*=============================================================
	メモリ解放系.
	=============================================================*/
	CloseHandle(fenceEvent);
	//fence->Release();
	//rtvDiscriptorHeap->Release();
	//swapChainResource[0]->Release();
	//swapChainResource[1]->Release();
	//swapChain->Release();
	//commandList->Release();
	//commandAllocator->Release();
	//commandQueue->Release();
	//device->Release();
	//useAdapter->Release();
	//dxgiFactory->Release();

	//// 三角形の解放開始.
	//vertexResource->Release();
	//indexResource->Release();
	//graphicsPipelineState->Release();
	//signatureBlob->Release();

	//if (errorBlob) {
	//	errorBlob->Release();
	//}

	//rootSignature->Release();
	//pixelShaderBlob->Release();
	//vertexShaderBlob->Release();
	//materialResource->Release();
	//directionalLightResource->Release();
	//wvpResource->Release();
	//// 三角形の解放終了.

	//// Spriteの解放開始.
	//vertexResourceSprite->Release();
	//indexResourceSprite->Release();
	//transformationMatrixResourceSprite->Release();
	//materialResourceSprite->Release();
	//// Spriteの解放終了.


	/*=============================================================
	 ImGuiの終了処理.
	=============================================================*/
	// 初期化と逆順に行う.
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif // USE_IMGUI



	//srvDescriptorHeap->Release();


#ifdef _DEBUG
	//debugController->Release();
#endif // _DEBUG
	CloseWindow(hwnd);

	/*=============================================================
	Texture系の解放.
	=============================================================*/
	//for (uint32_t dataNumber = 0; dataNumber < textureDataMax; dataNumber++) {
	//	textureResource[dataNumber]->Release();
	//}
	//depthStencilResource->Release();
	//dsvDescriptorHeap->Release();

	

	CoUninitialize();

	return 0;
}
