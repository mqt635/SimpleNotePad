﻿// FormatConvertDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SimpleNotePad.h"
#include "FormatConvertDlg.h"
#include "afxdialogex.h"
#include "FilePathHelper.h"

// CFormatConvertDlg 对话框

IMPLEMENT_DYNAMIC(CFormatConvertDlg, CBaseDialog)

CFormatConvertDlg::CFormatConvertDlg(CWnd* pParent /*=NULL*/)
	: CBaseDialog(IDD_FORMAT_CONVERT_DIALOG, pParent)
{

}

CFormatConvertDlg::~CFormatConvertDlg()
{
}

void CFormatConvertDlg::LoadConfig()
{
    bool cover_original_file = (theApp.GetProfileInt(_T("format_convert"), _T("cover_original_file"), 1) != 0);
    CheckDlgButton(IDC_COVER_ORI_FILE_CHECK, cover_original_file);
}

void CFormatConvertDlg::SaveConfig() const
{
    bool cover_original_file = (IsDlgButtonChecked(IDC_COVER_ORI_FILE_CHECK) != 0);
    theApp.WriteProfileInt(_T("format_convert"), _T("cover_original_file"), cover_original_file);
}

void CFormatConvertDlg::ShowFileList()
{
	m_list_box.ResetContent();
	for (const auto& item : m_file_list)
	{
		m_list_box.AddString(item.c_str());
	}
	CCommon::AdjustListWidth(m_list_box);
}


void CFormatConvertDlg::JudgeCode()
{
	if (m_input_string.size() >= 3 && m_input_string[0] == -17 && m_input_string[1] == -69 && m_input_string[2] == -65)
		m_input_format = CodeType::UTF8;
	else if (m_input_string.size() >= 2 && m_input_string[0] == -1 && m_input_string[1] == -2)
		m_input_format = CodeType::UTF16;
	else if (m_input_string.size() >= 2 && m_input_string[0] == -2 && m_input_string[1] == -1)
		m_input_format = CodeType::UTF16BE;
	else if (CCommon::IsUTF8Bytes(m_input_string.c_str()))
		m_input_format = CodeType::UTF8_NO_BOM;
	else m_input_format = CodeType::ANSI;
}

bool CFormatConvertDlg::OpenFile(LPCTSTR file_path)
{
	m_input_string.clear();
	ifstream file{ file_path, std::ios::binary };
	if (file.fail())
	{
		CString info = CCommon::LoadTextFormat(IDS_CANNOT_OPEN_FILE_WARNING, { file_path });
		MessageBox(info, NULL, MB_OK | MB_ICONSTOP);
		return false;
	}
	while (!file.eof())
	{
		m_input_string.push_back(file.get());
	}
	m_input_string.pop_back();

	if (!m_input_auto_detect)
	{
		m_temp_string = CCommon::StrToUnicode(m_input_string, m_input_format, m_code_page);	//转换成Unicode
	}
	else		//输入编码格式为“自动”时，自动判断编码类型
	{
		JudgeCode();											//判断编码类型
		m_temp_string = CCommon::StrToUnicode(m_input_string, m_input_format, m_code_page);	//转换成Unicode
		if (m_temp_string.size() < m_input_string.size() / 4)		//如果以自动识别的格式转换成Unicode后，Unicode字符串的长度小于多字节字符串长度的1/4，则文本的编码格式可能是UTF16
		{
			m_input_format = CodeType::UTF16;
			m_temp_string = CCommon::StrToUnicode(m_input_string, m_input_format);	//重新转换成Unicode
		}
	}
	return true;
}

bool CFormatConvertDlg::SaveFile(LPCTSTR file_path)
{
	bool char_connot_convert;
	m_output_string = CCommon::UnicodeToStr(m_temp_string, char_connot_convert, m_output_format, m_code_page);
	if (char_connot_convert)	//当文件中包含Unicode字符时，提示有些字符可能无法转换
	{
		CString info;
		info.Format(CCommon::LoadText(IDS_FORMAT_UNICODE_CHARACTER_WARNING), file_path);
		MessageBox(info, NULL, MB_ICONWARNING);
	}
	ofstream file{ file_path, std::ios::binary };
	if (file.fail())
	{
		//CString info;
		//info.Format(CCommon::LoadText(IDS_FILE_CONNOT_SAVE_PATH_WARNING), file_path);
		//MessageBox(info, NULL, MB_OK | MB_ICONSTOP);
		return false;
	}
	file << m_output_string;
	return true;
}

