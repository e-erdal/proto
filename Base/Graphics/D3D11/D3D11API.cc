#include "D3D11API.hh"

#include "D3D11RenderBuffer.hh"
#include "D3D11SwapChain.hh"

#include "Core/Window.hh"

#include "Graphics/D3D.hh"

static ID3D11ShaderResourceView *kNullSRV[1] = { nullptr };
constexpr ID3D11UnorderedAccessView *kNullUAV[1] = { nullptr };
constexpr ID3D11RenderTargetView *kNullRTV[1] = { nullptr };
constexpr ID3D11Buffer *kNullBuffer[1] = { nullptr };
constexpr ID3D11SamplerState *kNullSampler[1] = { nullptr };

namespace lr
{
    bool D3D11API::CreateDevice()
    {
        LOG_TRACE("Initializing D3D11 device...");

        HRESULT hr;
        u32 flags = 0;

#ifdef _DEBUG
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        constexpr D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

        // feature level that device will select
        D3D_FEATURE_LEVEL currentFeatureLevel;
        if (FAILED(hr = D3D11CreateDevice(0,
                                          D3D_DRIVER_TYPE_HARDWARE,
                                          0,
                                          flags,
                                          featureLevels,
                                          _countof(featureLevels),
                                          D3D11_SDK_VERSION,
                                          &m_pDevice,
                                          &currentFeatureLevel,
                                          &m_pContext)))

        {
            LOG_ERROR("Failed to create D3D11 device!");
            return false;
        }

        return true;
    }

    bool D3D11API::CreateDepthTexture(u32 width, u32 height)
    {
        HRESULT hr;

        TextureDesc desc = {};
        desc.Type = TextureType::Depth;

        TextureData depthData = {};
        depthData.Format = TextureFormat::R32T, depthData.Width = width, depthData.Height = height;

        m_DepthTexture.Init(&desc, &depthData);

        return true;
    }

    bool D3D11API::CreateDepthStencil()
    {
        m_SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        m_SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        m_SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        m_SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        m_SamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        m_SamplerDesc.MipLODBias = 0.0f;
        m_SamplerDesc.MinLOD = 0;
        m_SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        m_DepthStencilDesc.DepthEnable = true;
        m_DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        m_DepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

        m_DepthStencilDesc.StencilEnable = true;
        m_DepthStencilDesc.StencilReadMask = 0xFF;
        m_DepthStencilDesc.StencilWriteMask = 0xFF;

        m_DepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        m_DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        m_DepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        m_DepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        m_DepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        m_DepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        m_DepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        m_DepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        return m_pDepthStencilState = m_StateManager.Get(m_DepthStencilDesc);
    }

    bool D3D11API::CreateBlendState()
    {
        m_BlendDesc.AlphaToCoverageEnable = false;
        m_BlendDesc.RenderTarget[0].BlendEnable = true;
        m_BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        m_BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        m_BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        m_BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        m_BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        m_BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        m_BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        return m_pBlendState = m_StateManager.Get(m_BlendDesc);
    }

    bool D3D11API::CreateRasterizer()
    {
        m_RasterizerDesc.CullMode = (D3D11_CULL_MODE)Cull::Back;
        m_RasterizerDesc.FillMode = D3D11_FILL_SOLID;

        m_RasterizerDesc.DepthClipEnable = true;
        m_RasterizerDesc.DepthBias = 0;
        m_RasterizerDesc.DepthBiasClamp = 0.0f;
        m_RasterizerDesc.SlopeScaledDepthBias = 0.0f;

        m_RasterizerDesc.AntialiasedLineEnable = m_MSAACount > 0;
        m_RasterizerDesc.MultisampleEnable = m_MSAACount > 0;
        m_RasterizerDesc.FrontCounterClockwise = false;
        m_RasterizerDesc.ScissorEnable = true;

        return m_pRasterizerState = m_StateManager.Get(m_RasterizerDesc);
    }

