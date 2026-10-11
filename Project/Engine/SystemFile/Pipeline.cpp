#pragma comment(lib,"dxcompiler.lib")
#include "Pipeline.h"
#include "GameSystem.h"

void Pipeline::Initialize(){
	DXCompilerInitialize();
	ShaderCompile();
	CreateAllPipeline();
}

void Pipeline::CreateAllPipeline() {
	for (uint32_t i = 0; i < static_cast<uint32_t>(BlendMode::kCount); i++) {
		pipelineObject3d_[i] = PipelineCreate(static_cast<BlendMode>(i),ShaderType::kNormal,D3D12_DEPTH_WRITE_MASK_ALL,kUsingFilter_,D3D12_CULL_MODE_BACK);
	}

	for (uint32_t i = 0; i < static_cast<uint32_t>(BlendMode::kCount); i++) {
		pipelineSprite_[i] = PipelineCreate(static_cast<BlendMode>(i),ShaderType::kNormal,D3D12_DEPTH_WRITE_MASK_ALL,kUsingFilter_,D3D12_CULL_MODE_NONE);
	}

	for (uint32_t i = 0; i < static_cast<uint32_t>(BlendMode::kCount); i++) {
		pipelineSprite_[i] = PipelineCreate(static_cast<BlendMode>(i),ShaderType::kParticle,D3D12_DEPTH_WRITE_MASK_ALL,kUsingFilter_,D3D12_CULL_MODE_NONE);
	}

	pipelineLine_ = PipelineCreate(BlendMode::kNormal, ShaderType::kLine, D3D12_DEPTH_WRITE_MASK_ALL, kUsingFilter_, D3D12_CULL_MODE_NONE);
}

