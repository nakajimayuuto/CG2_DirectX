#include "FontManager.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>

#include "../externals/nlohmann/json.hpp"
#include "../Engine/Renderer/Renderer.h"
#include "TextureManager.h"

#ifdef FONT_MANAGER_ENABLE_BAKE
// フォントファイルのラスタライズにはimguiに同梱されているstb_truetypeを使う.
// (imgui_draw.cpp側の実装はstaticなので、こちらでも別途staticで実装を持つ)
#pragma warning(push, 0)
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "../externals/imgui/imstb_truetype.h"
#pragma warning(pop)
#endif

namespace {
	// 生成ファイルのフォーマットバージョン(中身を変えたら上げる).
	constexpr int kFormatVersion = 1;

	// グリフ間の余白(mipmapでのにじみ対策).
	constexpr int kGlyphPadding = 4;

	void LogFont(const std::string& message) {
		OutputDebugStringA(("[FontManager] " + message + "\n").c_str());
	}
}

FontManager* FontManager::GetInstance() {
	static FontManager instance;
	return &instance;
}

/*=============================================================
UTF-8 変換.
=============================================================*/
std::u32string FontManager::DecodeUtf8(const std::string& text) {
	std::u32string result;
	result.reserve(text.size());

	size_t i = 0;
	while (i < text.size()) {
		uint8_t c = static_cast<uint8_t>(text[i]);
		char32_t codePoint = 0;
		size_t length = 0;

		if (c < 0x80) {
			codePoint = c;
			length = 1;
		} else if ((c & 0xE0) == 0xC0) {
			codePoint = c & 0x1F;
			length = 2;
		} else if ((c & 0xF0) == 0xE0) {
			codePoint = c & 0x0F;
			length = 3;
		} else if ((c & 0xF8) == 0xF0) {
			codePoint = c & 0x07;
			length = 4;
		} else {
			// 不正な先頭バイト.
			i++;
			continue;
		}

		if (i + length > text.size()) {
			break;
		}

		bool isValid = true;
		for (size_t j = 1; j < length; j++) {
			uint8_t next = static_cast<uint8_t>(text[i + j]);
			if ((next & 0xC0) != 0x80) {
				isValid = false;
				break;
			}
			codePoint = (codePoint << 6) | (next & 0x3F);
		}

		if (!isValid) {
			i++;
			continue;
		}

		result.push_back(codePoint);
		i += length;
	}
	return result;
}

std::string FontManager::EncodeUtf8(const std::u32string& text) {
	std::string result;
	for (char32_t c : text) {
		if (c < 0x80) {
			result.push_back(static_cast<char>(c));
		} else if (c < 0x800) {
			result.push_back(static_cast<char>(0xC0 | (c >> 6)));
			result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
		} else if (c < 0x10000) {
			result.push_back(static_cast<char>(0xE0 | (c >> 12)));
			result.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
			result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
		} else {
			result.push_back(static_cast<char>(0xF0 | (c >> 18)));
			result.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F)));
			result.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
			result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
		}
	}
	return result;
}

/*=============================================================
設定.
=============================================================*/
void FontManager::AddCharacters(const std::string& utf8Text) {
#ifdef FONT_MANAGER_ENABLE_BAKE
	for (char32_t c : DecodeUtf8(utf8Text)) {
		additionalCharacters_.insert(c);
	}
#else
	(void)utf8Text;
#endif
}

void FontManager::SetSourceScanSettings(const std::vector<std::string>& directories, const std::vector<std::string>& extensions) {
	scanDirectories_ = directories;
	scanExtensions_ = extensions;
	isScanned_ = false;
	scannedCharacters_.clear();
}

std::string FontManager::GetPngPath(const std::string& name) const {
	return outputDirectory_ + "/" + name + ".png";
}

std::string FontManager::GetJsonPath(const std::string& name) const {
	return outputDirectory_ + "/" + name + ".json";
}

