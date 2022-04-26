//
// Created on Saturday 23rd April 2022 by e-erdal
//

#pragma once

namespace lr
{
    enum class VertexAttribType
    {
        None = 0,
        Float,
        Vec2,
        Vec3,
        Vec3PK,
        Vec4,
        Mat4,
        UInt,
        UInt4PKN  /// 4x(0-255 range) unsigned ints packet into a normalized unsigned int
    };

    static u32 VertexAttribSize(VertexAttribType type)
    {
        switch (type)
        {
            case VertexAttribType::Float: return sizeof(float);
            case VertexAttribType::Vec2: return sizeof(XMFLOAT2);
            case VertexAttribType::Vec3: return sizeof(XMFLOAT3);
            case VertexAttribType::Vec3PK: return sizeof(XMFLOAT3PK);
            case VertexAttribType::Vec4: return sizeof(XMVECTOR);
            case VertexAttribType::Mat4: return sizeof(XMMATRIX);
            case VertexAttribType::UInt: return sizeof(u32);
            case VertexAttribType::UInt4PKN: return sizeof(u32);
            default: return 0;
        }
    }

    struct VertexAttrib
    {
        VertexAttrib() = default;

        VertexAttrib(VertexAttribType type, eastl::string_view name) : m_Name(name), m_Type(type), m_Size(VertexAttribSize(type)), m_Offset(0){};

        eastl::string_view m_Name;
        VertexAttribType m_Type;
        u32 m_Size;
        u32 m_Offset;
    };

    class InputLayout
    {
    public:
        InputLayout() = default;
        InputLayout(std::initializer_list<VertexAttrib> elements);

    public:
        u32 GetStride() const
        {
            return m_Stride;
        }

        const eastl::vector<VertexAttrib> &GetElements() const
        {
            return m_Elements;
        }

        eastl::vector<VertexAttrib>::iterator begin()
        {
            return m_Elements.begin();
        }

        eastl::vector<VertexAttrib>::iterator end()
        {
            return m_Elements.end();
        }

        eastl::vector<VertexAttrib>::const_iterator begin() const
        {
            return m_Elements.begin();
        }

        eastl::vector<VertexAttrib>::const_iterator end() const
        {
            return m_Elements.end();
        }

    private:
        eastl::vector<VertexAttrib> m_Elements = {};
        u32 m_Stride = 0;
    };

}  // namespace lr