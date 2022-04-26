#include "FileStream.hh"

#include "BufferStream.hh"

namespace lr
{
    FileStream::FileStream(eastl::string_view path, bool write)
    {
        if (write)
        {
            m_File = fopen(path.data(), "wb");
        }
        else
        {
            m_File = fopen(path.data(), "rb");
        }

        if (!IsOK()) return;

        GetSize();
    }

    void FileStream::Reopen(eastl::string_view path, bool write)
    {
        if (IsOK()) Close();

        if (write)
        {
            m_File = fopen(path.data(), "wb");
        }
        else
        {
            m_File = fopen(path.data(), "rb");
        }

        if (!IsOK()) return;

        GetSize();
    }

    void FileStream::Close()
    {
        fclose(m_File);
    }

    void FileStream::WritePtr(const u8 *t, u32 size)
    {
        fwrite(t, 1, size, m_File);
    }

    void FileStream::WriteString(eastl::string_view val)
    {
        fwrite(val.data(), 1, val.length(), m_File);
    }

    void FileStream::GetSize()
    {
        if (m_FileSize > 0) return;

        fseek(m_File, 0, SEEK_END);
        m_FileSize = ftell(m_File);
        rewind(m_File);
    }

}  // namespace lr