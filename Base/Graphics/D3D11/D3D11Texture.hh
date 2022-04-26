//
// Created on Tuesday 12th October 2021 by e-erdal
//

#pragma once

namespace lr
{
    enum class TextureFormat : u16
    {
        Unidentified,  //! Throw error
        BC1,

        RGBA8,    /// Each channel is u8, packed into normalized u32
        RGBA16,   ///
        RGBA32F,  /// Each channel is float
        BGRA8,
        R24TG8T,           /// R channel is 24 bits, G channel is 8 bits
        R32T,              /// R channel is 32 bits typeless
        R32U,              /// R channel is 32 bits u32
        R32F,              /// R channel is 32 bits float
        DEPTH32F,          /// Depth format, A channel is float
        DEPTH24_STENCIL8,  /// Z-Buffer format, 24 bits for depth, 8 bits for stencil
    };

    constexpr u32 TextureFormatToSize(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::BC1: return sizeof(u8);
            case TextureFormat::BGRA8: return sizeof(u8) * 4;
            case TextureFormat::RGBA8: return sizeof(u8) * 4;
            case TextureFormat::RGBA16: return sizeof(u16) * 4;
            case TextureFormat::RGBA32F: return sizeof(float) * 4;
            case TextureFormat::R24TG8T: return sizeof(u32);
            case TextureFormat::R32T:
            case TextureFormat::R32U: return sizeof(u32);
            case TextureFormat::R32F: return sizeof(float);
            case TextureFormat::DEPTH32F: return sizeof(float);
            case TextureFormat::DEPTH24_STENCIL8: return sizeof(u32);
            default: return 0;
        }
    }

    enum class TextureType : u8
    {
        Default,
        Depth,
        RenderTarget,
        RW
    };

    struct TextureDesc
    {
        TextureType Type = TextureType::Default;
        u32 Filters = 0;
        u32 MipMapLevels = 1;
    };

    struct TextureData
    {
        u32 Width = 0;
        u32 Height = 0;

        TextureFormat Format = TextureFormat::Unidentified;

        u32 DataSize = 0;
        u8 *Data = 0;
    };

    class Texture
    {
    public:
        void Init(TextureDesc *pDesc, TextureData *pData);
        ID3D11Texture2D *&GetHandle();
        void Delete();

        ~Texture();

    public:
        const auto &GetWidth() const
        {
            return m_Width;
        }

        const auto &GetHeight() const
        {
            return m_Height;
        }

        const auto &GetDataSize() const
        {
            return m_DataSize;
        }

        auto GetMipCount()
        {
            return m_TotalMips;
        }

        auto GetCurrentMip()
        {
            return m_UsingMip;
        }

        ID3D11ShaderResourceView *&GetShaderResource()
        {
            return m_pShaderResource;
        }

        ID3D11UnorderedAccessView *&GetUAV()
        {
            return m_pUAV;
        }

        ID3D11RenderTargetView *&GetRenderTarget()
        {
            return m_pRenderTarget;
        }

        ID3D11DepthStencilView *&GetDepthStencil()
        {
            return m_pDepthStencil;
        }

    private:
        u32 m_Width = 0;
        u32 m_Height = 0;
        u32 m_DataSize = 0;
        u32 m_Filters = 0;

        u32 m_UsingMip = 0;
        u32 m_TotalMips = 1;

        TextureFormat m_Format;

        ID3D11Texture2D *m_pHandle = nullptr;

        ID3D11ShaderResourceView *m_pShaderResource = nullptr;
        ID3D11UnorderedAccessView *m_pUAV = nullptr;
        ID3D11RenderTargetView *m_pRenderTarget = nullptr;
        ID3D11DepthStencilView *m_pDepthStencil = nullptr;
    };

}  // namespace lr