#include "DialogBoxManager.h"

#include <filesystem>
#include <algorithm>
#include <cassert>

#include "../Engine/Renderer/Renderer.h"
#include "../Engine/SystemFile/Convert.h"
#include "TextureManager.h"

namespace {
	void LogDialogBox(const std::string& message) {
		OutputDebugStringA(("[DialogBoxManager] " + message + "\n").c_str());
	}

	// 画像内のピクセル範囲 → UV(サンプリングで隣のマスが混ざらないように半テクセル内側にする).
	void MakeUV(float x0, float y0, float x1, float y1, float textureWidth, float textureHeight, Vector2& outLeftTop, Vector2& outRightBottom) {
		outLeftTop = { (x0 + 0.5f) / textureWidth, (y0 + 0.5f) / textureHeight };
		outRightBottom = { (x1 - 0.5f) / textureWidth, (y1 - 0.5f) / textureHeight };
	}

	// 左上座標と大きさで、UV範囲を指定して1枚描画する.
	void DrawPiece(const TextureInfo& texture, float left, float top, float width, float height, const Vector4& color, const Vector2& uvLeftTop, const Vector2& uvRightBottom) {
		if (width <= 0.0f || height <= 0.0f) {
			return;
		}
		// スプライトは中心が原点なので中心座標に変換する.
		Transform transform = Transform::GetInitialValue(
			{ 1.0f,1.0f,1.0f },
			{ 0.0f,0.0f,0.0f },
			{ left + width * 0.5f, top + height * 0.5f, 0.0f });
		Renderer::GetInstance()->DrawSpriteRect(transform, { width,height }, texture, color, uvLeftTop, uvRightBottom);
	}
}

DialogBoxManager* DialogBoxManager::GetInstance() {
	static DialogBoxManager instance;
	return &instance;
}

/*=============================================================
読み込み.
=============================================================*/
void DialogBoxManager::LoadDialogBox(const std::string& name, const std::string& filePath, uint32_t cellSize) {
	const std::string framePath = outputDirectory_ + "/" + name + "_frame.png";
	const std::string fillPath = outputDirectory_ + "/" + name + "_fill.png";

	uint32_t imageHeight = 0;
	if (!SplitTemplate(filePath, framePath, fillPath, imageHeight)) {
		assert(false && "DialogBoxManager : テンプレート画像の読み込みに失敗しました");
		return;
	}

	DialogBoxData data;
	data.name = name;
	data.filePath = filePath;
	data.cellSize = (cellSize != 0) ? cellSize : imageHeight;

	const std::string frameName = "dialog_box_" + name + "_frame";
	const std::string fillName = "dialog_box_" + name + "_fill";
	if (!TextureManager::GetInstance()->HasTexture(frameName)) {
		TextureManager::GetInstance()->RegisterTexture(frameName, framePath, false);
	}
	if (!TextureManager::GetInstance()->HasTexture(fillName)) {
		TextureManager::GetInstance()->RegisterTexture(fillName, fillPath, false);
	}
	data.frameTexture = TextureManager::GetInstance()->GetTextureInfo(frameName);
	data.fillTexture = TextureManager::GetInstance()->GetTextureInfo(fillName);

	// 使うマス(中央のマス)まで画像の幅が足りているか.
	assert(data.frameTexture.width >= data.cellSize * (kCenterCell_ + 1) && "DialogBoxManager : テンプレート画像のマスが足りません");

	dialogBoxes_[name] = data;

	// 最初に読み込んだものを使用中にする.
	if (currentName_.empty()) {
		currentName_ = name;
	}
}

