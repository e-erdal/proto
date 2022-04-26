//
// Created on July 6th 2021 by e-erdal.
//

#pragma once

#include "Graphics/Type.hh"

#include "D3D11Texture.hh"
#include "D3D11Shader.hh"
#include "D3D11RenderBuffer.hh"
#include "D3D11APIStateManager.hh"

#include "RenderTargetView.hh"

#include "InputLayout.hh"

namespace lr
{
    /// Renderer configurations
    namespace Renderer
    {
        namespace Config
        {
            constexpr u32 kMaxFrameBuffers = 3;
            constexpr u32 kMaxRenderTargets = 8;
        }  // namespace Config

    }  // namespace Renderer

    class Window;
    class SwapChain;
    class D3D11API
    {
    public:
        bool Init(Window *pWindow, u32 width, u32 height, APIFlags flags);

        /// SWAPCHAIN ///
        void Resize(u32 width, u32 height);

        /// RS STATE ///
        void SetViewport(u32 width, u32 height, float farZ, float nearZ);
        void SetScissor(const XMUINT4 &lrtb);
        void SetDepthFunc(DepthFunc func, bool depthEnabled);
        void SetCulling(Cull cull, bool counterClockwise);
        void SetWireframeState(bool enabled);
        void SetBlend(bool enableBlending, bool alphaCoverage);
        void SetPrimitiveType(PrimitiveType type);

        /// RENDER TARGETS ///
        void SetRenderTarget(RenderTargetView *pHandle);
        void ClearRenderTarget(RenderTargetView *pHandle, const XMFLOAT4 &color = { 0, 0, 0, 1 });

        /// IA STATE ///
        void SetVertexBuffer(RenderBuffer *pBuffer, InputLayout *pLayout, u32 offset = 0);
        void SetIndexBuffer(RenderBuffer *pBuffer, bool index32 = true, u32 offset = 0);

        /// RENDER BUFFERS ///
        void SetConstantBuffer(RenderBuffer *pBuffer, RenderBufferTarget target, u32 slot);

        void SetShaderResource(RenderBuffer *pBuffer, RenderBufferTarget target, u32 slot);
        void SetUAVResource(RenderBuffer *pBuffer, RenderBufferTarget target, u32 slot);

        void SetShaderResource(Texture *pTexture, RenderBufferTarget target, u32 slot);
        void SetUAVResource(Texture *pTexture, RenderBufferTarget target, u32 slot);
        void SetSamplerState(TextureFiltering filtering, TextureAddress addressU, TextureAddress addressV, TextureAddress addressW,
                             RenderBufferTarget target, u32 slot);

        void MapBuffer(RenderBuffer *pBuffer, void *pData, u32 dataSize);
        void GetGPUTexture(Texture *pCPUTexture, Texture *pGPUTexture);

        /// SHADERS  ///
        void SetShader(Shader *pShader);
        void DisableShader(ShaderType type);

        /// DRAWING STAGE ///
        void BeginFrame();
        void Frame();
        void Draw(u32 vertexCount);
        void DrawIndexed(u32 indexCount, u32 startIndex, u32 baseVertex);
        void Dispatch(u32 thrX, u32 thrY, u32 thrZ);

        /// STATE CACHE ///
        void ResetResources();

        /// TEXTURES ///
        void CreateTexture2D(Texture *pTexture, TextureType type, TextureData *pData);
        void CreateShaderResource(Texture *pTexture, TextureFormat format, u32 mipCount);
        void GenerateMips(Texture *pTexture);

        void CreateDepthStencilView(Texture *pTexture);
        void CreateRenderTargetView(Texture *pTexture, TextureFormat format);
        void CreateUnorderedAccessView(Texture *pTexture);

    private:
        bool CreateDevice();
        bool CreateDepthTexture(u32 width, u32 height);
        bool CreateDepthStencil();
        bool CreateBlendState();
        bool CreateRasterizer();

    public:
        ID3D11Device *GetDevice() const
        {
            return m_pDevice;
        }

        ID3D11DeviceContext *GetDeviceContext() const
        {
            return m_pContext;
        }

    private:
        ID3D11Device *m_pDevice = nullptr;
        ID3D11DeviceContext *m_pContext = nullptr;
        SwapChain *m_pSwapChain = nullptr;

        D3D11APIStateManager m_StateManager;

        Texture m_DepthTexture;
        Texture m_PlaceholderTexture;

        D3D11_BLEND_DESC m_BlendDesc = {};
        D3D11_SAMPLER_DESC m_SamplerDesc = {};
        D3D11_RASTERIZER_DESC m_RasterizerDesc = {};
        D3D11_DEPTH_STENCIL_DESC m_DepthStencilDesc = {};

        /// Context states pointing to State Manager
        ID3D11BlendState *m_pBlendState = nullptr;
        ID3D11SamplerState *m_pSamplerState = nullptr;
        ID3D11RasterizerState *m_pRasterizerState = nullptr;
        ID3D11DepthStencilState *m_pDepthStencilState = nullptr;

        bool m_NeedToPresent = true;
        bool m_IsContextReady = false;

        u32 m_MSAACount = 1;
        u32 m_MSAAQuality = 0;
    };

}  // namespace lr