    bool D3D11API::Init(Window *pWindow, u32 width, u32 height, APIFlags flags)
    {
        if (!CreateDevice()) return false;

        if (!pWindow)
        {
            LOG_WARN("Console application mode, only initializing D3D11 device.");
            return true;
        }

        m_pSwapChain = new SwapChain;

        /// MSAA
        if (flags & APIFlags::MSAAX2)
            m_MSAACount = 2;
        else if (flags & APIFlags::MSAAX4)
            m_MSAACount = 4;
        else if (flags & APIFlags::MSAAX8)
            m_MSAACount = 8;

        if (m_MSAACount > 1)
        {
            HRESULT hr;
            HRCallRet(m_pDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, m_MSAACount, &m_MSAAQuality),
                      "Selected MSAA feature is not supported in this hardware.",
                      false);

            m_MSAAQuality = eastl::max(0u, m_MSAAQuality - 1);
        }
        else
        {
            m_MSAACount = 1;
            m_MSAAQuality = 0;
        }

        m_pSwapChain->Init(m_pDevice, pWindow, SwapChainFlags::None, m_MSAACount, m_MSAAQuality);
        m_StateManager.SetDevice(m_pDevice);

        if (!CreateDepthTexture(width, height)) return false;
        if (!CreateDepthStencil()) return false;
        if (!CreateBlendState()) return false;
        if (!CreateRasterizer()) return false;

        m_IsContextReady = true;

        SetViewport(width, height, 1.f, 0.0f);
        SetScissor({ 0, 0, pWindow->GetWidth(), pWindow->GetHeight() });

        LOG_TRACE("Successfully initialized D3D11 device.");

        TextureDesc desc;

        constexpr u32 whiteColor = 0xffffffff;
        TextureData data;
        data.Width = 1;
        data.Height = 1;
        data.DataSize = sizeof(u32);
        data.Data = (u8 *)malloc(data.DataSize);
        data.Format = TextureFormat::RGBA8;
        memcpy(data.Data, &whiteColor, data.DataSize);

        m_PlaceholderTexture.Init(&desc, &data);

