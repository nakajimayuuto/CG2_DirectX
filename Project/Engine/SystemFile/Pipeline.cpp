#include "Pipeline.h"
#include "GameSystem.h"

void Pipeline::CreateAllPipeline(){

}

void Pipeline::PipelineCreate(BlendMode blendMode, D3D12_DEPTH_WRITE_MASK mask) {
	/*=============================================================
	DepthStencilTextureをつくる
	=============================================================*/
	depthStencilResource = CreateDepthStencilTextureResource(GameSystem::GetInstance()->GetDevice(), static_cast<int32_t>(Environment::GetInstance()->GetWindowSize().width), static_cast<int32_t>(Environment::GetInstance()->GetWindowSize().height));

	
	// DSVの設定.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Format。基本的にResourceに合わせる.
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2dTexture.
	// DSVHeapの先頭にDSVをつくる.
	GameSystem::GetInstance()->GetDevice()->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, GameSystem::GetInstance()-GetCPUDescriptorHandleForHeapStart());

	// DepthStencilStateの設定.
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc = CreateDepthStencilDesc(mask);

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
		staticSamplers[0].Filter = kUsingFilter_; // バイリニアフィルタ.(重要!ゲーム的にここをLINEARからPOINTに切り替えています)
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

	} else if (shaderType == ShaderType::kNoTexture) {
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
	} else if (blendMode == BlendMode::kLine) {
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

D3D12_DEPTH_STENCIL_DESC Pipeline::CreateDepthStencilDesc(D3D12_DEPTH_WRITE_MASK mask) {
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
}