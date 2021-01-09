﻿#pragma once
#include "stdafx.h"
#include "tinyxml2.h"

class CLanguage
{
public:
    struct SyntaxColor
    {
        std::wstring name;
        int id;
        COLORREF color;
    };
    std::wstring m_name;
    int m_id;
    std::set<wstring> m_ext;
    std::map<int, std::string> m_keywords_list;
    std::vector<SyntaxColor> m_syntax_list;

    //从配置文件“Language”节点解析语法高亮信息
    //ele: xml节点
    //syntax_from: 如果语法高亮颜色设置要从其他语言复制，则将该语言的名称保存在这里
    void FromXmlElement(tinyxml2::XMLElement* ele, wstring& syntax_from);
};

class CSyntaxHighlight
{
public:
    void LoadFromFile(const char* file_path);

    CLanguage FindLanguageByExt(const wchar_t* ext);
    CLanguage FindLanguageByName(const wchar_t* name);
    CLanguage GetLanguage(int index);
    int IndexOf(const wstring& name);

    const std::vector<CLanguage>& GetLanguageList();

private:
    std::vector<CLanguage> m_language_list;
};

