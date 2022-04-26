//
// Created on Thursday 14th October 2021 by e-erdal
//

#pragma once

namespace lr
{
    enum class RenderBufferUsage : u8
    {
        Default,    // GPU RW
        Dynamic,    // GPU R     CPU W
        Immutable,  // GPU R
        Staging     // GPU RW    CPU RW
    };

    // MemoryFlags is the probably best naming for this, DataFlags looks weird
    enum class RenderBufferMemoryFlags : u16
    {
        None = 0,
        Structured = 1 << 0,
        AllowRawViews = 1 << 1,

        // Access
        Access_CPUW = 1 << 2,
        Access_CPUR = 1 << 3,

        // UAV
        UAV_Counter = 1 << 4,
        UAV_Append = 1 << 5
    };
    BitFlags(RenderBufferMemoryFlags);

    enum class RenderBufferType : u8
    {
        None,
        Vertex = 1 << 0,
        Index = 1 << 1,
        Constant = 1 << 2,
        ShaderResource = 1 << 3,
        UAV = 1 << 4
    };
    BitFlags(RenderBufferType);

    enum class RenderBufferTarget : u8
    {
        Vertex = 1 << 0,
        Pixel = 1 << 1,
        Compute = 1 << 2,
        Domain = 1 << 3,
        Hull = 1 << 4,

        Count = 5,
    };
    BitFlags(RenderBufferTarget);

    struct RenderBufferDesc
    {
        RenderBufferType Type = RenderBufferType::None;
        RenderBufferUsage Usage = RenderBufferUsage::Default;
        RenderBufferMemoryFlags MemFlags = RenderBufferMemoryFlags::None;

        void *pData = nullptr;  // Data if available
        size_t DataLen = 0;

        u32 ByteStride = 0;
        u32 UAVElements = 0;  // The number of elements in the resource. For structured buffers, this is the number of structures in the buffer.
    };

    class RenderBuffer
    {
    public:
        void Init(const RenderBufferDesc &desc);
        bool Valid();

        void Delete();
        virtual ~RenderBuffer();

    public:
        ID3D11Buffer *&GetHandle()
        {
            return m_pHandle;
        }

        ID3D11ShaderResourceView *GetShaderResource()
        {
            return m_pSRV;
        }

        ID3D11UnorderedAccessView *GetUAV()
        {
            return m_pUAV;
        }

        D3D11_MAP GetMappingType()
        {
            return m_Mapping;
        }

    private:
        ID3D11Buffer *m_pHandle = 0;

        ID3D11ShaderResourceView *m_pSRV = 0;
        ID3D11UnorderedAccessView *m_pUAV = 0;

        D3D11_MAP m_Mapping;
    };
}  // namespace lr