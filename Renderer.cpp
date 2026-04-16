#include "Renderer.h"
#include "GameSystem.h"
#include <vector>

void Renderer::Model::Initialize(const ModelInfo& info) {
	// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)

	isVisible_;
	isVisible_ = true;

	modelInfo_ = info;

	vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * modelInfo_.modelData.vertices.size());

	// 【MaterialResourceを生成する】
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResource 
	materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む.
	//Material* materialData = nullptr;
	// 書き込むためのアドレスを取得.
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// 今回は赤を書き込んでみる
	materialData_->color = {1.0f, 1.0f, 1.0f, 1.0f};
	materialData_->enableLighting = true;
	materialData_->uvTransform = Matrix4x4::Identity();
	
	// 【TransformationMatrix】
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する.
	//Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource 
	wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む.
	//TransformationMatrix* wvpData = nullptr;
	// 書き込むためのアドレスを取得.
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	// 単位行列を書き込んでおく.
	wvpData_->WVP = Matrix4x4::Identity();
	wvpData_->World = Matrix4x4::Identity();


	// 【VertexBufferViewを作成する】

	// 頂点バッファビューを作成する.
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// リソースの先頭のアドレスから使う.
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ.(多分ここは他の場所でも変えられる。Rendererから頂点数取ってきて代入とかできそう)
	//vertexBufferView.SizeInBytes = sizeof(VertexData) * kSubdivision * kSubdivision * 4;
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelInfo_.modelData.vertices.size());
	// 1頂点あたりのサイズ.
	vertexBufferView_.StrideInBytes = sizeof(VertexData);


	// 【Resourceにデータを書き込む】

	// 頂点リソースにデータを書き込む.
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得.
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std:memcpy(vertexData, modelInfo_.modelData.vertices.data(), sizeof(VertexData) * modelInfo_.modelData.vertices.size());

	
}

void Renderer::Model::Draw(const Transform& transform){
	if (!isVisible_) {
		return;
	}
	
	Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform);

	wvpData_->World = worldMatrix;
	wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);

	/*=============================================================
	三角形の描画のコマンド.
	=============================================================*/
	//D3D12_VIEWPORT viewport = GameSystem::GetInstance()->GetViewport();
	//D3D12_RECT scissorRect = GameSystem::GetInstance()->GetScissorRect();
	//
	//GameSystem::GetInstance()->GetCommandList()->RSSetViewports(1, &viewport); // Viewportを設定.
	//GameSystem::GetInstance()->GetCommandList()->RSSetScissorRects(1, &scissorRect); // Scissorを設定.
	//// RootSignatureを設定。PS0に設定しているけど別途設定が必要.
	//GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootSignature(GameSystem::GetInstance()->GetRootSignature().Get());
	//GameSystem::GetInstance()->GetCommandList()->SetPipelineState(GameSystem::GetInstance()->GetGraphicsPipelineState().Get()); // PS0を設定.
	GameSystem::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定.
	//commandList->IASetIndexBuffer(&indexBufferView); // IBVを設定.
	// 形状を設定。PS0に設定しているものとはまた別。同じものを設定すると考えておけば良い.
	GameSystem::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// CBufferの場所を設定.
	// マテリアル用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// WVP用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭の設定。2はrootParameter[2]である.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(2, modelInfo_.textureSrvHandlesGPU);
	// DirectionalLight用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(3, DirectionalLight::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後.
	//commandList->DrawIndexedInstanced(kSubdivision* kSubdivision * 6, 1, 0, 0,0);
	GameSystem::GetInstance()->GetCommandList()->DrawInstanced(UINT(modelInfo_.modelData.vertices.size()), 1, 0, 0);
}

