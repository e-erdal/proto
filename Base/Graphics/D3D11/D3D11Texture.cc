#include "D3D11Texture.hh"

#include "Core/BaseApp.hh"

namespace lr
{
    void Texture::Init(TextureDesc *pDesc, TextureData *pData)
    {
        if (!pDesc && !pData) return;

        m_TotalMips = pDesc->MipMapLevels;
        m_Format = pData->Format;
        m_Width = pData->Width;
        m_Height = pData->Height;
        m_DataSize = pData->DataSize;

        LOG_TRACE("Creating Texture2D({}, {})...", m_Width, m_Height);

        D3D11API *pAPI = GetApp()->GetAPI();

        pAPI->CreateTexture2D(this, pDesc->Type, pData);

        switch (pDesc->Type)
        {
            case TextureType::RenderTarget: pAPI->CreateRenderTargetView(this, pData->Format); break;
            case TextureType::Depth:
                pAPI->CreateDepthStencilView(this);
                if (m_Format == TextureFormat::R32T) m_Format = TextureFormat::R32F;
                break;
            case TextureType::RW: pAPI->CreateUnorderedAccessView(this); break;

            default: break;
        }

        pAPI->CreateShaderResource(this, m_Format, m_TotalMips);

        LOG_TRACE("Created a Texture2D({}, {})!", m_Width, m_Height);
    }

    ID3D11Texture2D *&Texture::GetHandle()
    {
        return m_pHandle;
    }

    void Texture::Delete()
    {
        SAFE_RELEASE(m_pHandle);

        SAFE_RELEASE(m_pShaderResource);

        SAFE_RELEASE(m_pUAV);
        SAFE_RELEASE(m_pRenderTarget);

        SAFE_RELEASE(m_pDepthStencil);
    }

    Texture::~Texture()
    {
        Delete();
    }

}  // namespace lr