/*=============================================================
読み込み.
=============================================================*/
void FontManager::LoadFont(const std::string& name, const std::string& fontFilePath, float pixelHeight, int faceIndex) {
	if (HasFont(name)) {
		return;
	}

	FontData font;
	font.name = name;
	font.fontFilePath = fontFilePath;
	font.faceIndex = faceIndex;
	font.pixelHeight = pixelHeight;

#ifdef FONT_MANAGER_ENABLE_BAKE
	ScanSourceCharacters();

	// 前回の実行中に見つかった文字を引き継ぐ.
	{
		std::ifstream file(GetJsonPath(name));
		if (file.is_open()) {
			try {
				nlohmann::json json = nlohmann::json::parse(file);
				if (json.contains("extraCharacters")) {
					for (char32_t c : DecodeUtf8(json["extraCharacters"].get<std::string>())) {
						font.extraCharacters.insert(c);
					}
				}
			} catch (const std::exception& e) {
				LogFont(std::string("json parse error : ") + e.what());
			}
		}
	}

	// 表示可能なASCII(数字の表示などで動的に使われやすいので常に入れる).
	for (char32_t c = 0x20; c < 0x7F; c++) {
		font.requestedCharacters.insert(c);
	}
	font.requestedCharacters.insert(scannedCharacters_.begin(), scannedCharacters_.end());
	font.requestedCharacters.insert(additionalCharacters_.begin(), additionalCharacters_.end());
	font.requestedCharacters.insert(font.extraCharacters.begin(), font.extraCharacters.end());

	if (IsUpToDate(font)) {
		LogFont("\"" + name + "\" is up to date. skip bake.");
	} else {
		bool isBaked = Bake(font);
		assert(isBaked && "FontManager : フォントのベイクに失敗しました(フォントファイルのパスを確認してください)");
		(void)isBaked;
	}
#endif

	bool isLoaded = LoadGenerated(font);
	assert(isLoaded && "FontManager : 生成済みフォントの読み込みに失敗しました(Debugで一度実行して生成してください)");
	if (!isLoaded) {
		return;
	}

	fonts_[name] = std::move(font);
}

bool FontManager::LoadGenerated(FontData& font) {
	std::ifstream file(GetJsonPath(font.name));
	if (!file.is_open()) {
		LogFont("not found : " + GetJsonPath(font.name));
		return false;
	}

	nlohmann::json json;
	try {
		json = nlohmann::json::parse(file);
	} catch (const std::exception& e) {
		LogFont(std::string("json parse error : ") + e.what());
		return false;
	}

	font.pixelHeight = json["pixelHeight"].get<float>();
	font.ascent = json["ascent"].get<float>();
	font.descent = json["descent"].get<float>();
	font.lineGap = json["lineGap"].get<float>();
	font.lineHeight = json["lineHeight"].get<float>();

	float atlasWidth = json["atlasWidth"].get<float>();
	float atlasHeight = json["atlasHeight"].get<float>();

	font.glyphs.clear();
	// [codePoint, x, y, w, h, offsetX, offsetY, advance].
	for (const auto& g : json["glyphs"]) {
		char32_t codePoint = static_cast<char32_t>(g[0].get<uint32_t>());
		float x = g[1].get<float>();
		float y = g[2].get<float>();
		float w = g[3].get<float>();
		float h = g[4].get<float>();

		FontGlyph glyph;
		glyph.uvLeftTop = { x / atlasWidth, y / atlasHeight };
		glyph.uvRightBottom = { (x + w) / atlasWidth, (y + h) / atlasHeight };
		glyph.size = { w, h };
		glyph.offset = { g[5].get<float>(), g[6].get<float>() };
		glyph.advance = g[7].get<float>();
		font.glyphs[codePoint] = glyph;
	}

	// テクスチャの登録(Debugで作り直した時は別名で登録し直す).
	std::string textureName = "font_" + font.name;
	if (font.textureRevision > 0) {
		textureName += "_" + std::to_string(font.textureRevision);
	}
	if (!TextureManager::GetInstance()->HasTexture(textureName)) {
		TextureManager::GetInstance()->RegisterTexture(textureName, GetPngPath(font.name), false);
	}
	font.textureInfo = TextureManager::GetInstance()->GetTextureInfo(textureName);
	return true;
}

