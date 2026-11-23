#pragma once
#include <string>
#include <string_view>
#include <map>
#include <cstdint>

#include "TextureInfo.h"
#include "FontManager.h"
#include "../Engine/Math/Vector2.h"
#include "../Engine/Math/Vector4.h"

/// <summary>
/// ダイアログボックス管理.
/// テンプレート画像(正方形のマスを横に並べた画像)を9分割(9-slice)して、
/// 角・辺の見た目を崩さずに好きな大きさのダイアログボックスを描画する.
///
/// テンプレート画像のマス(左から 0,1,2,...)の使い方.
///   2 : 左上の角
///   3 : 左の辺
///   5 : 中央(塗りつぶし)
///   ※ 右上/左下/右下の角、右/下の辺はUVを反転して作る.
///   ※ 上の辺は角(マス2)の一番右の列を引き伸ばして作る.
///
/// 色について.
///   テンプレート画像の「明るい部分 = 枠」「暗い部分 = 背景」として読み込み時に2枚に分ける.
///   背景は指定した背景色で塗られ、枠は枠の色(SetFrameColor、既定は白)で描画される.
///   分けた画像は Resources/dialog_box/Generated に書き出され、元画像が更新された時だけ作り直す.
/// </summary>
class DialogBoxManager {
public:
	static DialogBoxManager* GetInstance();

	/// <summary>
	/// ダイアログボックスのテンプレート画像を読み込む.
	/// 最初に読み込んだものが「使用中のテンプレート」になる.
	/// </summary>
	/// <param name="name">登録名</param>
	/// <param name="filePath">テンプレート画像(png)のパス</param>
	/// <param name="cellSize">1マスの大きさ(px)。0なら画像の高さを使う</param>
	void LoadDialogBox(const std::string& name, const std::string& filePath, uint32_t cellSize = 0);

	/// <summary>
	/// 背景のみのダイアログボックスを描画する.
	/// </summary>
	/// <param name="position">左上の座標(スクリーン座標)</param>
	/// <param name="size">大きさ(px)</param>
	/// <param name="backgroundColor">背景の色</param>
	void DrawDialogBox(const Vector2& position, const Vector2& size, const Vector4& backgroundColor = { 0.0f,0.0f,0.0f,1.0f });

	/// <summary>
	/// 文字付きのダイアログボックスを描画する.
	/// </summary>
	/// <param name="position">左上の座標(スクリーン座標)</param>
	/// <param name="size">大きさ(px)</param>
	/// <param name="text">表示する文字列(UTF-8、"\n"で改行)</param>
	/// <param name="textColor">文字の色</param>
	/// <param name="backgroundColor">背景の色</param>
	/// ※ std::string / "文字列" / L"文字列" のどれでも渡せる.
	///   (string_view にしているのは、背景のみ版に {r,g,b,a} を直接書いた時に曖昧にならないようにするため)
	void DrawDialogBox(const Vector2& position, const Vector2& size, std::string_view text, const Vector4& textColor = { 1.0f,1.0f,1.0f,1.0f }, const Vector4& backgroundColor = { 0.0f,0.0f,0.0f,1.0f });
	void DrawDialogBox(const Vector2& position, const Vector2& size, std::wstring_view text, const Vector4& textColor = { 1.0f,1.0f,1.0f,1.0f }, const Vector4& backgroundColor = { 0.0f,0.0f,0.0f,1.0f });

	/*=============================================================
	設定(一度設定すると以降の描画すべてに反映される).
	=============================================================*/

	// 使用するテンプレートを切り替える(LoadDialogBoxで登録した名前).
	void SetDialogBox(const std::string& name);
	// 文字の描画に使うフォント(FontManager::LoadFontで登録した名前)と文字の大きさ(px).
	void SetFont(const std::string& fontName, float fontSize) { fontName_ = fontName; fontSize_ = fontSize; };
	// 文字の大きさ(px).
	void SetFontSize(float fontSize) { fontSize_ = fontSize; };
	// ボックスの外枠から文字までの余白(px).
	void SetTextPadding(const Vector2& padding) { textPadding_ = padding; };
	// 文字の横揃え.
	void SetTextAlign(TextAlign align) { textAlign_ = align; };
	// 枠の色(テンプレートの明るい部分に掛ける色).
	void SetFrameColor(const Vector4& color) { frameColor_ = color; };
	// 枠(角・辺)の拡大率. 1.0でテンプレート画像と同じ太さ.
	void SetFrameScale(float scale) { frameScale_ = scale; };

	bool HasDialogBox(const std::string& name) const { return dialogBoxes_.find(name) != dialogBoxes_.end(); };

private:
	struct DialogBoxData {
		std::string name;
		std::string filePath;
		uint32_t cellSize = 0;
		TextureInfo frameTexture{}; // 枠の部分だけの画像.
		TextureInfo fillTexture{};  // 背景の部分だけの画像(白).
	};

	DialogBoxManager() = default;
	~DialogBoxManager() = default;
	DialogBoxManager(const DialogBoxManager&) = delete;
	DialogBoxManager& operator=(const DialogBoxManager&) = delete;

	// テンプレート画像を枠/背景の2枚に分けて書き出す.
	bool SplitTemplate(const std::string& filePath, const std::string& framePath, const std::string& fillPath, uint32_t& outHeight);

	// 9分割で1枚分(枠 or 背景)を描画する.
	void DrawNineSlice(const DialogBoxData& data, const TextureInfo& texture, const Vector2& position, const Vector2& size, const Vector4& color);

	// 現在のテンプレートを取得(無ければnullptr).
	const DialogBoxData* GetCurrent() const;

private:
	std::map<std::string, DialogBoxData> dialogBoxes_;
	std::string currentName_;

	std::string outputDirectory_ = "Resources/dialog_box/Generated";

	std::string fontName_ = "main";
	float fontSize_ = 18.0f;
	Vector2 textPadding_ = { 16.0f,16.0f };
	TextAlign textAlign_ = TextAlign::kLeft;
	Vector4 frameColor_ = { 1.0f,1.0f,1.0f,1.0f };
	float frameScale_ = 1.0f;

	// テンプレート画像内のマス番号.
	static inline const uint32_t kCornerCell_ = 2;
	static inline const uint32_t kEdgeCell_ = 3;
	static inline const uint32_t kCenterCell_ = 5;
};
