//
// Created on Monday 18th October 2021 by e-erdal
// A tool that will help API to handle/cache states
//

#pragma once

#include "Utils/Hash.hh"

HASH_INTERFACE(D3D11_RASTERIZER_DESC);
HASH_INTERFACE(D3D11_DEPTH_STENCIL_DESC);
HASH_INTERFACE(D3D11_BLEND_DESC);
HASH_INTERFACE(D3D11_SAMPLER_DESC);

namespace lr
{
    class D3D11APIStateManager
    {
    public:
        void SetDevice(ID3D11Device *pDevice);

        ID3D11RasterizerState *Get(D3D11_RASTERIZER_DESC &desc);
        ID3D11DepthStencilState *Get(D3D11_DEPTH_STENCIL_DESC &desc);
        ID3D11BlendState *Get(D3D11_BLEND_DESC &desc);
        ID3D11SamplerState *Get(D3D11_SAMPLER_DESC &desc);

    private:
        eastl::unordered_map<D3D11_RASTERIZER_DESC, ID3D11RasterizerState *> m_RasterizerStates;
        eastl::unordered_map<D3D11_DEPTH_STENCIL_DESC, ID3D11DepthStencilState *> m_DepthStates;
        eastl::unordered_map<D3D11_BLEND_DESC, ID3D11BlendState *> m_BlendStates;
        eastl::unordered_map<D3D11_SAMPLER_DESC, ID3D11SamplerState *> m_SamplerStates;

        ID3D11Device *m_pDevice = 0;
    };

}  // namespace lr

// eastl::unordered_map operator support
OPR_INTERFACE_CMP(D3D11_RASTERIZER_DESC);
OPR_INTERFACE_CMP(D3D11_DEPTH_STENCIL_DESC);
OPR_INTERFACE_CMP(D3D11_BLEND_DESC);
OPR_INTERFACE_CMP(D3D11_SAMPLER_DESC);