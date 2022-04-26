//
// Created on Saturday 23rd April 2022 by e-erdal
//

#pragma once

namespace lr
{
    enum class APIFlags : u32
    {
        None,

        VSync = 1 << 0,

        /// PLEASE DONT USE MSAA
        MSAAX2 = 1 << 1,
        MSAAX4 = 1 << 2,
        MSAAX8 = 1 << 3,
    };
    BitFlags(APIFlags);

    enum class PrimitiveType
    {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
        Patch_3
    };

    enum class DepthFunc : u8
    {
        Never = 1,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always
    };

    enum class Cull : u8
    {
        None = 1,
        Front,
        Back,
    };

    enum class TextureFiltering : u8
    {
        Point = 0,
        Linear,
        Ansio,

        Count
    };

    enum class TextureAddress : u8
    {
        Wrap,
        Mirror,
        Clamp,
        Border,
    };

}  // namespace lr