        return true;
    }

    void D3D11API::Resize(u32 width, u32 height)
    {
        if (!m_IsContextReady) return;

        m_pContext->OMSetRenderTargets(0, 0, 0);

        m_DepthTexture.Delete();
        m_pSwapChain->Resize(m_pDevice, width, height);

        CreateDepthTexture(width, height);
        SetViewport(width, height, 1.f, 0.f);
    }

    void D3D11API::SetViewport(u32 width, u32 height, float farZ, float nearZ)
    {
        if (!m_IsContextReady) return;

        D3D11_VIEWPORT vp;
        ZeroMemory(&vp, sizeof(D3D11_VIEWPORT));

        vp.Width = (float)width;
        vp.Height = (float)height;
        vp.MaxDepth = farZ;
        vp.MinDepth = nearZ;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;

        m_pContext->RSSetViewports(1, &vp);
    }

    void D3D11API::SetScissor(const XMUINT4 &lrtb)
    {
        const D3D11_RECT rect = { (LONG)lrtb.x, (LONG)lrtb.y, (LONG)lrtb.z, (LONG)lrtb.w };

        m_pContext->RSSetScissorRects(1, &rect);
    }

    void D3D11API::SetDepthFunc(DepthFunc func, bool depthEnabled)
    {
        m_DepthStencilDesc.DepthEnable = depthEnabled;
        m_DepthStencilDesc.StencilEnable = depthEnabled;  // TODO: THEY ARE SEPERATE!!!
        m_DepthStencilDesc.DepthFunc = (D3D11_COMPARISON_FUNC)func;

        auto state = m_StateManager.Get(m_DepthStencilDesc);
        if (m_pDepthStencilState != state)
        {
            m_pDepthStencilState = state;
            m_pContext->OMSetDepthStencilState(m_pDepthStencilState, 1);
        }
    }

    void D3D11API::SetCulling(Cull cull, bool counterClockwise)
    {
        m_RasterizerDesc.FrontCounterClockwise = counterClockwise;
        m_RasterizerDesc.CullMode = (D3D11_CULL_MODE)cull;

        auto state = m_StateManager.Get(m_RasterizerDesc);
        if (m_pRasterizerState != state)
        {
            m_pRasterizerState = state;
            m_pContext->RSSetState(m_pRasterizerState);
        }
    }

    void D3D11API::SetWireframeState(bool enabled)
    {
        m_RasterizerDesc.FillMode = enabled ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;

        auto state = m_StateManager.Get(m_RasterizerDesc);
        if (m_pRasterizerState != state)
        {
            m_pRasterizerState = state;
            m_pContext->RSSetState(m_pRasterizerState);
        }
    }

    void D3D11API::SetBlend(bool enableBlending, bool alphaCoverage)
    {
        m_BlendDesc.AlphaToCoverageEnable = alphaCoverage;
        m_BlendDesc.RenderTarget[0].BlendEnable = enableBlending;

        auto state = m_StateManager.Get(m_BlendDesc);
        if (m_pBlendState != state)
        {
            m_pBlendState = state;
            float factor[] = { 0, 0, 0, 0 };
            m_pContext->OMSetBlendState(m_pBlendState, factor, 0xffffffff);
        }
    }

    void D3D11API::SetPrimitiveType(PrimitiveType type)
    {
        m_pContext->IASetPrimitiveTopology(D3D::ToDXPrimitive(type));
    }

    void D3D11API::SetRenderTarget(RenderTargetView *pHandle)
    {
        if (pHandle)
        {
            ID3D11RenderTargetView *ppRenderTargetViews[pHandle->m_ColorAttachments.count];
            memset(ppRenderTargetViews, 0, sizeof(ID3D11RenderTargetView *) * pHandle->m_ColorAttachments.count);

            Texture *pDepthAttachment = pHandle->m_pDepthAttachment;
            ID3D11DepthStencilView *pDepthStencil = pDepthAttachment ? pDepthAttachment->GetDepthStencil() : nullptr;

            for (u32 i = 0; i < pHandle->m_ColorAttachmentCount; i++) ppRenderTargetViews[i] = pHandle->m_ColorAttachments[i]->GetRenderTarget();

            m_pContext->OMSetRenderTargets(pHandle->m_ColorAttachmentCount, ppRenderTargetViews, pDepthStencil);

            SetViewport(pHandle->m_Width, pHandle->m_Height, 1.0, 0.0);
        }
        else
        {
            /// We assume we wanted to set backbuffer as render target
            ID3D11RenderTargetView *pRTV = m_pSwapChain->GetRenderTarget();
            m_pContext->OMSetRenderTargets(1, &pRTV, m_DepthTexture.GetDepthStencil());

            XMUINT2 viewport = m_pSwapChain->GetViewportSize();
            SetViewport(viewport.x, viewport.y, 1.0, 0.0);
        }
    }

    void D3D11API::ClearRenderTarget(RenderTargetView *pHandle, const XMFLOAT4 &color)
    {
        if (pHandle)
        {
            for (u32 i = 0; i < pHandle->m_ColorAttachmentCount; i++)
            {
                ID3D11RenderTargetView *pRTV = pHandle->m_ColorAttachments[i]->GetRenderTarget();
                m_pContext->ClearRenderTargetView(pRTV, (float *)&color);
            }

            Texture *pDepthAttachment = pHandle->m_pDepthAttachment;
            if (pDepthAttachment)
            {
                m_pContext->ClearDepthStencilView(pDepthAttachment->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
            }
        }
        else
        {
            /// We assume we wanted to set backbuffer as render target
            ID3D11RenderTargetView *pRTV = m_pSwapChain->GetRenderTarget();

            m_pContext->ClearDepthStencilView(m_DepthTexture.GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
            m_pContext->ClearRenderTargetView(pRTV, (float *)&color);
        }
    }

    void D3D11API::SetVertexBuffer(RenderBuffer *pBuffer, InputLayout *pLayout, u32 offset)
    {
        auto stride = pLayout->GetStride();
        m_pContext->IASetVertexBuffers(0, 1, &pBuffer->GetHandle(), &stride, &offset);
    }

    void D3D11API::SetIndexBuffer(RenderBuffer *pBuffer, bool index32, u32 offset)
    {
        m_pContext->IASetIndexBuffer((ID3D11Buffer *)pBuffer->GetHandle(), index32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, offset);
    }

    void D3D11API::SetConstantBuffer(RenderBuffer *pBuffer, RenderBufferTarget target, u32 slot)
    {
        ID3D11Buffer *pNativeBuffer = *kNullBuffer;
        if (pBuffer) pNativeBuffer = pBuffer->GetHandle();

        if (target & RenderBufferTarget::Vertex) m_pContext->VSSetConstantBuffers(slot, 1, &pNativeBuffer);
        if (target & RenderBufferTarget::Pixel) m_pContext->PSSetConstantBuffers(slot, 1, &pNativeBuffer);
        if (target & RenderBufferTarget::Compute) m_pContext->CSSetConstantBuffers(slot, 1, &pNativeBuffer);
        if (target & RenderBufferTarget::Domain) m_pContext->DSSetConstantBuffers(slot, 1, &pNativeBuffer);
        if (target & RenderBufferTarget::Hull) m_pContext->HSSetConstantBuffers(slot, 1, &pNativeBuffer);
    }

    void D3D11API::SetShaderResource(RenderBuffer *pBuffer, RenderBufferTarget target, u32 slot)
    {
        ID3D11ShaderResourceView **ppSRV = kNullSRV;
        if (pBuffer) ppSRV[0] = pBuffer->GetShaderResource();

        if (target & RenderBufferTarget::Vertex) m_pContext->VSSetShaderResources(slot, 1, ppSRV);
        if (target & RenderBufferTarget::Pixel) m_pContext->PSSetShaderResources(slot, 1, ppSRV);
        if (target & RenderBufferTarget::Compute) m_pContext->CSSetShaderResources(slot, 1, ppSRV);
        if (target & RenderBufferTarget::Domain) m_pContext->DSSetShaderResources(slot, 1, ppSRV);
        if (target & RenderBufferTarget::Hull) m_pContext->HSSetShaderResources(slot, 1, ppSRV);
    }

    void D3D11API::SetUAVResource(RenderBuffer *pBuffer, RenderBufferTarget target, u32 slot)
    {
        ID3D11UnorderedAccessView *pUAV = *kNullUAV;
        if (pBuffer) pUAV = pBuffer->GetUAV();

        m_pContext->CSSetUnorderedAccessViews(slot, 1, &pUAV, 0);
    }

    void D3D11API::SetShaderResource(Texture *pTexture, RenderBufferTarget target, u32 slot)
    {
        ID3D11ShaderResourceView *pSRV = *kNullSRV;
        if (pTexture) pSRV = pTexture->GetShaderResource();

        if (target & RenderBufferTarget::Vertex) m_pContext->VSSetShaderResources(slot, 1, &pSRV);
        if (target & RenderBufferTarget::Pixel) m_pContext->PSSetShaderResources(slot, 1, &pSRV);
        if (target & RenderBufferTarget::Compute) m_pContext->CSSetShaderResources(slot, 1, &pSRV);
        if (target & RenderBufferTarget::Domain) m_pContext->DSSetShaderResources(slot, 1, &pSRV);
        if (target & RenderBufferTarget::Hull) m_pContext->HSSetShaderResources(slot, 1, &pSRV);
    }

    void D3D11API::SetUAVResource(Texture *pTexture, RenderBufferTarget target, u32 slot)
    {
        ID3D11UnorderedAccessView *pUAV = *kNullUAV;
        if (pTexture) pUAV = pTexture->GetUAV();

        m_pContext->CSSetUnorderedAccessViews(slot, 1, &pUAV, 0);
    }

    void D3D11API::SetSamplerState(TextureFiltering filtering, TextureAddress addressU, TextureAddress addressV, TextureAddress addressW,
                                   RenderBufferTarget target, u32 slot)
    {
        m_SamplerDesc.Filter = (D3D11_FILTER)D3D::ToDXTextureFiltering(filtering);
        m_SamplerDesc.AddressU = (D3D11_TEXTURE_ADDRESS_MODE)D3D::ToDXTextureAddress(addressU);
        m_SamplerDesc.AddressV = (D3D11_TEXTURE_ADDRESS_MODE)D3D::ToDXTextureAddress(addressV);
        m_SamplerDesc.AddressW = (D3D11_TEXTURE_ADDRESS_MODE)D3D::ToDXTextureAddress(addressW);

        ID3D11SamplerState *pSamplerState = *kNullSampler;
        if (filtering != TextureFiltering::Count) pSamplerState = m_StateManager.Get(m_SamplerDesc);

        if (target & RenderBufferTarget::Vertex) m_pContext->VSSetSamplers(slot, 1, &pSamplerState);
        if (target & RenderBufferTarget::Pixel) m_pContext->PSSetSamplers(slot, 1, &pSamplerState);
        if (target & RenderBufferTarget::Compute) m_pContext->CSSetSamplers(slot, 1, &pSamplerState);
        if (target & RenderBufferTarget::Domain) m_pContext->DSSetSamplers(slot, 1, &pSamplerState);
        if (target & RenderBufferTarget::Hull) m_pContext->HSSetSamplers(slot, 1, &pSamplerState);
    }

    void D3D11API::MapBuffer(RenderBuffer *pBuffer, void *pData, u32 dataSize)
    {
        D3D11_MAPPED_SUBRESOURCE mappedResc = {};
        if (SUCCEEDED(m_pContext->Map(pBuffer->GetHandle(), 0, pBuffer->GetMappingType(), 0, &mappedResc)))
        {
            memcpy(mappedResc.pData, pData, dataSize);
            m_pContext->Unmap(pBuffer->GetHandle(), 0);
        }
    }

    void D3D11API::GetGPUTexture(Texture *pCPUTexture, Texture *pGPUTexture)
    {
        m_pContext->CopyResource(pCPUTexture->GetHandle(), pGPUTexture->GetHandle());
    }

    void D3D11API::SetShader(Shader *pShader)
    {
        ShaderType type = pShader->GetType();
        auto &shaderUnion = pShader->GetShader();

        switch (type)
        {
            case ShaderType::Vertex:
                m_pContext->IASetInputLayout(pShader->GetLayout());
                m_pContext->VSSetShader(shaderUnion.m_pVertexShader, 0, 0);
                break;
            case ShaderType::Pixel: m_pContext->PSSetShader(shaderUnion.m_pPixelShader, 0, 0); break;
            case ShaderType::Compute: m_pContext->CSSetShader(shaderUnion.m_pComputeShader, 0, 0); break;
            case ShaderType::Domain: m_pContext->DSSetShader(shaderUnion.m_pDomainShader, 0, 0); break;
            case ShaderType::Hull: m_pContext->HSSetShader(shaderUnion.m_pHullShader, 0, 0); break;
            default: break;
        }
    }

    void D3D11API::DisableShader(ShaderType type)
    {
        switch (type)
        {
            case ShaderType::Vertex:
                m_pContext->IASetInputLayout(nullptr);
                m_pContext->VSSetShader(nullptr, 0, 0);
                break;
            case ShaderType::Pixel: m_pContext->PSSetShader(nullptr, 0, 0); break;
            case ShaderType::Compute: m_pContext->CSSetShader(nullptr, 0, 0); break;
            case ShaderType::Domain: m_pContext->DSSetShader(nullptr, 0, 0); break;
            case ShaderType::Hull: m_pContext->HSSetShader(nullptr, 0, 0); break;
            default: break;
        }
    }

    void D3D11API::BeginFrame()
    {
        /// Reset states
        ResetResources();

        SetRenderTarget(nullptr);
        ClearRenderTarget(nullptr);

        SetDepthFunc(DepthFunc::Less, true);
        SetCulling(Cull::Back, false);
        SetBlend(true, true);
        SetPrimitiveType(PrimitiveType::TriangleList);
    }

    void D3D11API::Frame()
    {
        m_pSwapChain->Present();
    }

    void D3D11API::Draw(u32 vertexCount)
    {
        m_pContext->Draw(vertexCount, 0);
    }

    void D3D11API::DrawIndexed(u32 indexCount, u32 startIndex, u32 baseVertex)
    {
        m_pContext->DrawIndexed(indexCount, startIndex, baseVertex);
    }

    void D3D11API::Dispatch(u32 thrX, u32 thrY, u32 thrZ)
    {
        m_pContext->Dispatch(thrX, thrY, thrZ);
    }

    void D3D11API::ResetResources()
    {
        ID3D11ShaderResourceView *ppShaderResources[Renderer::Config::kMaxRenderTargets];
        memset(ppShaderResources, 0, sizeof(ppShaderResources));
        m_pContext->PSSetShaderResources(0, Renderer::Config::kMaxRenderTargets, ppShaderResources);
        m_pContext->VSSetShaderResources(0, Renderer::Config::kMaxRenderTargets, ppShaderResources);
        m_pContext->CSSetShaderResources(0, Renderer::Config::kMaxRenderTargets, ppShaderResources);
    }

    void D3D11API::CreateTexture2D(Texture *pTexture, TextureType type, TextureData *pData)
    {
        ID3D11Texture2D *&pHandle = pTexture->GetHandle();

        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.ArraySize = 1;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.Format = D3D::TextureFormatToDXFormat(pData->Format);
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureDesc.Width = pData->Width;
        textureDesc.Height = pData->Height;

        if ((textureDesc.MipLevels = pTexture->GetMipCount()) > 1)
        {
            textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
        }

        switch (type)
        {
            case TextureType::RenderTarget:
                textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
                textureDesc.SampleDesc.Quality = m_MSAAQuality;
                textureDesc.SampleDesc.Count = m_MSAACount;
                break;

            case TextureType::Depth:
                textureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
                textureDesc.SampleDesc.Quality = m_MSAAQuality;
                textureDesc.SampleDesc.Count = m_MSAACount;
                break;

            case TextureType::RW: textureDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS; break;
            default: break;
        }

        HRESULT hr;
        HRCall(m_pDevice->CreateTexture2D(&textureDesc, nullptr, &pHandle), "Failed to create Texture2D!");

        if (pData->Data)
        {
            m_pContext->UpdateSubresource(pHandle, 0, nullptr, pData->Data, TextureFormatToSize(pData->Format) * pData->Width, 0);
        }
    }

    void D3D11API::CreateShaderResource(Texture *pTexture, TextureFormat format, u32 mipCount)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Texture2D.MipLevels = mipCount;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Format = D3D::TextureFormatToDXFormat(format);

        if (m_MSAACount > 1)
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
        else
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

        ID3D11Texture2D *pHandle = pTexture->GetHandle();
        ID3D11ShaderResourceView *&pShaderResource = pTexture->GetShaderResource();

        HRESULT hr;
        HRCall(m_pDevice->CreateShaderResourceView(pHandle, &srvDesc, &pShaderResource), "Failed to create D3D11 texture shader resource view!");
    }

    void D3D11API::GenerateMips(Texture *pTexture)
    {
        ID3D11ShaderResourceView *pShaderResource = pTexture->GetShaderResource();

        if (pTexture->GetMipCount() > 1) m_pContext->GenerateMips(pShaderResource);
    }

    void D3D11API::CreateDepthStencilView(Texture *pTexture)
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;

        if (m_MSAACount > 1)
            depthStencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        else
            depthStencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

        ID3D11Texture2D *pHandle = pTexture->GetHandle();
        ID3D11DepthStencilView *&pDepthStencil = pTexture->GetDepthStencil();

        HRESULT hr;
        HRCall(m_pDevice->CreateDepthStencilView(pHandle, &depthStencilDesc, &pDepthStencil), "Failed to create Depth Stencil View!");
    }

    void D3D11API::CreateRenderTargetView(Texture *pTexture, TextureFormat format)
    {
        D3D11_RENDER_TARGET_VIEW_DESC viewDesc = {};
        viewDesc.Format = D3D::TextureFormatToDXFormat(format);
        viewDesc.Texture2D.MipSlice = 1;
        viewDesc.Texture2D.MipSlice = 0;

        if (m_MSAACount > 1)
            viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
        else
            viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

        ID3D11Texture2D *pHandle = pTexture->GetHandle();
        ID3D11RenderTargetView *&pRenderTarget = pTexture->GetRenderTarget();

        HRESULT hr;
        HRCall(m_pDevice->CreateRenderTargetView(pHandle, &viewDesc, &pRenderTarget), "Failed to create Render Target View!");
    }

    void D3D11API::CreateUnorderedAccessView(Texture *pTexture)
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = 0;

        ID3D11Texture2D *pHandle = pTexture->GetHandle();
        ID3D11UnorderedAccessView *&pUAV = pTexture->GetUAV();

        HRESULT hr;
        HRCall(m_pDevice->CreateUnorderedAccessView(pHandle, &uavDesc, &pUAV), "Failed to create Unordered Access View!");
    }

}  // namespace lr