void Pipeline::ShaderCompile() {
	vertexShaderBlobParticle = CompileShader(L"./Resources/Shaders/Particle.VS.hlsl", L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	pixelShaderBlobParticle = CompileShader(L"./Resources/Shaders/Particle.PS.hlsl", L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());

	vertexShaderBlobLine = CompileShader(L"./Resources/Shaders/Line.VS.hlsl", L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	pixelShaderBlobLine = CompileShader(L"./Resources/Shaders/Line.PS.hlsl", L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());

	vertexShaderBlob3dObject = CompileShader(L"./Resources/Shaders/Object3d.VS.hlsl", L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	pixelShaderBlob3dObject = CompileShader(L"./Resources/Shaders/Object3d.PS.hlsl", L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());

	vertexShaderBlobNoTexture = CompileShader(L"./Resources/Shaders/NoTexture.VS.hlsl", L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());
	pixelShaderBlobNoTexture = CompileShader(L"./Resources/Shaders/NoTexture.PS.hlsl", L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get());

}

PipelineData Pipeline::PipelineCreate(BlendMode blendMode, ShaderType shaderType, D3D12_DEPTH_WRITE_MASK depthMask, D3D12_FILTER usingFilter, D3D12_CULL_MODE cullMode) {
	// DepthStencilStateの設定.
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc = CreateDepthStencilDesc(depthMask, shaderType);

	/*=============================================================
	PSO(どこに書けばいいかわからぬ).
	=============================================================*/
	// 【RootSignature作成】
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// RootParameter作成。複数設定出来るので配列。
	CreateRootParameters(descriptionRootSignature, shaderType);



	// Samplerの設定.
	CreateStaticSamplers(descriptionRootSignature, shaderType, usingFilter);

	// シリアライズしてバイナリにする.
	PipelineData pipeline;
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		GameSystem::ExportLog(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成.
	hr = GameSystem::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pipeline.rootSignature));
	assert(SUCCEEDED(hr));

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc = CreateInputRayout(shaderType);

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
	rasterizerDesc.CullMode = cullMode;
	// 三角形の中を塗りつぶす.
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする.
	ShaderBlobs shaderBlobs;
	SelectShaderBlobs(shaderBlobs,shaderType);

	// 【PSO】
	graphicsPipelineStateDesc.pRootSignature = pipeline.rootSignature.Get(); // RootSignature.
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc; // InputLayout.
	graphicsPipelineStateDesc.VS = { shaderBlobs.vertexShaderBlob->GetBufferPointer(),shaderBlobs.vertexShaderBlob->GetBufferSize() }; // VertexShader.
	graphicsPipelineStateDesc.PS = { shaderBlobs.pixelShaderBlob->GetBufferPointer(),shaderBlobs.pixelShaderBlob->GetBufferSize() }; // PixelShader.
	graphicsPipelineStateDesc.BlendState = blendDesc; // BlendState.
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState.
	// 書き込むRTVの情報.
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するとトポロジ(形状)のタイプ。三角形.
	if (shaderType == ShaderType::kLine) {
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
	hr = GameSystem::GetInstance()->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipeline.graphicsPipelineState));
	assert(SUCCEEDED(hr));

	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootSignature(pipeline.rootSignature.Get());
	GameSystem::GetInstance()->GetCommandList()->SetPipelineState(pipeline.graphicsPipelineState.Get()); // PS0を設定.
	return pipeline;
}

PipelineData Pipeline::GetPSO(BlendMode blendMode, ShaderType shaderType){
	switch (shaderType){
	case ShaderType::kNormal:
		return GetObject3dPSO(blendMode);
		break;
	case ShaderType::kLine:
		return GetLinePSO();
		break;
	case ShaderType::kParticle:
		return GetParticlePSO(blendMode);
		break;
	case ShaderType::kSprite:
		return GetSpritePSO(blendMode);
		break;
	}

	return GetObject3dPSO(BlendMode::kNormal);
}


Microsoft::WRL::ComPtr<ID3D12Resource> Pipeline::CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height) {
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

D3D12_DEPTH_STENCIL_DESC Pipeline::CreateDepthStencilDesc(D3D12_DEPTH_WRITE_MASK mask, ShaderType shaderType) {
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

	return depthStencilDesc;
}

void Pipeline::CreateDescriptorRange(D3D12_DESCRIPTOR_RANGE* descriptorRange, UINT baseShaderRegister, UINT numDescriptors) {
	descriptorRange[0].BaseShaderRegister = baseShaderRegister; // 0から始まる.
	descriptorRange[0].NumDescriptors = numDescriptors; // 数は1つ.
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う.
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetの自動計算.
}

void Pipeline::CreateConstantBufferView(D3D12_ROOT_PARAMETER& rootParameter, D3D12_SHADER_VISIBILITY shaderVisibility, UINT descriptorShaderRegister) {
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う.
	rootParameter.ShaderVisibility = shaderVisibility; // 使うシェーダー.
	rootParameter.Descriptor.ShaderRegister = descriptorShaderRegister; // レジスタ番号.
}

void Pipeline::CreateDescriptorTable(D3D12_ROOT_PARAMETER& rootParameter, D3D12_SHADER_VISIBILITY shaderVisibility, D3D12_DESCRIPTOR_RANGE* descriptorRange, UINT numDescriptorRanges) {
	
	// SpotLightData.
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //DescriptorTableを使う.
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
	rootParameter.DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定.
	rootParameter.DescriptorTable.NumDescriptorRanges = numDescriptorRanges; // Tableで利用する数.
}

void Pipeline::CreateRootParameters(D3D12_ROOT_SIGNATURE_DESC& rootSignature, ShaderType shaderType) {
	CreateDescriptorRange(descriptorRangeForInstancing, 0, 1);
	CreateDescriptorRange(descriptorRange, 0, 1);
	CreateDescriptorRange(pointLightDescriptorRange, 1, 1);
	CreateDescriptorRange(spotLightDescriptorRange, 2, 1);

	switch (shaderType) {
	case ShaderType::kParticle:
		CreateConstantBufferView(rootParametersParticle[0],D3D12_SHADER_VISIBILITY_PIXEL, 0);
		CreateDescriptorTable(rootParametersParticle[1],D3D12_SHADER_VISIBILITY_PIXEL, descriptorRangeForInstancing, _countof(descriptorRangeForInstancing));
		CreateDescriptorTable(rootParametersParticle[2],D3D12_SHADER_VISIBILITY_PIXEL, descriptorRange, _countof(descriptorRange));
		rootSignature.pParameters = rootParametersParticle; // ルートパラメータ配列へのポインタ.
		rootSignature.NumParameters = _countof(rootParametersParticle); // 配列の長さ.
		return;
		break;
	case ShaderType::kNoTexture:
		CreateConstantBufferView(rootParametersNoTexture[0], D3D12_SHADER_VISIBILITY_PIXEL, 0);
		CreateConstantBufferView(rootParametersNoTexture[1], D3D12_SHADER_VISIBILITY_VERTEX, 0);
		// DirectionalLightData.
		 CreateConstantBufferView(rootParametersNoTexture[2],D3D12_SHADER_VISIBILITY_PIXEL, 1);
		// CameraData.
		 CreateConstantBufferView(rootParametersNoTexture[3],D3D12_SHADER_VISIBILITY_PIXEL, 2);
		// PointLightData.
		CreateDescriptorTable(rootParametersNoTexture[4],D3D12_SHADER_VISIBILITY_PIXEL, pointLightDescriptorRange, _countof(pointLightDescriptorRange));
		// SpotLightData.
		CreateDescriptorTable(rootParametersNoTexture[5],D3D12_SHADER_VISIBILITY_PIXEL, spotLightDescriptorRange, _countof(spotLightDescriptorRange));
		// LightNumData.
		CreateConstantBufferView(rootParametersNoTexture[6],D3D12_SHADER_VISIBILITY_PIXEL, 3);

		rootSignature.pParameters = rootParametersNoTexture; // ルートパラメータ配列へのポインタ.
		rootSignature.NumParameters = _countof(rootParametersNoTexture); // 配列の長さ.
		return;
		break;
	case ShaderType::kLine:
		CreateConstantBufferView(rootParametersLine[0],D3D12_SHADER_VISIBILITY_VERTEX, 0);

		rootSignature.pParameters = rootParametersLine; // ルートパラメータ配列へのポインタ.
		rootSignature.NumParameters = _countof(rootParametersLine); // 配列の長さ.
		return;
		break;
	default:
		CreateConstantBufferView(rootParameters[0], D3D12_SHADER_VISIBILITY_PIXEL, 0);
		CreateConstantBufferView(rootParameters[1], D3D12_SHADER_VISIBILITY_VERTEX, 0);
		CreateDescriptorTable(rootParameters[2],D3D12_SHADER_VISIBILITY_PIXEL, descriptorRange, _countof(descriptorRange));
		// DirectionalLightData.
		CreateConstantBufferView(rootParameters[3],D3D12_SHADER_VISIBILITY_PIXEL, 1);
		// CameraData.
		CreateConstantBufferView(rootParameters[4],D3D12_SHADER_VISIBILITY_PIXEL, 2);
		// PointLightData.
		CreateDescriptorTable(rootParameters[5],D3D12_SHADER_VISIBILITY_PIXEL, pointLightDescriptorRange, _countof(pointLightDescriptorRange));
		// SpotLightData.
		CreateDescriptorTable(rootParameters[6],D3D12_SHADER_VISIBILITY_PIXEL, spotLightDescriptorRange, _countof(spotLightDescriptorRange));
		// LightNumData.
		CreateConstantBufferView(rootParameters[7],D3D12_SHADER_VISIBILITY_PIXEL, 3);

		rootSignature.pParameters = rootParameters; // ルートパラメータ配列へのポインタ.
		rootSignature.NumParameters = _countof(rootParameters); // 配列の長さ.
		return;
		break;
	}

	assert(false && "存在しないShaderTypeです");
}

void Pipeline::CreateStaticSamplers(D3D12_ROOT_SIGNATURE_DESC& rootSignature, ShaderType shaderType, D3D12_FILTER filter) {
	if (shaderType == ShaderType::kLine || shaderType == ShaderType::kNoTexture) {
		return ;
	}

	staticSamplers[0].Filter = filter; // バイリニアフィルタ.(重要!ゲーム的にここをLINEARからPOINTに切り替えています)
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲外をリピート.
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない.
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipmapを使う.
	staticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う.
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う.
	rootSignature.pStaticSamplers = staticSamplers;
	rootSignature.NumStaticSamplers = _countof(staticSamplers);
	return ;
}

D3D12_INPUT_LAYOUT_DESC Pipeline::CreateInputRayout(ShaderType shaderType) {
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	// 【InputLayout】
	switch (shaderType) {
	case ShaderType::kNoTexture:
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
		return inputLayoutDesc;
		break;
	case ShaderType::kLine:
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
		return inputLayoutDesc;
		break;
	default:
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
		return inputLayoutDesc;
		break;
	}

	return inputLayoutDesc;
}

void Pipeline::SelectShaderBlobs(ShaderBlobs& shaderBlobs, ShaderType shaderType) {

	switch (shaderType) {
	case ShaderType::kNoTexture:
		shaderBlobs.vertexShaderBlob = vertexShaderBlobNoTexture;
		shaderBlobs.pixelShaderBlob = pixelShaderBlobNoTexture;
		break;
	case ShaderType::kLine:
		shaderBlobs.vertexShaderBlob = vertexShaderBlobLine;
		shaderBlobs.pixelShaderBlob = pixelShaderBlobLine;
		break;
	case ShaderType::kParticle:
		shaderBlobs.vertexShaderBlob = vertexShaderBlobParticle;
		shaderBlobs.pixelShaderBlob = pixelShaderBlobParticle;
		break;
	default:
		shaderBlobs.vertexShaderBlob = vertexShaderBlob3dObject;
		shaderBlobs.pixelShaderBlob = pixelShaderBlob3dObject;
		break;
	}

	assert(shaderBlobs.vertexShaderBlob != nullptr);
	assert(shaderBlobs.pixelShaderBlob != nullptr);
}

void Pipeline::DXCompilerInitialize() {
	/*=============================================================
	DXCの初期化.
	=============================================================*/
	// dxcCompilerを初期化.
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));

	// 現時点でincludeはしないが、includeに対応するための設定を行っておく.
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

IDxcBlob* Pipeline::CompileShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler) {

	// ここの中身をこの後に書いていく.

	// 1. hlslファイルを読む.

	// これからシェーダーをコンパイルする旨をログに出す.
	GameSystem::GetInstance()->ExportLog(Convert::ConvertString(std::format(L"Begin CompileShader, path:{},profile:{}\n", filePath, profile)));
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
		GameSystem::GetInstance()->ExportLog(shaderError->GetStringPointer());
		// 警告・エラー　ダメゼッタイ.
		assert(false);
	}


	// 4. Compile結果を受け取って返す.
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	// 成功したログを出す.
	GameSystem::GetInstance()->ExportLog(Convert::ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));
	// もう使わないリソースを解放.
	shaderSource->Release();
	shaderResult->Release();
	// 実行用のバイナリを返却.
	return shaderBlob;
}