/*=============================================================
描画.
=============================================================*/
void FontManager::DrawString(const std::string& fontName, const std::string& text, const Vector2& position, float fontSize,float yBlank, const Vector4& color, TextAlign align) {
	auto it = fonts_.find(fontName);
	assert(it != fonts_.end() && "FontManager : 登録されていないフォントです");
	if (it == fonts_.end()) {
		return;
	}
	FontData& font = it->second;

	std::u32string codePoints = DecodeUtf8(text);

#ifdef FONT_MANAGER_ENABLE_BAKE
	EnsureCharacters(font, codePoints);
#endif

	const float scale = fontSize / font.pixelHeight;
	const float spaceAdvance = font.pixelHeight * 0.5f;

	// 行ごとの幅を先に求める(揃え用).
	auto measureLine = [&](size_t begin) {
		float width = 0.0f;
		for (size_t i = begin; i < codePoints.size() && codePoints[i] != U'\n'; i++) {
			auto glyphIt = font.glyphs.find(codePoints[i]);
			width += (glyphIt != font.glyphs.end()) ? glyphIt->second.advance : spaceAdvance;
		}
		return width * scale;
	};

	auto lineStartX = [&](size_t begin) {
		switch (align) {
		case TextAlign::kCenter: return position.x - measureLine(begin) * 0.5f;
		case TextAlign::kRight:  return position.x - measureLine(begin);
		default:                 return position.x;
		}
	};

	float penX = lineStartX(0);
	float baselineY = position.y + font.ascent * scale;

	for (size_t i = 0; i < codePoints.size(); i++) {
		char32_t c = codePoints[i];

		if (c == U'\n') {
			penX = lineStartX(i + 1);
			baselineY += (font.lineHeight * scale) + yBlank;
			continue;
		}
		if (c == U'\r') {
			continue;
		}

		auto glyphIt = font.glyphs.find(c);
		if (glyphIt == font.glyphs.end()) {
			// アトラスに無い文字は空白扱い.
			penX += spaceAdvance * scale;
			continue;
		}
		const FontGlyph& glyph = glyphIt->second;

		if (glyph.size.x > 0.0f && glyph.size.y > 0.0f) {
			Vector2 size = { glyph.size.x * scale, glyph.size.y * scale };
			float left = penX + glyph.offset.x * scale;
			float top = baselineY + glyph.offset.y * scale;

			// スプライトは中心が原点なので中心座標に変換する.
			Transform transform = Transform::GetInitialValue(
				{ 1.0f,1.0f,1.0f },
				{ 0.0f,0.0f,0.0f },
				{ left + size.x * 0.5f, top + size.y * 0.5f, 0.0f });

			Renderer::GetInstance()->DrawSpriteRect(transform, size, font.textureInfo, color, glyph.uvLeftTop, glyph.uvRightBottom);
		}

		penX += glyph.advance * scale;
	}
}

void FontManager::DrawString(const std::string& fontName, const std::wstring& text, const Vector2& position, float fontSize, float yBlank, const Vector4& color, TextAlign align) {
	DrawString(fontName, Convert::ConvertString(text), position, fontSize,yBlank, color, align);
}

Vector2 FontManager::MeasureText(const std::string& fontName, const std::string& text, float fontSize) {
	auto it = fonts_.find(fontName);
	assert(it != fonts_.end() && "FontManager : 登録されていないフォントです");
	if (it == fonts_.end()) {
		return { 0.0f,0.0f };
	}
	const FontData& font = it->second;

	const float scale = fontSize / font.pixelHeight;
	const float spaceAdvance = font.pixelHeight * 0.5f;

	float maxWidth = 0.0f;
	float lineWidth = 0.0f;
	int lineCount = 1;
	for (char32_t c : DecodeUtf8(text)) {
		if (c == U'\n') {
			maxWidth = (std::max)(maxWidth, lineWidth);
			lineWidth = 0.0f;
			lineCount++;
			continue;
		}
		auto glyphIt = font.glyphs.find(c);
		lineWidth += (glyphIt != font.glyphs.end()) ? glyphIt->second.advance : spaceAdvance;
	}
	maxWidth = (std::max)(maxWidth, lineWidth);

	float height = (font.ascent - font.descent) + font.lineHeight * static_cast<float>(lineCount - 1);
	return { maxWidth * scale, height * scale };
}

