#include "RenderTargetView.hh"

#include "D3D11Texture.hh"

namespace lr
{
    RenderTargetView &RenderTargetView::AttachColor(Texture *pTexture)
    {
        m_ColorAttachments[m_ColorAttachmentCount++] = pTexture;

        return *this;
    }

    RenderTargetView &RenderTargetView::AttachDepth(Texture *pDepthTexture)
    {
        m_pDepthAttachment = pDepthTexture;

        return *this;
    }

    void RenderTargetView::Reset(bool deleteAttachments, bool deleteDepthAttachment)
    {
        if (deleteAttachments)
        {
            for (u32 i = 0; i < m_ColorAttachmentCount; i++)
            {
                delete m_ColorAttachments[i];
            }
        }

        if (deleteDepthAttachment)
        {
            delete m_pDepthAttachment;
        }

        memset(m_ColorAttachments.data(), 0, m_ColorAttachments.count * sizeof(void *));
        m_ColorAttachmentCount = 0;
        m_pDepthAttachment = nullptr;
    }

}  // namespace lr