//
// Created on August 12th 2021 by e-erdal.
//

#pragma once

namespace lr
{
    struct BufferStreamMemoyWatcher
    {
    public:
        BufferStreamMemoyWatcher(bool enable) : m_Log(enable)
        {
        }

        void Allocated(size_t size);
        void Deallocated(size_t size);

        bool m_Log = true;
        size_t m_TotalSize = 0;
    };

    extern BufferStreamMemoyWatcher *g_pBSWatcher;

    class FileStream;
    class BufferStream
    {
    public:
        BufferStream() = default;

        BufferStream(size_t dataLen);
        BufferStream(u8 *pData, size_t dataLen);
        BufferStream(eastl::vector<u8> &data);
        BufferStream(FileStream &fileStream);

        ~BufferStream();

    public:
        void Reset();
        void Reset(size_t dataLen);
        void Reset(u8 *pData, size_t dataLen);
        void Reset(eastl::vector<u8> &data);
        void Reset(BufferStream &data);

        u8 *GetOffsetPtr();

        void Dump();
        void Seek(u8 seekTo, intptr_t pos);
        void Expand(size_t len);

        void Assign(void *pData, size_t dataLen, u32 count = 1);
        void AssignZero(size_t dataSize);
        void AssignString(const eastl::string &val);

        void Insert(void *pData, size_t dataLen, u32 count = 1);
        void InsertZero(size_t dataLen);
        void InsertString(const eastl::string &val);

        void *Get(size_t dataLen, u32 count = 1);
        void *GetNew(size_t dataLen, u32 count = 1);
        eastl::string GetString(size_t dataSize);

        void StartOver();

        void operator=(FileStream &fs);

        template<typename T>
        void Assign(T &&val)
        {
            memcpy(m_pData + m_Offset, &val, sizeof(T));
            m_Offset += sizeof(T);
        }

        template<typename T>
        void AssignString(const eastl::string &val)
        {
            size_t len = val.length();
            Assign(&len, sizeof(T));

            if (val.length()) Assign((void *)val.data(), val.length());
        }

        template<typename T>
        void Insert(T &&val)
        {
            Expand(sizeof(T));

            memcpy(m_pData + m_Offset, (u8 *)&val, sizeof(T));
            m_Offset += sizeof(T);
        }
        template<typename T>
        void InsertString(const eastl::string &val)
        {
            size_t len = val.length();

            Expand(val.length() + sizeof(T));
            Assign(&len, sizeof(T));

            if (val.length()) Assign((void *)val.data(), val.length());
        }

        template<typename T>
        T &Get()
        {
            T *pData = (T *)(m_pData + m_Offset);
            m_Offset += sizeof(T);
            return *pData;
        }

        template<typename T>
        eastl::string GetString()
        {
            u32 strLen = *(T *)Get(sizeof(T));

            return GetString(strLen);
        }

    public:
        inline u8 *GetData()
        {
            return m_pData;
        }

        inline const size_t &GetSize() const
        {
            return m_DataLen;
        }

        inline const uintptr_t &GetOffset() const
        {
            return m_Offset;
        }

    private:
        u8 *m_pData = 0;
        size_t m_DataLen = 0;

        uintptr_t m_Offset = 0;
    };

}  // namespace lr