#ifdef FONT_MANAGER_ENABLE_BAKE
/*=============================================================
ソースコードの走査(Debug系のみ).
=============================================================*/
void FontManager::ScanSourceCharacters() {
	if (isScanned_) {
		return;
	}
	isScanned_ = true;

	namespace fs = std::filesystem;
	size_t fileCount = 0;

	for (const std::string& directory : scanDirectories_) {
		std::error_code ec;
		if (!fs::exists(directory, ec)) {
			LogFont("scan directory not found : " + directory);
			continue;
		}

		for (const auto& entry : fs::recursive_directory_iterator(directory, fs::directory_options::skip_permission_denied, ec)) {
			if (!entry.is_regular_file()) {
				continue;
			}
			std::string extension = entry.path().extension().string();
			std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
			if (std::find(scanExtensions_.begin(), scanExtensions_.end(), extension) == scanExtensions_.end()) {
				continue;
			}

			std::ifstream file(entry.path(), std::ios::binary);
			if (!file.is_open()) {
				continue;
			}
			std::stringstream buffer;
			buffer << file.rdbuf();
			ExtractStringLiterals(buffer.str(), scannedCharacters_);
			fileCount++;
		}
	}

	LogFont("scanned " + std::to_string(fileCount) + " files, " + std::to_string(scannedCharacters_.size()) + " characters.");
}

void FontManager::ExtractStringLiterals(const std::string& source, std::set<char32_t>& out) {
	const size_t size = source.size();

	auto addLiteral = [&](const std::string& literal) {
		for (char32_t c : DecodeUtf8(literal)) {
			// 制御文字は除外.
			if (c >= 0x20 && c != 0x7F) {
				out.insert(c);
			}
		}
	};

	auto isIdentifierChar = [](char ch) {
		return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
	};

	size_t i = 0;
	while (i < size) {
		char c = source[i];

		// 行コメント.
		if (c == '/' && i + 1 < size && source[i + 1] == '/') {
			while (i < size && source[i] != '\n') {
				i++;
			}
			continue;
		}
		// ブロックコメント.
		if (c == '/' && i + 1 < size && source[i + 1] == '*') {
			size_t end = source.find("*/", i + 2);
			i = (end == std::string::npos) ? size : end + 2;
			continue;
		}
		// 文字リテラル(数値の桁区切り 1'000 は除外).
		if (c == '\'') {
			if (i > 0 && std::isxdigit(static_cast<unsigned char>(source[i - 1]))) {
				i++;
				continue;
			}
			i++;
			while (i < size && source[i] != '\'' && source[i] != '\n') {
				if (source[i] == '\\') {
					i++;
				}
				i++;
			}
			i++;
			continue;
		}
		// 文字列リテラル.
		if (c == '"') {
			// 生文字列リテラル R"delim( ... )delim".
			if (i > 0 && source[i - 1] == 'R' && (i < 2 || !isIdentifierChar(source[i - 2]) || source[i - 2] == 'L' || source[i - 2] == '8' || source[i - 2] == 'u' || source[i - 2] == 'U')) {
				size_t open = source.find('(', i + 1);
				if (open != std::string::npos) {
					std::string delimiter = ")" + source.substr(i + 1, open - (i + 1)) + "\"";
					size_t close = source.find(delimiter, open + 1);
					if (close != std::string::npos) {
						addLiteral(source.substr(open + 1, close - (open + 1)));
						i = close + delimiter.size();
						continue;
					}
				}
			}

			std::string literal;
			i++;
			while (i < size && source[i] != '"' && source[i] != '\n') {
				if (source[i] == '\\' && i + 1 < size) {
					char escape = source[i + 1];
					if (escape == 'u' || escape == 'U') {
						// \uXXXX / \UXXXXXXXX.
						size_t digits = (escape == 'u') ? 4 : 8;
						if (i + 2 + digits <= size) {
							char32_t codePoint = static_cast<char32_t>(std::strtoul(source.substr(i + 2, digits).c_str(), nullptr, 16));
							literal += EncodeUtf8(std::u32string(1, codePoint));
						}
						i += 2 + digits;
						continue;
					}
					if (escape == '"' || escape == '\\' || escape == '\'' || escape == '?') {
						literal.push_back(escape);
					}
					// \n や \t などの制御文字は無視.
					i += 2;
					continue;
				}
				literal.push_back(source[i]);
				i++;
			}
			i++;
			addLiteral(literal);
			continue;
		}

		i++;
	}
}

