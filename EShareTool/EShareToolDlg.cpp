
// EShareToolDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "EShareTool.h"
#include "EShareToolDlg.h"
#include "afxdialogex.h"

#include "EShareToolWin7.h"

#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>

#include <afxinet.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString SubKey;


bool isfrozen = false;
bool isfirst = true;
bool isinit = true;


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CEShareToolDlg 对话框



CEShareToolDlg::CEShareToolDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ESHARETOOL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEShareToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEShareToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_ShareSwitch, &CEShareToolDlg::OnBnClickedShareswitch)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CEShareToolDlg 消息处理程序

BOOL CEShareToolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	CAboutDlg showAboutDlg;
	showAboutDlg.DoModal();
	
	if (ifWin10() == TRUE)
	{
		
	}
	else if (ifWin7() == TRUE)
	{
		EShareToolWin7 dialog;
		dialog.DoModal();
		CDialog::OnOK();
		return true;
	}
	else
	{
		AfxMessageBox(L"不支持的系统版本!\r\n目前仅支持 Windows 7 和 Windows 10 1903 及以上版本.");
		CDialog::OnOK();
		return true;
	}

	MessageBox(L"本程序仅适合通过无线网卡开启热点共享校园网!\r\n若您的电脑没有无线网卡,则本程序为摆设.");

	HKEY hTestKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\ControlSet001\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}", 0, KEY_READ, &hTestKey) == ERROR_SUCCESS)
	{
		CheckVirtualNetAdapter(hTestKey, QueryKey_Sub);
		if (isfirst == true)
		{
			MessageBox(L"无法找到关键项,请先开启移动热点!");
			ShellExecute(NULL, NULL, L"ms-settings:network", NULL, NULL, SW_SHOWNORMAL);
		}
	}
	RegCloseKey(hTestKey);

	isinit = false;
	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CEShareToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CEShareToolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CEShareToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CEShareToolDlg::OnBnClickedShareswitch()
{
	// TODO: 在此添加控件通知处理程序代码

	if (isfrozen == false)
	{
		SetDlgItemText(IDC_ShareSwitch, L"恢复");
		isfrozen = true;
		HKEY hTestKey;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"SYSTEM\\ControlSet001\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}",0, KEY_READ ,&hTestKey) == ERROR_SUCCESS)
		{
			CheckVirtualNetAdapter(hTestKey,QueryKey_Sub);
			if (isfirst == true)
			{
				MessageBox(L"无法找到关键项，请先开启移动热点后再进行配置共享!");
				ShellExecute(NULL, NULL, L"ms-settings:network", NULL, NULL, SW_SHOWNORMAL);

				SetDlgItemText(IDC_ShareSwitch, L"配置共享");
				isfrozen = false;
			}
		}
		RegCloseKey(hTestKey);
	}
	else if (isfrozen == true)
	{
		SetDlgItemText(IDC_ShareSwitch, L"配置共享");
		isfrozen = false;
		HKEY hTestKey;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\ControlSet001\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}", 0, KEY_READ, &hTestKey) == ERROR_SUCCESS)
		{
			CheckVirtualNetAdapter(hTestKey, QueryKey_Sub);
			if (isfirst == true)
			{
				MessageBox(L"无法找到关键项，请先开启移动热点后再进行恢复!");
				ShellExecute(NULL, NULL, L"ms-settings:network", NULL, NULL, SW_SHOWNORMAL);

				SetDlgItemText(IDC_ShareSwitch, L"恢复");
				isfrozen = true;
			}
		}
		RegCloseKey(hTestKey);
	}
}

void CEShareToolDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);
}

BOOL CEShareToolDlg::ifWin10()
{
	typedef void(__stdcall* NTPROC)(DWORD*, DWORD*, DWORD*);
	HINSTANCE hinst = LoadLibrary(L"ntdll.dll");
	DWORD dwMajor, dwMinor, dwBuildNumber;
	NTPROC proc = (NTPROC)GetProcAddress(hinst, "RtlGetNtVersionNumbers");
	proc(&dwMajor, &dwMinor, &dwBuildNumber);
	if (dwMajor == 10 && dwMinor == 0)	//win 10
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CEShareToolDlg::ifWin7()
{
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi);

	// win7的系统版本为NT6.1
	if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId && osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}



