#include "StringUtils.hh"

namespace lr
{
    StringUtils::StringUtils(eastl::string &target) : m_Target(target)
    {
    }

    StringUtils &StringUtils::Trim(const char *pSearch)
    {
        m_Target.erase(0, m_Target.find_first_not_of(pSearch));
        m_Target.erase(m_Target.find_last_not_of(pSearch) + 1);

        return *this;
    }

    StringUtils &StringUtils::ReplaceAll(const eastl::string &from, const eastl::string &to)
    {
        /// Probably expensive for bigger blobs of strings.

        u32 pos;
        while ((pos = m_Target.find(from)) != -1)
        {
            m_Target.replace(pos, from.length(), to);
            pos += to.length();
        }

        return *this;
    }

    StringUtils &StringUtils::Lowercase()
    {
        eastl::transform(m_Target.begin(), m_Target.end(), m_Target.begin(), ::tolower);

        return *this;
    }

    StringUtils &StringUtils::Uppercase()
    {
        eastl::transform(m_Target.begin(), m_Target.end(), m_Target.begin(), ::toupper);

        return *this;
    }

    StringUtils &StringUtils::Normalize()
    {
        ReplaceAll("\\\\", "/");
        ReplaceAll("\\", "/");
        ReplaceAll("//", "/");
        Lowercase();
        
        return *this;
    }

    eastl::string StringUtils::GetFileName(char token)
    {
        u32 extensionPos = m_Target.rfind('.');
        if (extensionPos == -1)
        {
            extensionPos = m_Target.length();
        }

        u32 namePos = m_Target.rfind(token);
        namePos++;

        return m_Target.substr(namePos, extensionPos - namePos);
    }

    eastl::string StringUtils::GetExtension()
    {
        u32 extensionPos = m_Target.rfind('.');
        if (extensionPos == -1) return "";

        return m_Target.substr(extensionPos + 1, m_Target.length());
    }

}  // namespace lr