void Renderer::Sphere::Initialize(TextureInfo info){
	isVisible_ = true;

	textureInfo_ = info;
	// 実際に頂点リソースを作る.(ここの量は多い分にはバグらない、その代わり不可がかかるんちゃうかな)
	//Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource 
	vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(),sizeof(VertexData) * kSubdivision_ * kSubdivision_ * 6);

	// 【MaterialResourceを生成する】
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResource 
	materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む.
	//Material* materialData = nullptr;
	// 書き込むためのアドレスを取得.
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// 今回は赤を書き込んでみる
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->enableLighting = true;
	materialData_->uvTransform = Matrix4x4::Identity();

	// 【TransformationMatrix】
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する.
	//Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource 
	wvpResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む.
	//TransformationMatrix* wvpData = nullptr;
	// 書き込むためのアドレスを取得.
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	// 単位行列を書き込んでおく.
	wvpData_->WVP = Matrix4x4::Identity();
	wvpData_->World = Matrix4x4::Identity();


	// 【VertexBufferViewを作成する】

	// 頂点バッファビューを作成する.
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// リソースの先頭のアドレスから使う.
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ.(多分ここは他の場所でも変えられる。Rendererから頂点数取ってきて代入とかできそう)
	//vertexBufferView.SizeInBytes = sizeof(VertexData) * kSubdivision * kSubdivision * 4;
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * kSubdivision_ * kSubdivision_ * 4);
	// 1頂点あたりのサイズ.
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 【IndexResourceを生成する】
	// 実際に頂点リソースを作る.
	//Microsoft::WRL::ComPtr<ID3D12Resource> indexResource 
	indexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(uint32_t) * kSubdivision_ * kSubdivision_ * 6);

	// 頂点バッファビューを作成する.
	//D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	// リソースの先頭のアドレスから使う.
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ.
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * kSubdivision_ * kSubdivision_ * 6;
	// インデックスはuint32_tとする.
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;


	// 【Resourceにデータを書き込む】

	// 頂点リソースにデータを書き込む.
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得.
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// インデックスリソースにデータを書き込む.
	uint32_t* indexData = nullptr;
	// 書き込むためのアドレスを取得.
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));


	// スフィアの描画プログラム.(いつかRendererに入れる)
	const float kLonEvery = std::numbers::pi_v<float> *2.0f / kSubdivision_;
	const float kLatEvery = std::numbers::pi_v<float> / kSubdivision_;

	for (uint32_t latIndex = 0; latIndex < kSubdivision_; latIndex++) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision_; lonIndex++) {
			uint32_t start = (latIndex * kSubdivision_ + lonIndex) * 4;
			uint32_t indexStart = (latIndex * kSubdivision_ + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;

			float u = static_cast<float>(lonIndex) / static_cast<float>(kSubdivision_);
			float v = 1.0f - static_cast<float>(latIndex) / static_cast<float>(kSubdivision_);

			vertexData[start].position = { cos(lat) * cos(lon),sin(lat),cos(lat) * sin(lon) ,1.0f };
			vertexData[start].texcoord = { u - 1.0f / static_cast<float>(kSubdivision_),v };
			vertexData[start + 1].position = { cos(lat + std::numbers::pi_v<float> / kSubdivision_) * cos(lon),sin(lat + std::numbers::pi_v<float> / kSubdivision_),cos(lat + std::numbers::pi_v<float> / kSubdivision_) * sin(lon) ,1.0f };
			vertexData[start + 1].texcoord = { u - 1.0f / static_cast<float>(kSubdivision_) ,v - 1.0f / static_cast<float>(kSubdivision_) };
			vertexData[start + 2].position = { cos(lat) * cos(lon + std::numbers::pi_v<float> *2.0f / kSubdivision_),sin(lat),cos(lat) * sin(lon + std::numbers::pi_v<float> *2.0f / kSubdivision_) ,1.0f };
			vertexData[start + 2].texcoord = { u ,v };
			vertexData[start + 3].position = { cos(lat + std::numbers::pi_v<float> / kSubdivision_) * cos(lon + std::numbers::pi_v<float> *2.0f / kSubdivision_),sin(lat + std::numbers::pi_v<float> / kSubdivision_),cos(lat + std::numbers::pi_v<float> / kSubdivision_) * sin(lon + std::numbers::pi_v<float> *2.0f / kSubdivision_),1.0f };
			vertexData[start + 3].texcoord = { u ,v - 1.0f / static_cast<float>(kSubdivision_) };

			indexData[indexStart] = start;
			indexData[indexStart + 1] = start + 1;
			indexData[indexStart + 2] = start + 2;
			indexData[indexStart + 3] = start + 1;
			indexData[indexStart + 4] = start + 3;
			indexData[indexStart + 5] = start + 2;


			for (uint32_t i = 0; i < 4; i++) {
				vertexData[start + i].normal.x = vertexData[start + i].position.x;
				vertexData[start + i].normal.y = vertexData[start + i].position.y;
				vertexData[start + i].normal.z = vertexData[start + i].position.z;
			}
		}
	}
}