void CFormatConvertDlg::EnableControl()
{
    bool cover_original_file = (IsDlgButtonChecked(IDC_COVER_ORI_FILE_CHECK) != 0);
    EnableDlgCtrl(IDC_FOLDER_EDIT, !cover_original_file);
    EnableDlgCtrl(IDC_BROWSE_BUTTON, !cover_original_file);
}

CString CFormatConvertDlg::GetDialogName() const
{
	return _T("FormatConvertDlg");
}



void CFormatConvertDlg::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_INPUT_COMBO, m_input_box);
    DDX_Control(pDX, IDC_OUTPUT_COMBO, m_output_box);
    DDX_Control(pDX, IDC_FILE_LIST, m_list_box);
    DDX_Control(pDX, IDC_INPUT_COMBO2, m_input_codepage_box);
}


BEGIN_MESSAGE_MAP(CFormatConvertDlg, CBaseDialog)
	ON_BN_CLICKED(IDC_ADD_BUTTON, &CFormatConvertDlg::OnBnClickedAddButton)
	ON_BN_CLICKED(IDC_REMOVE_BUTTON, &CFormatConvertDlg::OnBnClickedRemoveButton)
	ON_BN_CLICKED(IDC_CLEAR_BUTTON, &CFormatConvertDlg::OnBnClickedClearButton)
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, &CFormatConvertDlg::OnBnClickedBrowseButton)
	ON_BN_CLICKED(IDC_CONVERT_BUTTON, &CFormatConvertDlg::OnBnClickedConvertButton)
	ON_WM_DROPFILES()
    ON_BN_CLICKED(IDC_COVER_ORI_FILE_CHECK, &CFormatConvertDlg::OnBnClickedCoverOriFileCheck)
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_ADD_FOLDER_BUTTON, &CFormatConvertDlg::OnBnClickedAddFolderButton)
END_MESSAGE_MAP()


// CFormatConvertDlg 消息处理程序


BOOL CFormatConvertDlg::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
    LoadConfig();
    EnableControl();

    SetIcon(theApp.GetMenuIcon(IDI_CODE_BATCH), FALSE);

	//设置该对话框在任务栏显示
	ModifyStyleEx(0, WS_EX_APPWINDOW);

	m_input_box.AddString(CCommon::LoadText(IDS_AUTO_DETECT));
	m_input_box.AddString(_T("ANSI"));
	m_input_box.AddString(_T("UTF8"));
	m_input_box.AddString(_T("UTF16"));
	m_input_box.AddString(_T("UTF16 Big Ending"));
	m_input_box.SetCurSel(0);

	m_output_box.AddString(_T("ANSI"));
	m_output_box.AddString(CCommon::LoadText(IDS_UTF8_BOM));
    m_output_box.AddString(CCommon::LoadText(IDS_UTF8_NO_BOM));
	m_output_box.AddString(_T("UTF16"));
	m_output_box.AddString(_T("UTF16 Big Ending"));
	m_output_box.SetCurSel(1);

    for (size_t i{}; i < CONST_VAL->CodePageList().size() - 1; i++)
    {
        m_input_codepage_box.AddString(CONST_VAL->CodePageList()[i].name);
    }
    if (!CONST_VAL->CodePageList().empty())
        m_input_codepage_box.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CFormatConvertDlg::OnBnClickedAddButton()
{
	// TODO: 在此添加控件通知处理程序代码
	//设置过滤器
	CString szFilter = CCommon::LoadText(IDS_FILE_OPEN_FILTER);
	//构造打开文件对话框
	CFileDialog fileDlg(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT, szFilter, this);
	//显示打开文件对话框
	if (IDOK == fileDlg.DoModal())
	{
		POSITION posFile = fileDlg.GetStartPosition();
		while (posFile != NULL)
		{
			m_file_list.push_back(fileDlg.GetNextPathName(posFile).GetString());
		}
		ShowFileList();
	}
}


void CFormatConvertDlg::OnBnClickedRemoveButton()
{
	// TODO: 在此添加控件通知处理程序代码
	int select;
	select = m_list_box.GetCurSel();		//获取当前选中项序号
	if (select >= 0 && select < m_file_list.size())
	{
		m_file_list.erase(m_file_list.begin() + select);	//删除选中项
		ShowFileList();
	}
}


void CFormatConvertDlg::OnBnClickedClearButton()
{
	// TODO: 在此添加控件通知处理程序代码
	m_file_list.clear();
	ShowFileList();
}


void CFormatConvertDlg::OnBnClickedBrowseButton()
{
	// TODO: 在此添加控件通知处理程序代码
	CFolderPickerDialog folderPickerDlg;
	if (folderPickerDlg.DoModal() == IDOK)
	{
		m_output_path = folderPickerDlg.GetPathName();
		SetDlgItemText(IDC_FOLDER_EDIT, m_output_path.c_str());
	}
}


