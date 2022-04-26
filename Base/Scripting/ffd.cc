#include "ffd.hh"
#include "IO/FileStream.hh"

#include "ffd/ffd.skeleton.hh"
#include "ffd/ffd.lexer.hh"

static eastl::vector<lr::ffd::Category *> pCategories;
static eastl::string CurrentVar;
static eastl::string CurrentArray;

void ffd_push_category(char *var)
{
    auto categoryIt = pCategories.back()->m_Childeren.emplace(eastl::string(var), new lr::ffd::Category);
    pCategories.push_back(categoryIt.first->second);

    free(var);
}

void ffd_pop_category()
{
    pCategories.pop_back();
}

void ffd_push_array(char *var)
{
    CurrentArray = var;

    free(var);
}

void ffd_pop_array()
{
    CurrentArray = "";
}

void ffd_array_str(char *val)
{
    lr::ffd::Category *pCurrentCategory = pCategories.back();
    auto &strArr = pCurrentCategory->m_ArrayString[CurrentArray];

    eastl::string actualString = val;
    strArr.push_back(actualString.substr(1, actualString.length() - 2));

    free(val);
}

void ffd_array_num(double val)
{
    lr::ffd::Category *pCurrentCategory = pCategories.back();
    auto &numArr = pCurrentCategory->m_ArrayNumbers[CurrentArray];
    numArr.push_back(val);
}

void ffd_var(char *var)
{
    CurrentVar = var;
    free(var);
}

void ffd_num(double num)
{
    lr::ffd::Category *pCurrentCategory = pCategories.back();
    pCurrentCategory->m_Numbers.emplace(CurrentVar, num);

    CurrentVar = "";
}

void ffd_str(char *str)
{
    lr::ffd::Category *pCurrentCategory = pCategories.back();

    eastl::string actualString = str;
    pCurrentCategory->m_Strings.emplace(CurrentVar, actualString.substr(1, actualString.length() - 2));  // Remove "<str>"

    free(str);

    CurrentVar = "";
}

void ffd_bool(bool b)
{
    lr::ffd::Category *pCurrentCategory = pCategories.back();
    pCurrentCategory->m_Bools.emplace(CurrentVar, b);

    CurrentVar = "";
}

int yyerror(const char *s)
{
    char tmp_str[1024];
    sprintf_s(tmp_str, sizeof(tmp_str), "%s at line %i", s, yylineno);
    return 1;
}

namespace lr
{

    static ffd::Category kInvalidCat;

    template<typename Type, typename Map>
    static Type GetValueFromVar(Map &&map, const eastl::string &var)
    {
        auto valIt = map.find(var);
        if (valIt == map.end())
        {
            return Type{};
        }

        return valIt->second;
    }

    static void DeleteCategoryRecursive(ffd::Category *pStartCategory)
    {
        for (auto &v : pStartCategory->m_Childeren)
        {
            DeleteCategoryRecursive(v.second);

            delete v.second;
        }
    }

    static void PrintDepth(u32 depth)
    {
        eastl::string s = "";
        for (u32 i = 0; i < depth; i++) s += "    ";

        printf("%s", s.c_str());
    }

    static void PrintChildRecursive(ffd::Category *pCategory, u32 depth)
    {
        for (auto &v : pCategory->m_Strings)
        {
            PrintDepth(depth);
            printf("%s = %s\n", v.first.c_str(), v.second.c_str());
        }

        for (auto &v : pCategory->m_Numbers)
        {
            PrintDepth(depth);
            printf("%s = %f\n", v.first.c_str(), v.second);
        }

        for (auto &v : pCategory->m_Bools)
        {
            PrintDepth(depth);
            printf("%s = %s\n", v.first.c_str(), v.second ? "true" : "false");
        }

        for (auto &v : pCategory->m_ArrayString)
        {
            PrintDepth(depth);
            printf("%s = [\n", v.first.c_str());
            for (auto &e : v.second)
            {
                PrintDepth(depth + 1);
                printf("%s\n", e.c_str());
            }
            PrintDepth(depth);
            printf("]\n");
        }

        for (auto &v : pCategory->m_ArrayNumbers)
        {
            PrintDepth(depth);
            printf("%s = [\n", v.first.c_str());
            for (auto &e : v.second)
            {
                PrintDepth(depth + 1);
                printf("%lf\n", e);
            }
            PrintDepth(depth);
            printf("]\n");
        }

        for (auto &v : pCategory->m_Childeren)
        {
            PrintDepth(depth);

            printf("%s:\n", v.first.c_str());
            PrintChildRecursive(v.second, depth + 1);
        }
    }

    static void WriteChildRecursive(FileStream &fs, ffd::Category *pCategory, u32 depth)
    {
        for (auto &v : pCategory->m_Strings)
        {
            for (u32 i = 0; i < depth; i++) fs.WriteString("\t");
            fs.WriteString(Format("{} = \"{}\"\n", v.first.c_str(), v.second.c_str()));
        }

        for (auto &v : pCategory->m_Numbers)
        {
            for (u32 i = 0; i < depth; i++) fs.WriteString("\t");
            fs.WriteString(Format("{} = {}\n", v.first.c_str(), v.second));
        }

        for (auto &v : pCategory->m_Bools)
        {
            for (u32 i = 0; i < depth; i++) fs.WriteString("\t");
            fs.WriteString(Format("{} = {}\n", v.first.c_str(), v.second ? "true" : "false"));
        }

        for (auto &v : pCategory->m_Childeren)
        {
            for (u32 i = 0; i < depth; i++) fs.WriteString("\t");
            fs.WriteString(Format("{}\n", v.first.c_str()));
            for (u32 i = 0; i < depth; i++) fs.WriteString("\t");
            fs.WriteString("{\n");

            WriteChildRecursive(fs, v.second, depth + 1);

            for (u32 i = 0; i < depth; i++) fs.WriteString("\t");
            fs.WriteString("}\n");
        }
    }

