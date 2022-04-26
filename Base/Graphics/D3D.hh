// 
// Created on Saturday 23rd April 2022 by e-erdal
// 

#pragma once

#include "Graphics/D3D11/InputLayout.hh"
#include "Graphics/D3D11/D3D11Texture.hh"

namespace lr::D3D
{
    inline DXGI_FORMAT TextureFormatToDXFormat(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::BC1: return DXGI_FORMAT_BC1_UNORM;

            case TextureFormat::RGBA8: return DXGI_FORMAT_R8G8B8A8_UNORM;
            case TextureFormat::RGBA16: return DXGI_FORMAT_R16G16B16A16_UNORM;
            case TextureFormat::RGBA32F: return DXGI_FORMAT_R32G32B32A32_FLOAT;

            case TextureFormat::BGRA8: return DXGI_FORMAT_B8G8R8A8_UNORM;

            case TextureFormat::R24TG8T: return DXGI_FORMAT_R24G8_TYPELESS;

            case TextureFormat::R32T: return DXGI_FORMAT_R32_TYPELESS;
            case TextureFormat::R32U: return DXGI_FORMAT_R32_UINT;
            case TextureFormat::R32F: return DXGI_FORMAT_R32_FLOAT;

            case TextureFormat::DEPTH32F: return DXGI_FORMAT_D32_FLOAT;
            case TextureFormat::DEPTH24_STENCIL8: return DXGI_FORMAT_D24_UNORM_S8_UINT;

            default: return DXGI_FORMAT_UNKNOWN;
        }
    }

    inline DXGI_FORMAT VertexTypeToDXFormat(VertexAttribType type)
    {
        switch (type)
        {
            case VertexAttribType::Float: return DXGI_FORMAT_R32_FLOAT;
            case VertexAttribType::Vec2: return DXGI_FORMAT_R32G32_FLOAT;
            case VertexAttribType::Vec3: return DXGI_FORMAT_R32G32B32_FLOAT;
            case VertexAttribType::Vec3PK: return DXGI_FORMAT_R11G11B10_FLOAT;
            case VertexAttribType::Vec4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case VertexAttribType::UInt: return DXGI_FORMAT_R32_UINT;
            case VertexAttribType::UInt4PKN: return DXGI_FORMAT_R8G8B8A8_UNORM;
            default: return DXGI_FORMAT_UNKNOWN;
        }
    }

    constexpr D3D_PRIMITIVE_TOPOLOGY ToDXPrimitive(PrimitiveType type)
    {
        switch (type)
        {
            case PrimitiveType::PointList: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            case PrimitiveType::LineList: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            case PrimitiveType::LineStrip: return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
            case PrimitiveType::TriangleList: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case PrimitiveType::TriangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            case PrimitiveType::Patch_3: return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
        }
    }

    constexpr D3D11_FILTER ToDXTextureFiltering(TextureFiltering filtering)
    {
        switch (filtering)
        {
            case TextureFiltering::Point: return D3D11_FILTER_MIN_MAG_MIP_POINT;
            case TextureFiltering::Linear: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            case TextureFiltering::Ansio: return D3D11_FILTER_ANISOTROPIC;
            default: return D3D11_FILTER_MIN_MAG_MIP_POINT;
        }
    }

    constexpr D3D11_TEXTURE_ADDRESS_MODE ToDXTextureAddress(TextureAddress address)
    {
        switch (address)
        {
            case TextureAddress::Border: return D3D11_TEXTURE_ADDRESS_BORDER;
            case TextureAddress::Wrap: return D3D11_TEXTURE_ADDRESS_WRAP;
            case TextureAddress::Mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
            case TextureAddress::Clamp: return D3D11_TEXTURE_ADDRESS_CLAMP;
        }
    }

}  // namespace lr::D3D