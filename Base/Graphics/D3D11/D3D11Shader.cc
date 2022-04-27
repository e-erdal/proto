#include "D3D11Shader.hh"

#include "Core/BaseApp.hh"
#include "Graphics/D3D.hh"
#include "Scripting/ffd.hh"
#include "IO/FileStream.hh"

#include <d3dcompiler.h>
#include <d3d11shader.h>

namespace lr
{
    static eastl::string GetFullShaderModel(ShaderType type, const eastl::string &model)
    {
        const char *pVSTag = "vs";
        const char *pPSTag = "ps";
        const char *pCSTag = "cs";
        const char *pDSTag = "ds";
        const char *pHSTag = "hs";

        const char *pShaderTag = nullptr;

        switch (type)
        {
            case ShaderType::Vertex: pShaderTag = pVSTag; break;
            case ShaderType::Pixel: pShaderTag = pPSTag; break;
            case ShaderType::Compute: pShaderTag = pCSTag; break;
            case ShaderType::Domain: pShaderTag = pDSTag; break;
            case ShaderType::Hull: pShaderTag = pHSTag; break;
            default: break;
        }

        return Format("{}_{}", pShaderTag, model.c_str()).c_str();
    }

    constexpr u32 ToOptimizeLevel(u8 val)
    {
        switch (val)
        {
            case 0: return D3DCOMPILE_OPTIMIZATION_LEVEL0;
            case 1: return D3DCOMPILE_OPTIMIZATION_LEVEL1;
            case 2: return D3DCOMPILE_OPTIMIZATION_LEVEL2;
            case 3: return D3DCOMPILE_OPTIMIZATION_LEVEL3;
            default: break;
        }

        return D3DCOMPILE_OPTIMIZATION_LEVEL1;
    }

