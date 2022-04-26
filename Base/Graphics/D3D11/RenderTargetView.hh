//
// Created on Sunday 30th January 2022 by e-erdal
//

#pragma once

namespace lr
{
    /// OpenGL like "FrameBuffer" handle
    class Texture;
    struct RenderTargetView
    {
        RenderTargetView &AttachColor(Texture *pTexture);
        RenderTargetView &AttachDepth(Texture *pDepthTexture);  // Note: This function will discard old depth attachment

        void Reset(bool deleteAttachments, bool deleteDepthAttachment);

        u32 m_Width = 0;   // Width and Height values must be set by GAPI when creating it
        u32 m_Height = 0;  //

        u32 m_ColorAttachmentCount = 0;
        eastl::array<Texture *, 8> m_ColorAttachments;
        Texture *m_pDepthAttachment = nullptr;
    };

}  // namespace lr