#pragma once
#include "../Math/Vector2.h"
#include "../SystemFile/GameSystem.h"
#include <dwrite.h>
#include <d2d1.h>
#include <wincodec.h>
#include <wrl.h>

struct Glyph
{
    Vector2 uvMin;
    Vector2 uvMax;

    Vector2 size;

    Vector2 bearing;

    float advance;
};


class TextRenderer
{
public:

    void Initialize();

    void Draw(
        const std::wstring& text,
        const Vector2& position
    );

private:

    void CreateNewFont();
    void CreateGlyphAtlas();
    void CreateTexture();

    void DrawGlyph(Glyph glyph,Vector2 cursor);

private:

    Microsoft::WRL::ComPtr<IDWriteFactory> dwriteFactory_;

    Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory_;

    std::unordered_map<wchar_t, Glyph> glyphs_;

    Microsoft::WRL::ComPtr<ID3D12Resource> atlasTexture_;

    uint32_t atlasWidth_ = 1024;
    uint32_t atlasHeight_ = 1024;

    float fontSize_ = 32.0f;

    const std::wstring characters_ =
        L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        L"abcdefghijklmnopqrstuvwxyz"
        L"0123456789"
        L"!?.,:;+-*/()[]{}"
        L"あいうえおかきくけこさしすせそたちつてとなにぬねの"
        L"はひふへほまみむめもやゆよらりるれろわをんゐゑ";
};