/*=============================================================
ベイク(Debug系のみ).
=============================================================*/
bool FontManager::IsUpToDate(const FontData& font) const {
	namespace fs = std::filesystem;
	if (!fs::exists(GetPngPath(font.name))) {
		return false;
	}

	std::ifstream file(GetJsonPath(font.name));
	if (!file.is_open()) {
		return false;
	}

	try {
		nlohmann::json json = nlohmann::json::parse(file);
		std::u32string requested(font.requestedCharacters.begin(), font.requestedCharacters.end());

		return json.value("version", 0) == kFormatVersion &&
			json.value("fontFile", std::string()) == font.fontFilePath &&
			json.value("faceIndex", -1) == font.faceIndex &&
			json.value("pixelHeight", 0.0f) == font.pixelHeight &&
			json.value("characters", std::string()) == EncodeUtf8(requested);
	} catch (...) {
		return false;
	}
}

bool FontManager::Bake(FontData& font) {
	// フォントファイルを読む.
	std::ifstream file(font.fontFilePath, std::ios::binary);
	if (!file.is_open()) {
		LogFont("font file not found : " + font.fontFilePath);
		return false;
	}
	std::vector<unsigned char> fontBuffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	int fontOffset = stbtt_GetFontOffsetForIndex(fontBuffer.data(), font.faceIndex);
	if (fontOffset < 0) {
		LogFont("invalid face index : " + std::to_string(font.faceIndex));
		return false;
	}

	stbtt_fontinfo info;
	if (!stbtt_InitFont(&info, fontBuffer.data(), fontOffset)) {
		LogFont("stbtt_InitFont failed : " + font.fontFilePath);
		return false;
	}

	const float scale = stbtt_ScaleForPixelHeight(&info, font.pixelHeight);

	int ascent = 0, descent = 0, lineGap = 0;
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

	// 各グリフの大きさを調べる.
	struct BakeGlyph {
		char32_t codePoint;
		int glyphIndex;
		int x0, y0, x1, y1;
		float advance;
		int atlasX = 0;
		int atlasY = 0;
	};
	std::vector<BakeGlyph> bakeGlyphs;
	std::u32string missing;

	for (char32_t c : font.requestedCharacters) {
		int glyphIndex = stbtt_FindGlyphIndex(&info, static_cast<int>(c));
		if (glyphIndex == 0 && c != U' ') {
			missing.push_back(c);
			continue;
		}
		int advance = 0, leftSideBearing = 0;
		stbtt_GetGlyphHMetrics(&info, glyphIndex, &advance, &leftSideBearing);

		BakeGlyph glyph{};
		glyph.codePoint = c;
		glyph.glyphIndex = glyphIndex;
		stbtt_GetGlyphBitmapBox(&info, glyphIndex, scale, scale, &glyph.x0, &glyph.y0, &glyph.x1, &glyph.y1);
		glyph.advance = static_cast<float>(advance) * scale;
		bakeGlyphs.push_back(glyph);
	}

	if (!missing.empty()) {
		LogFont("glyphs not found in font : " + EncodeUtf8(missing));
	}

	// 棚詰め(高い順に並べて左から詰める).
	std::vector<size_t> order(bakeGlyphs.size());
	for (size_t i = 0; i < order.size(); i++) {
		order[i] = i;
	}
	std::sort(order.begin(), order.end(), [&](size_t a, size_t b) {
		return (bakeGlyphs[a].y1 - bakeGlyphs[a].y0) > (bakeGlyphs[b].y1 - bakeGlyphs[b].y0);
	});

	size_t totalArea = 0;
	for (const auto& g : bakeGlyphs) {
		totalArea += static_cast<size_t>(g.x1 - g.x0 + kGlyphPadding) * static_cast<size_t>(g.y1 - g.y0 + kGlyphPadding);
	}
	int atlasWidth = 256;
	while (static_cast<size_t>(atlasWidth) * static_cast<size_t>(atlasWidth) < totalArea * 2 && atlasWidth < 8192) {
		atlasWidth *= 2;
	}

	int penX = kGlyphPadding;
	int penY = kGlyphPadding;
	int rowHeight = 0;
	for (size_t index : order) {
		BakeGlyph& g = bakeGlyphs[index];
		int w = g.x1 - g.x0;
		int h = g.y1 - g.y0;
		if (penX + w + kGlyphPadding > atlasWidth) {
			penX = kGlyphPadding;
			penY += rowHeight + kGlyphPadding;
			rowHeight = 0;
		}
		g.atlasX = penX;
		g.atlasY = penY;
		penX += w + kGlyphPadding;
		rowHeight = (std::max)(rowHeight, h);
	}
	int atlasHeight = penY + rowHeight + kGlyphPadding;
	atlasHeight = (atlasHeight + 3) & ~3;

	// ラスタライズ(色は白、文字の形はアルファで表す).
	DirectX::ScratchImage image;
	HRESULT hr = image.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, atlasWidth, atlasHeight, 1, 1);
	if (FAILED(hr)) {
		LogFont("ScratchImage::Initialize2D failed");
		return false;
	}
	const DirectX::Image* atlas = image.GetImage(0, 0, 0);
	for (int y = 0; y < atlasHeight; y++) {
		uint8_t* row = atlas->pixels + atlas->rowPitch * y;
		for (int x = 0; x < atlasWidth; x++) {
			row[x * 4 + 0] = 255;
			row[x * 4 + 1] = 255;
			row[x * 4 + 2] = 255;
			row[x * 4 + 3] = 0;
		}
	}

	std::vector<unsigned char> glyphBitmap;
	for (const BakeGlyph& g : bakeGlyphs) {
		int w = g.x1 - g.x0;
		int h = g.y1 - g.y0;
		if (w <= 0 || h <= 0) {
			continue;
		}
		glyphBitmap.assign(static_cast<size_t>(w) * h, 0);
		stbtt_MakeGlyphBitmap(&info, glyphBitmap.data(), w, h, w, scale, scale, g.glyphIndex);
		for (int y = 0; y < h; y++) {
			uint8_t* row = atlas->pixels + atlas->rowPitch * (g.atlasY + y);
			for (int x = 0; x < w; x++) {
				row[(g.atlasX + x) * 4 + 3] = glyphBitmap[static_cast<size_t>(y) * w + x];
			}
		}
	}

	// 書き出し.
	std::error_code ec;
	std::filesystem::create_directories(outputDirectory_, ec);

	std::wstring pngPath = Convert::ConvertString(GetPngPath(font.name));
	hr = DirectX::SaveToWICFile(*atlas, DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG), pngPath.c_str());
	if (FAILED(hr)) {
		LogFont("SaveToWICFile failed : " + GetPngPath(font.name));
		return false;
	}

	nlohmann::json json;
	json["version"] = kFormatVersion;
	json["fontFile"] = font.fontFilePath;
	json["faceIndex"] = font.faceIndex;
	json["pixelHeight"] = font.pixelHeight;
	json["ascent"] = static_cast<float>(ascent) * scale;
	json["descent"] = static_cast<float>(descent) * scale;
	json["lineGap"] = static_cast<float>(lineGap) * scale;
	json["lineHeight"] = static_cast<float>(ascent - descent + lineGap) * scale;
	json["atlasWidth"] = atlasWidth;
	json["atlasHeight"] = atlasHeight;
	json["characters"] = EncodeUtf8(std::u32string(font.requestedCharacters.begin(), font.requestedCharacters.end()));
	json["extraCharacters"] = EncodeUtf8(std::u32string(font.extraCharacters.begin(), font.extraCharacters.end()));

	nlohmann::json glyphs = nlohmann::json::array();
	for (const BakeGlyph& g : bakeGlyphs) {
		glyphs.push_back({
			static_cast<uint32_t>(g.codePoint),
			g.atlasX, g.atlasY, g.x1 - g.x0, g.y1 - g.y0,
			g.x0, g.y0,
			g.advance });
	}
	json["glyphs"] = glyphs;

	std::ofstream out(GetJsonPath(font.name));
	if (!out.is_open()) {
		LogFont("cannot write : " + GetJsonPath(font.name));
		return false;
	}
	out << json.dump(1);

	LogFont("baked \"" + font.name + "\" : " + std::to_string(bakeGlyphs.size()) + " glyphs, " + std::to_string(atlasWidth) + "x" + std::to_string(atlasHeight));
	return true;
}

void FontManager::EnsureCharacters(FontData& font, const std::u32string& text) {
	bool hasNew = false;
	for (char32_t c : text) {
		if (c < 0x20 || c == 0x7F) {
			continue;
		}
		// 一度要求した文字(フォントに無かった文字も含む)はもう作り直さない.
		if (font.requestedCharacters.insert(c).second) {
			font.extraCharacters.insert(c);
			hasNew = true;
		}
	}
	if (!hasNew) {
		return;
	}

	LogFont("new characters found at runtime. rebake \"" + font.name + "\".");
	if (Bake(font)) {
		font.textureRevision++;
		LoadGenerated(font);
	}
}
#endif
