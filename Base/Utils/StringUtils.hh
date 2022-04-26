//
// Created on Friday 11th March 2022 by e-erdal
//

#pragma once

namespace lr
{
    class StringUtils
    {
    public:
        StringUtils(eastl::string &target);

        StringUtils &Trim(const char *pSearch);
        StringUtils &ReplaceAll(const eastl::string &from, const eastl::string &to);
        StringUtils &Lowercase();
        StringUtils &Uppercase();
        StringUtils &Normalize(); // Converts string to something we can understand

        eastl::string GetFileName(char token = '/');
        eastl::string GetExtension();

    private:
        eastl::string &m_Target;
    };

}  // namespace lr