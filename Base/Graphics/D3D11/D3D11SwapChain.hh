//
// Created on Sunday 2nd January 2022 by e-erdal
//

#pragma once

namespace lr
{
    enum class SwapChainFlags : u8
    {
        None,
        vSync = 1 << 0,
        AllowTearing = 1 << 1,
        TripleBuffering = 1 << 2,
    };
    BitFlags(SwapChainFlags);

    class Window;
    class SwapChain
    {
    public:
        SwapChain() = default;

        void Init(ID3D11Device *pDevice, Window *pWindow, SwapChainFlags flags, u32 msaaCount, u32 msaaQuality);
        void Resize(ID3D11Device *pDevice, u32 width, u32 height);
        void Present();

        ID3D11Texture2D *GetBackBuffer();
        ID3D11RenderTargetView *GetRenderTarget()
        {
            return m_pBackBufferRTV;
        }

        XMUINT2 GetViewportSize()
        {
            return XMUINT2(m_SwapChainDesc.BufferDesc.Width, m_SwapChainDesc.BufferDesc.Height);
        }

    private:
        IDXGISwapChain *m_pHandle = nullptr;
        DXGI_SWAP_CHAIN_DESC m_SwapChainDesc;

        ID3D11RenderTargetView *m_pBackBufferRTV = nullptr;

        bool m_vSync = false;
        bool m_Tearing = false;
    };

}  // namespace lr