    eastl::string &ffd::Category::AsString(const eastl::string &var, u32 arrayIdx)
    {
        if (arrayIdx == -1)
        {
            static eastl::string kEmptyString = "";

            auto valIt = m_Strings.find(var);
            if (m_Strings.find(var) == m_Strings.end())
            {
                return kEmptyString;
            }

            return valIt->second;
        }

        return m_ArrayString[var][arrayIdx];
    }

    u32 ffd::Category::AsU32(const eastl::string &var, u32 arrayIdx)
    {
        if (arrayIdx == -1)
        {
            return GetValueFromVar<u32>(m_Numbers, var);
        }

        return (u32)m_ArrayNumbers[var][arrayIdx];
    }

    i32 ffd::Category::AsI32(const eastl::string &var, u32 arrayIdx)
    {
        if (arrayIdx == -1)
        {
            return GetValueFromVar<i32>(m_Numbers, var);
        }

        return (i32)m_ArrayNumbers[var][arrayIdx];
    }

    float ffd::Category::AsFloat(const eastl::string &var, u32 arrayIdx)
    {
        if (arrayIdx == -1)
        {
            return GetValueFromVar<float>(m_Numbers, var);
        }

        return (float)m_ArrayNumbers[var][arrayIdx];
    }

    bool ffd::Category::AsBool(const eastl::string &var)
    {
        return GetValueFromVar<bool>(m_Numbers, var);
    }

    u32 ffd::Category::GetStringArraySize(const eastl::string &var)
    {
        auto sizeIt = m_ArrayString.find(var);
        return (sizeIt != m_ArrayString.end()) ? sizeIt->second.size() : 0;
    }

    u32 ffd::Category::GetNumberArraySize(const eastl::string &var)
    {
        auto sizeIt = m_ArrayNumbers.find(var);
        return (sizeIt != m_ArrayNumbers.end()) ? sizeIt->second.size() : 0;
    }

    void ffd::Category::SetString(const eastl::string &var, const eastl::string &val)
    {
        m_Strings[var] = val;
    }

    void ffd::Category::SetU32(const eastl::string &var, u32 val)
    {
        m_Numbers[var] = val;
    }

    void ffd::Category::SetI32(const eastl::string &var, i32 val)
    {
        m_Numbers[var] = val;
    }

    void ffd::Category::SetFloat(const eastl::string &var, float val)
    {
        m_Numbers[var] = val;
    }

    void ffd::Category::SetBool(const eastl::string &var, bool val)
    {
        m_Bools[var] = val;
    }

    ffd::Category &ffd::Category::operator[](const eastl::string &var)
    {
        auto categoryIt = m_Childeren.find(var);
        if (categoryIt != m_Childeren.end())
        {
            return *categoryIt->second;
        }

        LOG_ERROR("Requested category '{}' does not exist.", var.c_str());

        return kInvalidCat;
    }

    ffd::ffd()
    {
        pCategories.push_back(&m_GlobalCategory);
    }

    ffd::~ffd()
    {
        DeleteCategoryRecursive(&m_GlobalCategory);
    }

    void ffd::FromMemory(const char *pCode, u32 len)
    {
        YY_BUFFER_STATE state = yy_scan_bytes(pCode, len);
        yyparse();
        yylex_destroy();

        // Ignore m_GlobalCategory, so i = 1
        for (u32 i = 1; i < pCategories.size(); i++)
        {
            delete pCategories[i];
        }

        pCategories.clear();
        CurrentVar = "";
    }

    void ffd::FromFile(const eastl::string &path)
    {
        FileStream scriptFile(path, false);
        if (!scriptFile.IsOK())
        {
            LOG_ERROR("Failed to load '{}'.", path.c_str());
            return;
        }

        const char *pScript = scriptFile.ReadAll<char>();
        u32 size = scriptFile.Size();
        scriptFile.Close();

        FromMemory(pScript, size);

        free((void *)pScript);
    }

    void ffd::Close(const eastl::string &path)
    {
        if (path == "") return;

        FileStream fs(path, true);
        WriteChildRecursive(fs, &m_GlobalCategory, 0);
        fs.Close();
    }

    void ffd::Print()
    {
        PrintChildRecursive(&m_GlobalCategory, 0);
    }

    ffd::Category &ffd::Global()
    {
        return m_GlobalCategory;
    }

    ffd::Category &ffd::operator[](const eastl::string &var)
    {
        auto categoryIt = m_GlobalCategory.m_Childeren.find(var);
        if (categoryIt != m_GlobalCategory.m_Childeren.end())
        {
            return *categoryIt->second;
        }

        LOG_ERROR("Requested category '{}' does not exist.", var.c_str());

        return kInvalidCat;
    }

}  // namespace lr