bool DialogBoxManager::SplitTemplate(const std::string& filePath, const std::string& framePath, const std::string& fillPath, uint32_t& outHeight) {
	namespace fs = std::filesystem;
	std::error_code ec;

	const std::wstring filePathW = Convert::ConvertString(filePath);

	// 元画像の大きさだけ先に読む.
	DirectX::TexMetadata metadata{};
	HRESULT hr = DirectX::GetMetadataFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_NONE, metadata);
	if (FAILED(hr)) {
		LogDialogBox("画像が読めません : " + filePath);
		return false;
	}
	outHeight = static_cast<uint32_t>(metadata.height);

	// 生成済みの画像が元画像より新しければ作り直さない.
	if (fs::exists(framePath, ec) && fs::exists(fillPath, ec) && fs::exists(filePath, ec)) {
		auto sourceTime = fs::last_write_time(filePath, ec);
		if (!ec && fs::last_write_time(framePath, ec) >= sourceTime && fs::last_write_time(fillPath, ec) >= sourceTime) {
			return true;
		}
	}

	// 元画像を読み込む(RGBA8に揃える).
	DirectX::ScratchImage source;
	hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, source);
	if (FAILED(hr)) {
		LogDialogBox("LoadFromWICFile failed : " + filePath);
		return false;
	}
	if (source.GetMetadata().format != DXGI_FORMAT_R8G8B8A8_UNORM) {
		DirectX::ScratchImage converted;
		hr = DirectX::Convert(*source.GetImage(0, 0, 0), DXGI_FORMAT_R8G8B8A8_UNORM, DirectX::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, converted);
		if (FAILED(hr)) {
			LogDialogBox("Convert failed : " + filePath);
			return false;
		}
		source = std::move(converted);
	}

	const DirectX::Image* src = source.GetImage(0, 0, 0);
	const size_t width = src->width;
	const size_t height = src->height;

	DirectX::ScratchImage frameImage;
	DirectX::ScratchImage fillImage;
	if (FAILED(frameImage.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1)) ||
		FAILED(fillImage.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1))) {
		LogDialogBox("ScratchImage::Initialize2D failed");
		return false;
	}
	const DirectX::Image* frame = frameImage.GetImage(0, 0, 0);
	const DirectX::Image* fill = fillImage.GetImage(0, 0, 0);

	// 明るさで「枠」と「背景」に分ける.
	//   枠   : 元の色のまま、明るい所ほど不透明.
	//   背景 : 白、暗い所ほど不透明(描画時に背景色を掛ける).
	for (size_t y = 0; y < height; y++) {
		const uint8_t* srcRow = src->pixels + src->rowPitch * y;
		uint8_t* frameRow = frame->pixels + frame->rowPitch * y;
		uint8_t* fillRow = fill->pixels + fill->rowPitch * y;
		for (size_t x = 0; x < width; x++) {
			const uint8_t r = srcRow[x * 4 + 0];
			const uint8_t g = srcRow[x * 4 + 1];
			const uint8_t b = srcRow[x * 4 + 2];
			const float a = srcRow[x * 4 + 3] / 255.0f;
			const float luminance = (0.299f * r + 0.587f * g + 0.114f * b) / 255.0f;

			frameRow[x * 4 + 0] = r;
			frameRow[x * 4 + 1] = g;
			frameRow[x * 4 + 2] = b;
			frameRow[x * 4 + 3] = static_cast<uint8_t>(std::clamp(a * luminance * 255.0f + 0.5f, 0.0f, 255.0f));

			fillRow[x * 4 + 0] = 255;
			fillRow[x * 4 + 1] = 255;
			fillRow[x * 4 + 2] = 255;
			fillRow[x * 4 + 3] = static_cast<uint8_t>(std::clamp(a * (1.0f - luminance) * 255.0f + 0.5f, 0.0f, 255.0f));
		}
	}

	// 書き出し.
	fs::create_directories(outputDirectory_, ec);
	const GUID& pngCodec = DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG);
	if (FAILED(DirectX::SaveToWICFile(*frame, DirectX::WIC_FLAGS_NONE, pngCodec, Convert::ConvertString(framePath).c_str())) ||
		FAILED(DirectX::SaveToWICFile(*fill, DirectX::WIC_FLAGS_NONE, pngCodec, Convert::ConvertString(fillPath).c_str()))) {
		LogDialogBox("SaveToWICFile failed : " + framePath);
		return false;
	}
	LogDialogBox("テンプレート画像を分割しました : " + filePath);
	return true;
}

/*=============================================================
設定.
=============================================================*/
void DialogBoxManager::SetDialogBox(const std::string& name) {
	assert(HasDialogBox(name) && "DialogBoxManager : 登録されていないダイアログボックスです");
	if (HasDialogBox(name)) {
		currentName_ = name;
	}
}

const DialogBoxManager::DialogBoxData* DialogBoxManager::GetCurrent() const {
	auto it = dialogBoxes_.find(currentName_);
	if (it == dialogBoxes_.end()) {
		return nullptr;
	}
	return &it->second;
}

/*=============================================================
描画.
=============================================================*/
void DialogBoxManager::DrawDialogBox(const Vector2& position, const Vector2& size, const Vector4& backgroundColor) {
	const DialogBoxData* data = GetCurrent();
	assert(data && "DialogBoxManager : LoadDialogBoxでテンプレート画像を読み込んでください");
	if (!data || size.x <= 0.0f || size.y <= 0.0f) {
		return;
	}

	// 背景 → 枠 の順に重ねる.
	if (backgroundColor.w > 0.0f) {
		DrawNineSlice(*data, data->fillTexture, position, size, backgroundColor);
	}
	if (frameColor_.w > 0.0f) {
		DrawNineSlice(*data, data->frameTexture, position, size, frameColor_);
	}
}

