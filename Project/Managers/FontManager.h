#pragma once
#include <string>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <cstdint>

#include "TextureInfo.h"
#include "../Engine/Math/Vector2.h"
#include "../Engine/Math/Vector4.h"

// Release以外(Debug / Development)ではフォントファイルからアトラス画像を自動生成する.
// Releaseでは生成済みの画像(png)と情報(json)を読み込むだけ.
#if !defined(NDEBUG)
#define FONT_MANAGER_ENABLE_BAKE
#endif

/// <summary>
/// 文字揃え(行ごと).
/// </summary>
enum class TextAlign {
	kLeft,
	kCenter,
	kRight,
};

/// <summary>
/// 1文字分の情報(すべてベイク時のピクセル単位).
/// </summary>
struct FontGlyph {
	Vector2 uvLeftTop;     // アトラス上のUV(左上).
	Vector2 uvRightBottom; // アトラス上のUV(右下).
	Vector2 size;          // グリフ画像の大きさ.
	Vector2 offset;        // ペン位置(ベースライン)からグリフ画像左上へのオフセット.
	float advance = 0.0f;  // 次の文字までの送り幅.
};

/// <summary>
/// フォント管理.
/// Debug/Development : ソースコード中の文字列リテラルで使われている文字を集め、
///                     フォントファイル(.ttf/.otf/.ttc)から文字画像(アトラス)を書き出す.
/// Release           : 書き出し済みのアトラス画像を読み込んで使う(フォントファイルは不要).
/// </summary>
class FontManager {
public:
	static FontManager* GetInstance();

	/// <summary>
	/// フォントを読み込む.
	/// Debug系ではベイク(必要な時だけ)→読み込み、Releaseでは生成済みファイルを読み込む.
	/// </summary>
	/// <param name="name">登録名(出力ファイル名にも使う)</param>
	/// <param name="fontFilePath">フォントファイルのパス(Debug系でのみ使用)</param>
	/// <param name="pixelHeight">ベイクする文字の高さ(px)</param>
	/// <param name="faceIndex">.ttcの場合のフォント番号</param>
	void LoadFont(const std::string& name, const std::string& fontFilePath, float pixelHeight = 48.0f, int faceIndex = 0);

	/// <summary>
	/// 文字列を描画する.
	/// </summary>
	/// <param name="fontName">LoadFontで登録した名前</param>
	/// <param name="text">UTF-8の文字列("\n"で改行)</param>
	/// <param name="position">文字列の左上(揃えがCenter/Rightの時はその基準X)</param>
	/// <param name="fontSize">描画する文字の高さ(px)</param>
	/// <param name="color">色</param>
	/// <param name="align">行ごとの揃え</param>
	void DrawString(const std::string& fontName, const std::string& text, const Vector2& position, float fontSize, float yBlank, const Vector4& color = { 1.0f,1.0f,1.0f,1.0f }, TextAlign align = TextAlign::kLeft);
	void DrawString(const std::string& fontName, const std::wstring& text, const Vector2& position, float fontSize, float yBlank, const Vector4& color = { 1.0f,1.0f,1.0f,1.0f }, TextAlign align = TextAlign::kLeft);
	void DrawString(const std::string& fontName, const std::string& text, const Vector2& position, float fontSize, const Vector4& color = { 1.0f,1.0f,1.0f,1.0f }, TextAlign align = TextAlign::kLeft) {DrawString(fontName,text,position,fontSize,0.0f,color,align);};
	void DrawString(const std::string& fontName, const std::wstring& text, const Vector2& position, float fontSize, const Vector4& color = { 1.0f,1.0f,1.0f,1.0f }, TextAlign align = TextAlign::kLeft) {DrawString(fontName, text, position, fontSize, 0.0f, color, align);};

	/// <summary>
	/// 文字列を描画した時の大きさを返す.
	/// </summary>
	Vector2 MeasureText(const std::string& fontName, const std::string& text, float fontSize);

	/// <summary>
	/// アトラスに必ず含める文字を追加する(外部データから読む文字列など).
	/// LoadFontより前に呼ぶこと。Releaseでは何もしない.
	/// </summary>
	void AddCharacters(const std::string& utf8Text);

	/// <summary>
	/// 文字列リテラルを探すフォルダ/拡張子を変更する(LoadFontより前に呼ぶ).
	/// 既定 : Application, Engine, Managers 以下の .cpp / .h / .hpp
	/// </summary>
	void SetSourceScanSettings(const std::vector<std::string>& directories, const std::vector<std::string>& extensions);

	/// <summary>
	/// 生成物の出力先フォルダ(既定 : Resources/Fonts/Generated).
	/// </summary>
	void SetOutputDirectory(const std::string& directory) { outputDirectory_ = directory; };

	bool HasFont(const std::string& name) const { return fonts_.find(name) != fonts_.end(); };

	// UTF-8 → UTF-32(不正なバイトは読み飛ばす).
	static std::u32string DecodeUtf8(const std::string& text);
	// UTF-32 → UTF-8.
	static std::string EncodeUtf8(const std::u32string& text);

private:
	struct FontData {
		std::string name;
		std::string fontFilePath;
		int faceIndex = 0;
		float pixelHeight = 48.0f;

		float ascent = 0.0f;
		float descent = 0.0f;
		float lineGap = 0.0f;
		float lineHeight = 0.0f;

		std::unordered_map<char32_t, FontGlyph> glyphs;
		TextureInfo textureInfo{};

		// ベイクを要求した文字一式(フォントに無い文字も含む).
		std::set<char32_t> requestedCharacters;
		// 実行中に見つかった文字(次回以降のベイクにも引き継ぐ).
		std::set<char32_t> extraCharacters;

		uint32_t textureRevision = 0;
	};

	FontManager() = default;
	~FontManager() = default;
	FontManager(const FontManager&) = delete;
	FontManager& operator=(const FontManager&) = delete;

	std::string GetPngPath(const std::string& name) const;
	std::string GetJsonPath(const std::string& name) const;

	// jsonと画像を読み込んでFontDataを作る.
	bool LoadGenerated(FontData& font);

#ifdef FONT_MANAGER_ENABLE_BAKE
	// ソースコードを走査して文字列リテラル中の文字を集める.
	void ScanSourceCharacters();
	static void ExtractStringLiterals(const std::string& source, std::set<char32_t>& out);

	// アトラスを生成して png / json を書き出す.
	bool Bake(FontData& font);

	// 既存のjsonが今回の設定・文字一式と一致するか.
	bool IsUpToDate(const FontData& font) const;

	// 描画しようとした文字のうちアトラスに無いものを追加してベイクし直す.
	void EnsureCharacters(FontData& font, const std::u32string& text);
#endif

private:
	std::map<std::string, FontData> fonts_;

	std::string outputDirectory_ = "Resources/Fonts/Generated";

	std::vector<std::string> scanDirectories_ = { "Application", "Engine", "Managers" };
	std::vector<std::string> scanExtensions_ = { ".cpp", ".h", ".hpp" };

	std::set<char32_t> scannedCharacters_;
	std::set<char32_t> additionalCharacters_;
	bool isScanned_ = false;
};