void Renderer::Sphere::Draw(const Transform& transform) {
	if (!isVisible_) {
		return;
	}
	
	Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform);

	wvpData_->World = worldMatrix;
	wvpData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrix(worldMatrix);
	/*=============================================================
	三角形の描画のコマンド.
	=============================================================*/
	//D3D12_VIEWPORT viewport = GameSystem::GetInstance()->GetViewport();
	//D3D12_RECT scissorRect = GameSystem::GetInstance()->GetScissorRect();
	//
	//GameSystem::GetInstance()->GetCommandList()->RSSetViewports(1, &viewport); // Viewportを設定.
	//GameSystem::GetInstance()->GetCommandList()->RSSetScissorRects(1, &scissorRect); // Scissorを設定.
	//// RootSignatureを設定。PS0に設定しているけど別途設定が必要.
	//GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootSignature(GameSystem::GetInstance()->GetRootSignature().Get());
	//GameSystem::GetInstance()->GetCommandList()->SetPipelineState(GameSystem::GetInstance()->GetGraphicsPipelineState().Get()); // PS0を設定.
	GameSystem::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定.
	GameSystem::GetInstance()->GetCommandList()->IASetIndexBuffer(&indexBufferView_); // IBVを設定.
	// 形状を設定。PS0に設定しているものとはまた別。同じものを設定すると考えておけば良い.
	GameSystem::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// CBufferの場所を設定.
	// マテリアル用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// WVP用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭の設定。2はrootParameter[2]である.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureInfo_.textureSrvHandlesGPU);
	// DirectionalLight用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(3, DirectionalLight::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後.
	GameSystem::GetInstance()->GetCommandList()->DrawIndexedInstanced(kSubdivision_* kSubdivision_ * 6, 1, 0, 0,0);
	//GameSystem::GetInstance()->GetCommandList()->DrawInstanced(UINT(modelInfo_.modelData.vertices.size()), 1, 0, 0);
}

void Renderer::Sprite::Initialize(TextureInfo info){
	isVisible_ = true;

	textureInfo_ = info;
	/*=============================================================
	Sprite用のResourceとView.
	=============================================================*/
	// 【VertexResourceを生成する】
	// 実際に頂点リソースを作る.
	//Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite 
	vertexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(VertexData) * 4);

	// 頂点バッファビューを作成する.
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	// リソースの先頭のアドレスから使う.
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ.
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点あたりのサイズ.
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 【IndexResourceを生成する】
	// 実際に頂点リソースを作る.
	//Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite 
	indexResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(uint32_t) * 6);

	// 頂点バッファビューを作成する.
	//D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	// リソースの先頭のアドレスから使う.
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ.
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする.
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;


	// 【MaterialResourceを生成する】
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する.
	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite
	materialResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む.
	//Material* materialDataSprite = nullptr;
	// 書き込むためのアドレスを取得.
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// 今回は赤を書き込んでみる
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->enableLighting = false;
	materialData_->uvTransform = Matrix4x4::Identity();

	// 【TransformationMatrix】
	//Sprite用のTransformationMatrixを作る。Matrix4x4 1つ分のサイズを用意する.
	//Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite 
	transformationMatrixResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む.
	//TransformationMatrix* transformationMatrixDataSprite = nullptr;
	// 書き込むためのアドレスを取得.
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	// 単位行列を書き込んでおく.
	transformationMatrixData_->WVP = Matrix4x4::Identity();
	transformationMatrixData_->World = Matrix4x4::Identity();

	// 【Resourceにデータを書き込む】

	// 頂点リソースにデータを書き込む.
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得.
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 1枚目の三角形.
	vertexData[0].position = { 0.0f,360.0f,0.0f,1.0f }; // 左下.
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[0].normal = { 0.0f,0.0f,-1.0f };
	vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f }; // 左上.
	vertexData[1].texcoord = { 0.0f,0.0f };
	vertexData[1].normal = { 0.0f,0.0f,-1.0f };
	vertexData[2].position = { 640.0f,360.0f,0.0f,1.0f }; // 右下.
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[2].normal = { 0.0f,0.0f,-1.0f };
	vertexData[3].position = { 640.0f,0.0f,0.0f,1.0f }; // 右上.
	vertexData[3].texcoord = { 1.0f,0.0f };
	vertexData[3].normal = { 0.0f,0.0f,-1.0f };

	// インデックスリソースにデータを書き込む.
	uint32_t* indexDataSprite = nullptr;
	// 書き込むためのアドレスを取得.
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	// 1枚目の三角形.
	indexDataSprite[0] = 0;
	indexDataSprite[1] = 1;
	indexDataSprite[2] = 2;
	indexDataSprite[3] = 1;
	indexDataSprite[4] = 3;
	indexDataSprite[5] = 2;
}

void Renderer::Sprite::Draw(const Transform& transform) {
	if (!isVisible_) {
		return;
	}

	Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform);

	transformationMatrixData_->World = worldMatrix;
	transformationMatrixData_->WVP = Camera::GetInstance()->GetWorldViewProjectionMatrixSprite(worldMatrix);
	/*=============================================================
	三角形のSpriteの描画のコマンド.
	=============================================================*/
	// Spriteの描画。変更が必要なものだけ変更する.
	GameSystem::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定.
	GameSystem::GetInstance()->GetCommandList()->IASetIndexBuffer(&indexBufferView_); // IBVを設定.
	// 形状を設定。PS0に設定しているものとはまた別。同じものを設定すると考えておけば良い.
	GameSystem::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// マテリアル用のCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// transformationMatrixCBufferの場所.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭の設定。2はrootParameter[2]である.
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureInfo_.textureSrvHandlesGPU);
	GameSystem::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(3, DirectionalLight::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後.
	GameSystem::GetInstance()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0,0);


	
}