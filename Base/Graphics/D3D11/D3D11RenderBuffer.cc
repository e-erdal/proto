#include "D3D11RenderBuffer.hh"

#include "Core/BaseApp.hh"

namespace lr
{
    void RenderBuffer::Init(const RenderBufferDesc &desc)
    {
        HRESULT hr;
        auto *pDevice = GetApp()->GetAPI()->GetDevice();

        D3D11_BUFFER_DESC d11Desc = {};
        d11Desc.ByteWidth = desc.DataLen;

        if (desc.MemFlags & RenderBufferMemoryFlags::Access_CPUR) d11Desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
        if (desc.MemFlags & RenderBufferMemoryFlags::Access_CPUW) d11Desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;

        switch (desc.Usage)
        {
            case RenderBufferUsage::Default: d11Desc.Usage = D3D11_USAGE_DEFAULT; break;
            case RenderBufferUsage::Immutable: d11Desc.Usage = D3D11_USAGE_IMMUTABLE; break;
            case RenderBufferUsage::Dynamic: d11Desc.Usage = D3D11_USAGE_DYNAMIC; break;
            case RenderBufferUsage::Staging: d11Desc.Usage = D3D11_USAGE_STAGING; break;
        }

        // Select the buffer mapping type
        if (desc.Usage == RenderBufferUsage::Dynamic
            && (desc.MemFlags & RenderBufferMemoryFlags::Access_CPUR && desc.MemFlags & RenderBufferMemoryFlags::Access_CPUW))
            m_Mapping = D3D11_MAP_READ_WRITE;
        else if (desc.Usage == RenderBufferUsage::Dynamic && desc.MemFlags & RenderBufferMemoryFlags::Access_CPUW)
            m_Mapping = D3D11_MAP_WRITE_DISCARD;
        else if (desc.MemFlags & RenderBufferMemoryFlags::Access_CPUR)
            m_Mapping = D3D11_MAP_READ;
        else if (desc.MemFlags & RenderBufferMemoryFlags::Access_CPUW)
            m_Mapping = D3D11_MAP_WRITE;

        // Note: D3D11_MAP_WRITE_NO_OVERWRITE Cannot be used on a resource created with the D3D11_BIND_CONSTANT_BUFFER flag.

        if (desc.Type & RenderBufferType::Vertex) d11Desc.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
        if (desc.Type & RenderBufferType::Index) d11Desc.BindFlags |= D3D11_BIND_INDEX_BUFFER;
        if (desc.Type & RenderBufferType::Constant) d11Desc.BindFlags |= D3D11_BIND_CONSTANT_BUFFER;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        if (desc.Type & RenderBufferType::ShaderResource)
        {
            d11Desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

            srvDesc.Format = DXGI_FORMAT_UNKNOWN;

            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
            srvDesc.BufferEx.NumElements = desc.DataLen / desc.ByteStride;
            d11Desc.StructureByteStride = desc.ByteStride;

            // https://stackoverflow.com/questions/32049639
            if (desc.MemFlags & RenderBufferMemoryFlags::Structured)
            {
                srvDesc.Format = DXGI_FORMAT_UNKNOWN;

                d11Desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
            }
            else if (desc.MemFlags & RenderBufferMemoryFlags::AllowRawViews)
            {
                srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;

                d11Desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
            }
        }

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        if (desc.Type & RenderBufferType::UAV)
        {
            d11Desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

            uavDesc.Format = DXGI_FORMAT_UNKNOWN;

            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.FirstElement = 0;
            uavDesc.Buffer.NumElements = desc.UAVElements;

            if (desc.MemFlags & RenderBufferMemoryFlags::AllowRawViews)
            {
                uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
            }
            else if (desc.MemFlags & RenderBufferMemoryFlags::UAV_Append)
            {
                uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
            }
            else if (desc.MemFlags & RenderBufferMemoryFlags::UAV_Counter)
            {
                uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
            }
            else if (desc.MemFlags & RenderBufferMemoryFlags::Structured)
            {
                uavDesc.Buffer.Flags = 0;
            }
        }

        if (desc.pData)
        {
            D3D11_SUBRESOURCE_DATA data = {};
            data.pSysMem = desc.pData;

            pDevice->CreateBuffer(&d11Desc, &data, &m_pHandle);
        }
        else
        {
            pDevice->CreateBuffer(&d11Desc, 0, &m_pHandle);
        }

        if (desc.Type & RenderBufferType::ShaderResource)
        {
            if (FAILED(hr = pDevice->CreateShaderResourceView(m_pHandle, &srvDesc, &m_pSRV)))
            {
                LOG_ERROR("Failed to create D3D11 shader resource view!");
                return;
            }
        }

        if (desc.Type & RenderBufferType::UAV)
        {
            if (FAILED(hr = pDevice->CreateUnorderedAccessView(m_pHandle, &uavDesc, &m_pUAV)))
            {
                LOG_ERROR("Failed to create D3D11 UAV!");
                return;
            }
        }
    }

    bool RenderBuffer::Valid()
    {
        return m_pHandle;
    }

    void RenderBuffer::Delete()
    {
        SAFE_RELEASE(m_pHandle);
        SAFE_RELEASE(m_pSRV);
        SAFE_RELEASE(m_pUAV);
    }

    RenderBuffer::~RenderBuffer()
    {
        Delete();
    }

}  // namespace lr