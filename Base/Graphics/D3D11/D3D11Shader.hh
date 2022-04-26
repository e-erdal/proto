//
// Created on Wednesday 13th October 2021 by e-erdal
//

#pragma once

#include "InputLayout.hh"

namespace lr
{
    enum class ShaderType : u8
    {
        Vertex,
        Pixel,
        Compute,

        Domain,
        Hull,

        Count
    };

    struct ShaderDesc
    {
        ShaderType Type;
        InputLayout *pLayout = nullptr;
    };

    struct ShaderData
    {
        u8 *pData = nullptr;
        u32 DataLen = 0;
    };

    class Shader
    {
    public:
        void Init(ShaderDesc *pDesc, eastl::string_view path);

    private:
        union
        {
            ID3D11VertexShader *m_pVertexShader = 0;
            ID3D11PixelShader *m_pPixelShader;
            ID3D11ComputeShader *m_pComputeShader;
            ID3D11DomainShader *m_pDomainShader;
            ID3D11HullShader *m_pHullShader;
        } m_Shader;

        ID3D11InputLayout *m_pLayout = nullptr;  // Input layout if shader type is Vertex
        ShaderType m_Type;

    public:
        const auto &GetShader() const
        {
            return m_Shader;
        }

        ShaderType GetType()
        {
            return m_Type;
        }

        ID3D11InputLayout *GetLayout()
        {
            return m_pLayout;
        }
    };

}  // namespace lr