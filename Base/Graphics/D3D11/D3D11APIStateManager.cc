#include "D3D11APIStateManager.hh"

namespace lr
{
    void D3D11APIStateManager::SetDevice(ID3D11Device *pDevice)
    {
        m_pDevice = pDevice;
    }

    ID3D11RasterizerState *D3D11APIStateManager::Get(D3D11_RASTERIZER_DESC &desc)
    {
        HRESULT hr;
        auto it = m_RasterizerStates.find(desc);
        if (it == m_RasterizerStates.end())
        {
            ID3D11RasterizerState *pRasterizer = 0;
            HRCallRet(hr = m_pDevice->CreateRasterizerState(&desc, &pRasterizer), "Failed to create new rasterizer state!", nullptr);

            m_RasterizerStates.emplace(desc, pRasterizer);
            return pRasterizer;
        }

        return it->second;
    }

    ID3D11DepthStencilState *D3D11APIStateManager::Get(D3D11_DEPTH_STENCIL_DESC &desc)
    {
        HRESULT hr;
        auto it = m_DepthStates.find(desc);
        if (it == m_DepthStates.end())
        {
            ID3D11DepthStencilState *pStencil = 0;
            HRCallRet(hr = m_pDevice->CreateDepthStencilState(&desc, &pStencil), "Failed to create new depth stencil state!", nullptr);

            m_DepthStates.emplace(desc, pStencil);
            return pStencil;
        }

        return it->second;
    }

    ID3D11BlendState *D3D11APIStateManager::Get(D3D11_BLEND_DESC &desc)
    {
        HRESULT hr;
        auto it = m_BlendStates.find(desc);
        if (it == m_BlendStates.end())
        {
            ID3D11BlendState *pBlend = 0;
            HRCallRet(hr = m_pDevice->CreateBlendState(&desc, &pBlend), "Failed to create new blend state!", nullptr);

            m_BlendStates.emplace(desc, pBlend);
            return pBlend;
        }

        return it->second;
    }

    ID3D11SamplerState *D3D11APIStateManager::Get(D3D11_SAMPLER_DESC &desc)
    {
        HRESULT hr;
        auto it = m_SamplerStates.find(desc);
        if (it == m_SamplerStates.end())
        {
            ID3D11SamplerState *pSampler = nullptr;
            HRCallRet(hr = m_pDevice->CreateSamplerState(&desc, &pSampler), "Failed to create new sampler state!", nullptr);

            m_SamplerStates.emplace(desc, pSampler);
            return pSampler;
        }

        return it->second;
    }

}  // namespace lr