void DialogBoxManager::DrawDialogBox(const Vector2& position, const Vector2& size, std::string_view text, float yBlank, const Vector4& textColor, const Vector4& backgroundColor) {
	// 背景.
	DrawDialogBox(position, size, backgroundColor);

	if (text.empty()) {
		return;
	}
	if (!FontManager::GetInstance()->HasFont(fontName_)) {
		assert(false && "DialogBoxManager : SetFontで指定したフォントが読み込まれていません");
		return;
	}

	// 文字(揃えに合わせて基準Xを決める).
	Vector2 textPosition = { position.x + textPadding_.x, position.y + textPadding_.y };
	switch (textAlign_) {
	case TextAlign::kCenter: textPosition.x = position.x + size.x * 0.5f; break;
	case TextAlign::kRight:  textPosition.x = position.x + size.x - textPadding_.x; break;
	default: break;
	}
	FontManager::GetInstance()->DrawString(fontName_, std::string(text), textPosition, fontSize_,yBlank, textColor, textAlign_);
}

void DialogBoxManager::DrawDialogBox(const Vector2& position, const Vector2& size, std::wstring_view text, float yBlank, const Vector4& textColor, const Vector4& backgroundColor) {
	DrawDialogBox(position, size, std::string_view(Convert::ConvertString(std::wstring(text))),yBlank, textColor, backgroundColor);
}

void DialogBoxManager::DrawNineSlice(const DialogBoxData& data, const TextureInfo& texture, const Vector2& position, const Vector2& size, const Vector4& color) {
	const float textureWidth = static_cast<float>(texture.width);
	const float textureHeight = static_cast<float>(texture.height);
	const float cell = static_cast<float>(data.cellSize);

	// 角の大きさ(ボックスが小さい時は半分までに抑える).
	const float cornerSize = cell * frameScale_;
	const float cornerW = (std::min)(cornerSize, size.x * 0.5f);
	const float cornerH = (std::min)(cornerSize, size.y * 0.5f);
	const float middleW = size.x - cornerW * 2.0f;
	const float middleH = size.y - cornerH * 2.0f;

	const float left = position.x;
	const float top = position.y;
	const float right = position.x + size.x - cornerW;
	const float bottom = position.y + size.y - cornerH;

	// 各パーツのUV.
	const float cornerX = cell * kCornerCell_;
	const float edgeX = cell * kEdgeCell_;
	const float centerX = cell * kCenterCell_;

	Vector2 cornerLT, cornerRB;
	MakeUV(cornerX, 0.0f, cornerX + cell, cell, textureWidth, textureHeight, cornerLT, cornerRB);
	// 上の辺 : 角のマスの一番右の列(上に枠がある1列)を横に引き伸ばす.
	Vector2 topLT, topRB;
	MakeUV(cornerX + cell - 1.0f, 0.0f, cornerX + cell, cell, textureWidth, textureHeight, topLT, topRB);
	// 左の辺.
	Vector2 edgeLT, edgeRB;
	MakeUV(edgeX, 0.0f, edgeX + cell, cell, textureWidth, textureHeight, edgeLT, edgeRB);
	// 中央.
	Vector2 centerLT, centerRB;
	MakeUV(centerX, 0.0f, centerX + cell, cell, textureWidth, textureHeight, centerLT, centerRB);

	// 角(UVを反転して4隅を作る).
	DrawPiece(texture, left, top, cornerW, cornerH, color, cornerLT, cornerRB);
	DrawPiece(texture, right, top, cornerW, cornerH, color, { cornerRB.x, cornerLT.y }, { cornerLT.x, cornerRB.y });
	DrawPiece(texture, left, bottom, cornerW, cornerH, color, { cornerLT.x, cornerRB.y }, { cornerRB.x, cornerLT.y });
	DrawPiece(texture, right, bottom, cornerW, cornerH, color, cornerRB, cornerLT);

	// 上下の辺.
	DrawPiece(texture, left + cornerW, top, middleW, cornerH, color, topLT, topRB);
	DrawPiece(texture, left + cornerW, bottom, middleW, cornerH, color, { topLT.x, topRB.y }, { topRB.x, topLT.y });

	// 左右の辺.
	DrawPiece(texture, left, top + cornerH, cornerW, middleH, color, edgeLT, edgeRB);
	DrawPiece(texture, right, top + cornerH, cornerW, middleH, color, { edgeRB.x, edgeLT.y }, { edgeLT.x, edgeRB.y });

	// 中央.
	DrawPiece(texture, left + cornerW, top + cornerH, middleW, middleH, color, centerLT, centerRB);
}
