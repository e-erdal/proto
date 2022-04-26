//
// Created by Mempler
//

#pragma once

namespace lr
{
    class ffd
    {
    public:
        struct Category
        {
            eastl::string &AsString(const eastl::string &var, u32 arrayIdx = -1);
            u32 AsU32(const eastl::string &var, u32 arrayIdx = -1);
            i32 AsI32(const eastl::string &var, u32 arrayIdx = -1);
            float AsFloat(const eastl::string &var, u32 arrayIdx = -1);
            bool AsBool(const eastl::string &var);

            u32 GetStringArraySize(const eastl::string &var);
            u32 GetNumberArraySize(const eastl::string &var);

            void SetString(const eastl::string &var, const eastl::string &val);
            void SetU32(const eastl::string &var, u32 val);
            void SetI32(const eastl::string &var, i32 val);
            void SetFloat(const eastl::string &var, float val);
            void SetBool(const eastl::string &var, bool val);

            Category &operator[](const eastl::string &var);

            eastl::unordered_map<eastl::string, double> m_Numbers;
            eastl::unordered_map<eastl::string, eastl::string> m_Strings;
            eastl::unordered_map<eastl::string, bool> m_Bools;

            eastl::unordered_map<eastl::string, eastl::vector<eastl::string>> m_ArrayString;
            eastl::unordered_map<eastl::string, eastl::vector<double>> m_ArrayNumbers;

            eastl::unordered_map<eastl::string, Category *> m_Childeren;
        };

    public:
        ffd();
        ~ffd();

        void FromMemory(const char *pCode, u32 len);
        void FromFile(const eastl::string &path);
        void Close(const eastl::string &path = "");

        void Print();

        Category &Global();
        Category &operator[](const eastl::string &var);

    private:
        Category m_GlobalCategory;
    };
}  // namespace lr
