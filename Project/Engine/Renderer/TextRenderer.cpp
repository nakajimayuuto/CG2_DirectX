#include "TextRenderer.h"
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#pragma comment(lib, "DirectXTex.lib")
#include "Renderer.h"

void TextRenderer::Initialize() {
    // DirectWriteの初期化.
    HRESULT hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(
            dwriteFactory_.GetAddressOf()
            )
    );

    assert(SUCCEEDED(hr));

    // フォントを指定する.
    Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;

     hr = dwriteFactory_->CreateTextFormat(
        L"Yu Gothic", // フォント名
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize_,
        L"",
        &textFormat
    );

    assert(SUCCEEDED(hr));

    // Atlas用のBitmapを作る.
    D2D1_FACTORY_OPTIONS options{};

    Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory;

    hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        d2dFactory.GetAddressOf()
    );

    assert(SUCCEEDED(hr));

    Microsoft::WRL::ComPtr<IWICImagingFactory> wicFactory;

    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wicFactory)
    );

    assert(SUCCEEDED(hr));

    Microsoft::WRL::ComPtr<IWICBitmap> bitmap;

    // WIC Bitmapを作る.
    hr = wicFactory->CreateBitmap(
        1024,
        1024,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapCacheOnLoad,
        &bitmap
    );

    assert(SUCCEEDED(hr));

    // Direct2DからWIC Bitmapへ描画する.
    Microsoft::WRL::ComPtr<ID2D1RenderTarget> renderTarget;

    D2D1_RENDER_TARGET_PROPERTIES properties =
        D2D1::RenderTargetProperties();

    hr = d2dFactory->CreateWicBitmapRenderTarget(
        bitmap.Get(),
        properties,
        &renderTarget
    );

    assert(SUCCEEDED(hr));

    renderTarget->BeginDraw();

    renderTarget->Clear(
        D2D1::ColorF(
            0.0f,
            0.0f,
            0.0f,
            0.0f
        )
    );

    // 文字色.
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;

    hr = renderTarget->CreateSolidColorBrush(
        D2D1::ColorF(
            1.0f,
            1.0f,
            1.0f,
            1.0f
        ),
        &brush
    );

    assert(SUCCEEDED(hr));

    // 各GlyphをAtlasへ配置する.
    int x = 0;
    int y = 0;

    const int padding = 2;
    const int glyphWidth = 32;
    const int glyphHeight = 48;

    for (wchar_t c : characters_)
    {
        // 文字を描画
        D2D1_RECT_F rect =
            D2D1::RectF(
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(x + glyphWidth),
                static_cast<float>(y + glyphHeight)
            );

        std::wstring str(1, c);

        renderTarget->DrawTextW(
            str.c_str(),
            1,
            textFormat.Get(),
            rect,
            brush.Get()
        );

        x += glyphWidth + padding;

       //if (x >= atlasWidth_)
       //{
       //    x = 0;
       //    y += glyphHeight + padding;
       //}
    }
}

void TextRenderer::Draw(const std::wstring& text, const Vector2& position){
    Vector2 cursor = position;

    for (wchar_t c : text)
    {
        auto itr = glyphs_.find(c);

        if (itr == glyphs_.end())
        {
            continue;
        }

        const Glyph& glyph = itr->second;

        // Spriteとして描画
        DrawGlyph(
            glyph,
            cursor
        );

        cursor.x += glyph.advance;
    }
}

void TextRenderer::DrawGlyph(Glyph glyph, Vector2 cursor){
    //Sprite sprite{};
    //
    //sprite.position = position;
    //
    //sprite.size = glyph.size;
    //
    //sprite.uvMin = glyph.uvMin;
    //sprite.uvMax = glyph.uvMax;
    //
    //sprite.texture = atlasTexture_;
    //
    //spriteRenderer_->Draw(sprite);
}