bool CFormatConvertDlg::ConvertSingleFile(const std::wstring& file_path)
{
    if (!OpenFile(file_path.c_str()))
        return false;		//如果当前文件无法打开，就跳过它

    wstring file_name = CFilePathHelper(file_path).GetFileName();
    wstring dest_file_path;
    if (IsDlgButtonChecked(IDC_COVER_ORI_FILE_CHECK) != 0)      //覆盖源文件
    {
        dest_file_path = file_path;
    }
    else
    {
        dest_file_path = m_output_path + file_name;
    }
    return SaveFile(dest_file_path.c_str());
}


void CFormatConvertDlg::OnBnClickedConvertButton()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_file_list.empty())
	{
		MessageBox(CCommon::LoadText(IDS_ADD_CONVERT_FILE_WARNING), NULL, MB_ICONWARNING);
		return;
	}
	if (IsDlgButtonChecked(IDC_COVER_ORI_FILE_CHECK) == 0 && m_output_path.empty())
	{
		MessageBox(CCommon::LoadText(IDS_SELECT_OUTPUT_FOLDER_WARNING), NULL, MB_ICONWARNING);
		return;
	}

	CWaitCursor wait_cursor;
	switch (m_output_box.GetCurSel())
	{
	case 0: m_output_format = CodeType::ANSI; break;
	case 1: m_output_format = CodeType::UTF8; break;
    case 2: m_output_format = CodeType::UTF8_NO_BOM; break;
	case 3: m_output_format = CodeType::UTF16; break;
	case 4: m_output_format = CodeType::UTF16BE; break;
	default:
		break;
	}

    switch (m_input_box.GetCurSel())
    {
    case 0: m_input_format = CodeType::AUTO; break;
    case 1: m_input_format = CodeType::ANSI; break;
    case 2: m_input_format = CodeType::UTF8; break;
    case 3: m_input_format = CodeType::UTF16; break;
    case 4: m_input_format = CodeType::UTF16BE; break;
    default: break;
    }
    m_input_auto_detect = (m_input_box.GetCurSel() == 0);

    m_code_page = 0;
    int index = m_input_codepage_box.GetCurSel();
    if (index >= 0 && index < static_cast<int>(CONST_VAL->CodePageList().size()))
        m_code_page = CONST_VAL->CodePageList()[index].code_page;

    std::set<std::wstring> convert_file_list;        //所有要转换的文件的路径
	for (const auto& item : m_file_list)
	{
        //判断列表中的项目是文件还是文件夹
        if (CCommon::IsFolder(item))
        {
            //遍历文件夹下所有文件
            std::vector<std::wstring> files;
            CCommon::GetFiles((item + L"*.*").c_str(), files);
            for (const auto& file_name : files)
            {
                convert_file_list.insert(item + file_name);
            }
        }
        else if (CCommon::IsFile(item))
        {
            convert_file_list.insert(item);
        }
	}

	int convert_cnt{};
    for (const auto& file_path : convert_file_list)
    {
        if (ConvertSingleFile(file_path))
            convert_cnt++;
    }

	CString info = CCommon::LoadTextFormat(IDS_CONVERT_FINISH_INFO, { convert_file_list.size(), convert_cnt, convert_file_list.size() - convert_cnt });
	MessageBox(info, NULL, MB_ICONINFORMATION);
}


void CFormatConvertDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	TCHAR file_path[MAX_PATH];
	int drop_count = DragQueryFile(hDropInfo, -1, NULL, 0);		//取得被拖动文件的数目
	for (int i{}; i < drop_count; i++)
	{
		DragQueryFile(hDropInfo, i, file_path, MAX_PATH);
		m_file_list.emplace_back(file_path);
	}
	ShowFileList();
	CBaseDialog::OnDropFiles(hDropInfo);
}


void CFormatConvertDlg::OnBnClickedCoverOriFileCheck()
{
    EnableControl();
}


void CFormatConvertDlg::OnDestroy()
{
    SaveConfig();

    CBaseDialog::OnDestroy();
}


void CFormatConvertDlg::OnBnClickedAddFolderButton()
{
    //构造打开文件对话框
    CFolderPickerDialog fileDlg(nullptr, 0, this);
    //显示打开文件对话框
    if (IDOK == fileDlg.DoModal())
    {
        std::wstring folder_path = fileDlg.GetPathName().GetString();
        folder_path.push_back(L'\\');
        m_file_list.push_back(folder_path);
        ShowFileList();
    }
}