void CEShareToolDlg::CheckVirtualNetAdapter(HKEY hKey, DWORD mode)
{
	TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
	DWORD    cbName;                   // size of name string 
	//TCHAR    achClass[MAX_PATH] = TEXT(""), _achClass[MAX_PATH] = TEXT(""); // buffer for class name 
	//DWORD    cchClassName = MAX_PATH, _cchClassName = MAX_PATH;  // size of class string 
	DWORD    cSubKeys = 0;               // number of subkeys 
	DWORD    cValues;              // number of values for key 
	DWORD	i, j ,retCode;

	// Get the class name and the value count. 
	retCode = RegQueryInfoKey(
		hKey,                    // key handle 
		NULL,//achClass,                // buffer for class name 
		NULL,//&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		NULL,//&cbMaxSubKey,            // longest subkey size 
		NULL,//&cchMaxClass,            // longest class string 
		NULL,//&cValues,                // number of values for this key 
		NULL,//&cchMaxValue,            // longest value name 
		NULL,//&cbMaxValueData,         // longest value data 
		NULL,//&cbSecurityDescriptor,   // security descriptor 
		NULL//&ftLastWriteTime       // last write time 
	);
	// Enumerate the subkeys, until RegEnumKeyEx fails.
	if (cSubKeys && mode == QueryKey_Sub)
	{
		for (i = 0; i < cSubKeys; i++)
		{
			cbName = MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(hKey, i,achKey,&cbName,NULL,NULL,NULL,NULL);
			if (retCode == ERROR_SUCCESS)
			{
				SubKey.Format(L"SYSTEM\\ControlSet001\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}\\%s", achKey);
				HKEY KeyValue;
				if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SubKey, 0, KEY_READ, &KeyValue) == ERROR_SUCCESS)
				{
					//CheckVirtualNetAdapter(KeyValue, QueryKey_Value);

					retCode = RegQueryInfoKey(
						KeyValue,                    // key handle 
						NULL,//achClass,                // buffer for class name 
						NULL,//&_cchClassName,           // size of class string 
						NULL,                    // reserved 
						NULL,//&cSubKeys,               // number of subkeys 
						NULL,//&cbMaxSubKey,            // longest subkey size 
						NULL,//&cchMaxClass,            // longest class string 
						&cValues,                // number of values for this key 
						NULL,//&cchMaxValue,            // longest value name 
						NULL,//&cbMaxValueData,         // longest value data 
						NULL,//&cbSecurityDescriptor,   // security descriptor 
						NULL//&ftLastWriteTime       // last write time 
					);
					// Enumerate the key values. 
					if (cValues)
					{
						TCHAR	achValue[MAX_VALUE_NAME];
						DWORD	cchValue = MAX_VALUE_NAME;

						CString DeviceName = NULL, Characteristics = NULL;
						for (j = 0; j < cValues; j++)
						{
							cchValue = MAX_VALUE_NAME;
							achValue[0] = '\0';
							retCode = RegEnumValue(KeyValue, j, achValue, &cchValue, NULL, NULL, NULL, NULL);
							if (retCode == ERROR_SUCCESS)
							{
								CString SubValue;
								SubValue.Format(L"%s", achValue);
								if (SubValue == L"DriverDesc")
								{
									DeviceName = OnQueryStrValue(SubKey, SubValue);
								}
								if (SubValue == L"Characteristics")
								{
									Characteristics = OnQueryDWORDValue(SubKey, SubValue);
								}
							}
						}
						//MessageBox(DeviceName + Characteristics);
						if (DeviceName.Find(L"Microsoft") >= 0 && DeviceName.Find(L"Direct") >= 0 && Characteristics == L"4")
						{
							isfirst = false;
							if (isinit == true)
							{
								SetDlgItemText(IDC_ShareSwitch, L"恢复");
								isfrozen = true;
							}
							else if (isinit == false && isfrozen == false)
							{
								SetRegKey(SubKey, L"Characteristics", 1);
							}
						}
						else if (DeviceName.Find(L"Microsoft") >= 0 && DeviceName.Find(L"Direct") >= 0 && Characteristics == L"1")
						{
							isfirst = false;
							if (isinit == true)
							{
								SetDlgItemText(IDC_ShareSwitch, L"配置共享");
								isfrozen = false;
							}
							else if (isinit == false && isfrozen == true)
							{
								SetRegKey(SubKey, L"Characteristics", 4);
							}
						}
						else if (DeviceName.Find(L"Microsoft") >= 0 && DeviceName.Find(L"Direct") >= 0 && Characteristics == L"c")
						{
							SetDlgItemText(IDC_ShareSwitch, L"恢复");
							isfrozen = true;
						}
					}

				}
				RegCloseKey(KeyValue);
			}
		}
	}
}

CString CEShareToolDlg::OnQueryStrValue(CString cRegKey,CString cRegName)
{
	HKEY hRegkey;
	BYTE data[128];
	memset(data, 0x00, sizeof(data));
	DWORD size;
	DWORD type = REG_SZ;
	CString value;
	long iRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, cRegKey,REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, &hRegkey);
	if (iRet == ERROR_SUCCESS)
	{
		iRet = RegQueryValueEx(hRegkey, cRegName, 0, &type, NULL, &size);//第一次查询
		if (iRet == 0)
		{
			iRet = RegQueryValueEx(hRegkey, cRegName, 0, &type, data, &size);//第二次查询
			if (iRet == 0)
			{
				value.Format(L"%s", data);//得到值
			}
		}
		RegCloseKey(hRegkey);
	}
	else
	{
		AfxMessageBox(L"Error 0x000000B0!");
		value = "-1";
	}
	return value;
}

CString CEShareToolDlg::OnQueryDWORDValue(CString cRegKey, CString cRegName)
{
	CString result;
	HKEY hKey;
	DWORD szLocation = 0;
	DWORD dwSize = sizeof(DWORD);
	DWORD dwType = REG_DWORD;
	LONG ret;
	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, cRegKey, 0, KEY_READ, &hKey);
	if (ERROR_SUCCESS == ret)
	{
		ret = RegQueryValueEx(hKey, cRegName, 0, &dwType, (LPBYTE)&szLocation, &dwSize);
		if (ERROR_SUCCESS == ret)
		{
			result.Format(L"%x", szLocation);
		}
		RegCloseKey(hKey);
	}
	return result;
}

BOOL CEShareToolDlg::SetRegKey(CString Key,CString Value,DWORD val)
{
	HKEY hKey = nullptr;
	//创建成功,将得到hKey,一个注册表句柄,用于下面操作注册表
	if (ERROR_SUCCESS != RegCreateKey(HKEY_LOCAL_MACHINE, Key, &hKey))
	{
		AfxMessageBox(L"Error 0x000000A0!");
		return 1;
	}
	LONG lRet = 0;
	//这个函数可以写入更多的Value                      //名称            //类型        //数据
	if (ERROR_SUCCESS != (lRet = RegSetValueEx(hKey, Value, 0, REG_DWORD, (CONST BYTE*) & val, 4)))
	{
		AfxMessageBox(L"Error 0x000000A2!");
		return 1;
	}
	RegCloseKey(hKey);
	return 1;
}