    void Shader::Init(ShaderDesc *pDesc, eastl::string_view path)
    {
        HRESULT hr;
        ID3D11Device *pDevice = GetApp()->GetAPI()->GetDevice();

        FileStream fs(Format("Resources/Shaders/{}.hlsl", path), false);
        if (!fs.IsOK())
        {
            LOG_ERROR("Couldn't open shader {}.", path);
            return;
        }

        char *pFileData = fs.ReadAll<char>();
        eastl::string code(pFileData, fs.Size());
        free(pFileData);
        fs.Close();

        ShaderData shaderData;
        eastl::string vsEntry = "VSMain";
        eastl::string psEntry = "PSMain";
        eastl::string csEntry = "";
        eastl::string dsEntry = "";
        eastl::string hsEntry = "";

        /// Search if we override shader settings
        constexpr eastl::string_view kSettingsOverrideTag = "_OverrideSettings_";
        constexpr u32 kOverrideTagLen = kSettingsOverrideTag.length();
        u32 overrideTagPosBegin = code.find(kSettingsOverrideTag.data());
        u32 overrideTagPosEnd = code.rfind(kSettingsOverrideTag.data());
        u32 overrideCodeBegin = overrideTagPosBegin + kOverrideTagLen;
        u32 overrideCodeEnd = overrideTagPosEnd + kOverrideTagLen;

        // We override settings
        if (overrideTagPosBegin != -1 && overrideTagPosEnd != -1)
        {
            eastl::string overrideCode = code.substr(overrideCodeBegin, overrideTagPosEnd - kOverrideTagLen);

            ffd overrideInfo;
            overrideInfo.FromMemory(overrideCode.c_str(), overrideCode.length());

            vsEntry = overrideInfo.Global().AsString("EntryVS");
            psEntry = overrideInfo.Global().AsString("EntryPS");
            csEntry = overrideInfo.Global().AsString("EntryCS");
            dsEntry = overrideInfo.Global().AsString("EntryDS");
            hsEntry = overrideInfo.Global().AsString("EntryHS");
        }

        if (overrideTagPosBegin != -1)
        {
            code.replace(overrideTagPosBegin, kOverrideTagLen, "/*");

            if (overrideTagPosEnd != -1)
            {
                code.replace(overrideTagPosEnd - kOverrideTagLen, kOverrideTagLen + 2, "\n*/");
            }
        }

        ID3DBlob *pError = nullptr;
        ID3DBlob *pData = nullptr;

        auto CompileShader = [&](const eastl::string &entry, ShaderType type) {
            u32 flags = D3DCOMPILE_ENABLE_STRICTNESS | ToOptimizeLevel(0);

            D3DCompile(code.c_str(),
                       code.length(),
                       path.data(),
                       nullptr,
                       D3D_COMPILE_STANDARD_FILE_INCLUDE,
                       entry.data(),
                       GetFullShaderModel(type, "5_0").data(),
                       flags,
                       0,
                       &pData,
                       &pError);

            if (pError)
            {
                LOG_ERROR("Failed to compile shader!\n\n {}", (char *)pError->GetBufferPointer());

                pError->Release();
                pError = nullptr;
            }

            shaderData.DataLen = pData->GetBufferSize();
            shaderData.pData = (u8 *)malloc(shaderData.DataLen);
            memcpy(shaderData.pData, pData->GetBufferPointer(), shaderData.DataLen);

            if (shaderData.DataLen == 0)
            {
                LOG_ERROR("Shader type({}) not found in data while trying to load.", (i32)pDesc->Type);
                return;
            }

            SAFE_RELEASE(pData);
        };

        switch (pDesc->Type)
        {
            case ShaderType::Vertex:
            {
                if (vsEntry.length())
                {
                    CompileShader(vsEntry, ShaderType::Vertex);
                    HRCallRet(pDevice->CreateVertexShader(shaderData.pData, shaderData.DataLen, 0, &m_Shader.m_pVertexShader),
                              "Failed to create D3D11 vertex shader!",
                              break);

                    if (pDesc->pLayout)
                    {
                        eastl::vector<D3D11_INPUT_ELEMENT_DESC> elements;
                        for (const auto &element : pDesc->pLayout->GetElements())
                        {
                            DXGI_FORMAT type = D3D::VertexTypeToDXFormat(element.m_Type);  // long ass d3d functions, had to satisfy clang format
                            elements.push_back({ element.m_Name.data(), 0, type, 0, element.m_Offset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
                        }

                        pDevice->CreateInputLayout(&elements[0], elements.size(), shaderData.pData, shaderData.DataLen, &m_pLayout);
                    }
                }

                break;
            }

            case ShaderType::Pixel:
            {
                if (psEntry.length())
                {
                    CompileShader(psEntry, ShaderType::Pixel);
                    HRCallRet(pDevice->CreatePixelShader(shaderData.pData, shaderData.DataLen, 0, &m_Shader.m_pPixelShader),
                              "Failed to create D3D11 pixel shader!",
                              break);
                }
                break;
            }

            case ShaderType::Compute:
            {
                if (csEntry.length())
                {
                    CompileShader(csEntry, ShaderType::Compute);
                    HRCallRet(pDevice->CreateComputeShader(shaderData.pData, shaderData.DataLen, 0, &m_Shader.m_pComputeShader),
                              "Failed to create D3D11 compute shader!",
                              break);
                }

                break;
            }

            case ShaderType::Domain:
            {
                if (dsEntry.length())
                {
                    CompileShader(dsEntry, ShaderType::Domain);
                    HRCallRet(pDevice->CreateDomainShader(shaderData.pData, shaderData.DataLen, 0, &m_Shader.m_pDomainShader),
                              "Failed to create D3D11 domain shader!",
                              break);
                }

                break;
            }

            case ShaderType::Hull:
            {
                if (hsEntry.length())
                {
                    CompileShader(hsEntry, ShaderType::Hull);
                    HRCallRet(pDevice->CreateHullShader(shaderData.pData, shaderData.DataLen, 0, &m_Shader.m_pHullShader),
                              "Failed to create D3D11 hull shader!",
                              break);
                }

                break;
            }

            default: break;
        }

        SAFE_FREE(shaderData.pData);

        m_Type = pDesc->Type;
    }

}  // namespace lr