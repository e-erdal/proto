#include "BufferStream.hh"
#include "FileStream.hh"

#include <bx/string.h>

namespace lr
{
    void BufferStreamMemoyWatcher::Allocated(size_t size)
    {
        if (m_Log)
        {
            m_TotalSize += size;

            char prettifySize[16];
            bx::prettify(prettifySize, BX_COUNTOF(prettifySize), size);

            char prettifyTotalSize[16];
            bx::prettify(prettifyTotalSize, BX_COUNTOF(prettifyTotalSize), m_TotalSize);

            LOG_TRACE("BufferStream allocated {}, total size is now {}.", prettifySize, prettifyTotalSize);
        }
    }

    void BufferStreamMemoyWatcher::Deallocated(size_t size)
    {
        if (m_Log)
        {
            m_TotalSize -= size;

            char prettifySize[16];
            bx::prettify(prettifySize, BX_COUNTOF(prettifySize), size);

            char prettifyTotalSize[16];
            bx::prettify(prettifyTotalSize, BX_COUNTOF(prettifyTotalSize), m_TotalSize);

            LOG_TRACE("BufferStream deallocated {}, total size is now {}.", prettifySize, prettifyTotalSize);
        }
    }

    BufferStream::BufferStream(size_t dataLen)
    {
        InsertZero(dataLen);
        StartOver();
    }

    BufferStream::BufferStream(u8 *pData, size_t dataLen)
    {
        Expand(dataLen);
        Assign(pData, dataLen);
        StartOver();
    }

    BufferStream::BufferStream(eastl::vector<u8> &data)
    {
        Expand(data.size());
        Assign(&data[0], data.size());
        StartOver();
    }

    BufferStream::BufferStream(FileStream &fileStream)
    {
        *this = fileStream;
    }

    BufferStream::~BufferStream()
    {
        g_pBSWatcher->Deallocated(m_DataLen);

        SAFE_FREE(m_pData);
    }

    void BufferStream::Reset()
    {
        g_pBSWatcher->Deallocated(m_DataLen);

        SAFE_FREE(m_pData);
        m_DataLen = 0;

        StartOver();
    }

    void BufferStream::Reset(size_t dataLen)
    {
        g_pBSWatcher->Deallocated(m_DataLen);

        SAFE_FREE(m_pData);
        m_DataLen = 0;

        InsertZero(dataLen);
        StartOver();
    }

    void BufferStream::Reset(u8 *pData, size_t dataLen)
    {
        g_pBSWatcher->Deallocated(m_DataLen);

        SAFE_FREE(m_pData);
        m_DataLen = 0;

        Expand(dataLen);
        Assign(pData, dataLen);
        StartOver();
    }

    void BufferStream::Reset(eastl::vector<u8> &data)
    {
        g_pBSWatcher->Deallocated(m_DataLen);

        SAFE_FREE(m_pData);
        m_DataLen = 0;

        Expand(data.size());
        Assign(&data[0], data.size());
        StartOver();
    }

    void BufferStream::Reset(BufferStream &data)
    {
        g_pBSWatcher->Deallocated(m_DataLen);

        SAFE_FREE(m_pData);
        m_DataLen = 0;

        Expand(data.m_DataLen);
        Assign(data.m_pData, data.m_DataLen);

        StartOver();
    }

    u8 *BufferStream::GetOffsetPtr()
    {
        return (m_pData + m_Offset);
    }

    void BufferStream::Dump()
    {
        printf("========= BufferStream memory dump =========\n");

        for (size_t i = 0; i < m_DataLen; i++)
        {
            printf("%x ", m_pData[i]);
        }

        printf("\n");
    }

    void BufferStream::Seek(u8 seekTo, intptr_t pos)
    {
        switch (seekTo)
        {
            case SEEK_END: m_Offset = m_DataLen - pos; break;
            case SEEK_SET: m_Offset = pos; break;
            case SEEK_CUR: m_Offset += pos; break;
            default: break;
        }

        if (m_Offset < 0)
            m_Offset = 0;
        else if (m_Offset > m_DataLen)
            m_Offset = m_DataLen;
    }

    void BufferStream::Expand(size_t len)
    {
        g_pBSWatcher->Allocated(len);
        m_DataLen += len;
        m_pData = (u8 *)realloc(m_pData, m_DataLen);
        _ZEROM((m_pData + m_Offset), len);
    }

    void BufferStream::Assign(void *pData, size_t dataLen, u32 count)
    {
        assert(m_pData != NULL);               // Our data has to be valid
        assert(m_DataLen > 0);                 // Our data has to be allocated
        assert(pData != NULL);                 // Data has to be vaild
        assert(m_DataLen >= dataLen * count);  // Input data cannot be larger than data we have

        memcpy(m_pData + m_Offset, pData, dataLen * count);
        m_Offset += dataLen * count;
    }

    void BufferStream::AssignZero(size_t dataSize)
    {
        memset(m_pData + m_Offset, 0, dataSize);
        m_Offset += dataSize;
    }

    void BufferStream::AssignString(const eastl::string &val)
    {
        if (val.length()) Assign((void *)val.data(), val.length());
    }

    void BufferStream::Insert(void *pData, size_t dataLen, u32 count)
    {
        Expand(dataLen);

        memcpy(m_pData + m_Offset, pData, dataLen * count);
        m_Offset += dataLen * count;
    }

    void BufferStream::InsertZero(size_t dataLen)
    {
        Expand(dataLen);

        memset(m_pData + m_Offset, 0, dataLen);
        m_Offset += dataLen;
    }

    void BufferStream::InsertString(const eastl::string &val)
    {
        size_t len = val.length();

        Expand(val.length());

        if (val.length()) Assign((void *)val.data(), val.length());
    }

    void *BufferStream::Get(size_t dataLen, u32 count)
    {
        void *pData = m_pData + m_Offset;
        m_Offset += dataLen * count;
        return pData;
    }

    void *BufferStream::GetNew(size_t dataLen, u32 count)
    {
        void *pData = malloc(dataLen * count);
        memcpy(pData, m_pData + m_Offset, dataLen * count);
        m_Offset += dataLen * count;
        return pData;
    }

    eastl::string BufferStream::GetString(size_t dataSize)
    {
        char *data = (char *)(m_pData + m_Offset);
        m_Offset += dataSize;

        return eastl::string(data, dataSize);
    }

    void BufferStream::StartOver()
    {
        m_Offset = 0;
    }

    void BufferStream::operator=(FileStream &fs)
    {
        Reset();

        m_pData = fs.ReadAll<u8>();
        m_DataLen = fs.Size();

        g_pBSWatcher->Allocated(m_DataLen);
    }

}  // namespace lr