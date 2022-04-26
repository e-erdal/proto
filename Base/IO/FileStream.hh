//
// Created on August 15th 2021 by e-erdal.
//

#pragma once

namespace lr
{
    class FileStream
    {
    public:
        FileStream() = default;
        FileStream(eastl::string_view path, bool write);
        void Reopen(eastl::string_view path, bool write);

        void Close();

        /// STL FUNCTIONS DON'T OWN MEMORY, YOU NEED TO FREE IT YOURSELF
        /// actually implemented a special function for reading string
        template<typename T>
        inline T *ReadAll()
        {
            GetSize();

            T *pBuffer = (T *)malloc(m_FileSize);
            fread(pBuffer, 1, m_FileSize, m_File);
            return pBuffer;
        }

        template<typename T>
        inline T *ReadPtr(size_t size = 0)
        {
            T *pBuffer = nullptr;

            if (size == 0)
                fread(pBuffer, sizeof(T), 1, m_File);
            else
                fread(pBuffer, size, 1, m_File);

            return pBuffer;
        }

        template<typename T>
        inline T &Read(u32 size = 0)
        {
            T *buffer;
            fread(buffer, (size == 0 ? sizeof(T) : size), 1, m_File);
            return *buffer;
        }

        inline eastl::string ReadString(u32 size)
        {
            return eastl::string(ReadPtr<char>(size), size);
        }

        template<typename T>
        inline void Write(const T &t, u32 size = 0)
        {
            if (size > 0)
                fwrite(&t, 1, size, m_File);
            else
                fwrite(&t, 1, sizeof(T), m_File);
        }

        void WritePtr(const u8 *t, u32 size = 0);
        void WriteString(eastl::string_view val);

    private:
        void GetSize();

    public:
        u32 Size()
        {
            return m_FileSize;
        }

        bool IsOK()
        {
            return m_File;
        }

    private:
        FILE *m_File = 0;
        u32 m_FileSize = 0;
    };

}  // namespace lr