// EShareToolWin7.cpp: 实现文件
//

#include "pch.h"
#include "EShareTool.h"
#include "EShareToolWin7.h"
#include "afxdialogex.h"

#include "wifishareCommonLib.h"


#define QueryKey_Sub WM_USER+100
#define QueryKey_Value WM_USER+101

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
CString SubKey_7;

bool isfrozen_7 = false;
bool isfirst_7 = true;
bool isinit_7 = true;

bool issethotspot = false;
bool isstarthotspot = false;

CString SSID = L"EShareTool", Password = L"12345678";

// EShareToolWin7 对话框

IMPLEMENT_DYNAMIC(EShareToolWin7, CDialogEx)

EShareToolWin7::EShareToolWin7(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EShareTool_Win7, pParent)
{
	m_hIconx = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

EShareToolWin7::~EShareToolWin7()
{
}

void EShareToolWin7::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(EShareToolWin7, CDialogEx)
	ON_BN_CLICKED(IDC_Apply, &EShareToolWin7::OnBnClickedApply)
	ON_BN_CLICKED(IDC_StartHotspot, &EShareToolWin7::OnBnClickedStarthotspot)
	ON_BN_CLICKED(IDC_Help, &EShareToolWin7::OnBnClickedHelp)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// EShareToolWin7 消息处理程序


BOOL EShareToolWin7::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	MessageBox(L"本程序仅适合通过无线网卡开启热点共享校园网!\r\n若您的电脑没有无线网卡,则本程序为摆设.");


	HKEY hTestKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\ControlSet001\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}", 0, KEY_READ, &hTestKey) == ERROR_SUCCESS)
	{
		CheckVirtualNetAdapter(hTestKey, QueryKey_Sub);
		if (isfirst_7 == true)
		{
			MessageBox(L"无法找到关键项,请先配置移动网络!");
			//ShellExecute(NULL, NULL, L"ms-settings:network", NULL, NULL, SW_SHOWNORMAL);
		}
	}
	RegCloseKey(hTestKey);

	if (ReadReg(L"SOFTWARE\\EShareTool", L"SSID") == L"false")
	{
		WriteReg(L"SOFTWARE\\EShareTool", L"SSID", SSID);
		WriteReg(L"SOFTWARE\\EShareTool", L"Password", Password);
	}
	else
	{
		SSID = ReadReg(L"SOFTWARE\\EShareTool", L"SSID");
		Password= ReadReg(L"SOFTWARE\\EShareTool", L"Password");
	}
	SetDlgItemText(IDC_SSID, SSID);
	SetDlgItemText(IDC_Password, Password);



	isinit_7 = false;
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void EShareToolWin7::CheckVirtualNetAdapter(HKEY hKey, DWORD mode)
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
				SubKey_7.Format(L"SYSTEM\\ControlSet001\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}\\%s", achKey);
				HKEY KeyValue;
				if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SubKey_7, 0, KEY_READ, &KeyValue) == ERROR_SUCCESS)
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
									DeviceName = OnQueryStrValue(SubKey_7, SubValue);
								}
								if (SubValue == L"Characteristics")
								{
									Characteristics = OnQueryDWORDValue(SubKey_7, SubValue);
								}
							}
						}
						//MessageBox(DeviceName + Characteristics);
						if (DeviceName.Find(L"Microsoft") >= 0 && DeviceName.Find(L"Direct") >= 0 && Characteristics == L"4")
						{
							isfirst_7 = false;
							if (isinit_7 == true)
							{
								SetDlgItemText(IDC_ShareSwitch, L"恢复");
								isfrozen_7 = true;
							}
							else if (isinit_7 == false && isfrozen_7 == false)
							{
								SetRegKey(SubKey_7, L"Characteristics", 1);
							}
						}
						else if (DeviceName.Find(L"Microsoft") >= 0 && DeviceName.Find(L"Direct") >= 0 && Characteristics == L"1")
						{
							isfirst_7 = false;
							if (isinit_7 == true)
							{
								SetDlgItemText(IDC_ShareSwitch, L"配置共享");
								isfrozen_7 = false;
							}
							else if (isinit_7 == false && isfrozen_7 == true)
							{
								SetRegKey(SubKey_7, L"Characteristics", 4);
							}
						}
						else if (DeviceName.Find(L"Microsoft") >= 0 && DeviceName.Find(L"Direct") >= 0 && Characteristics == L"c")
						{
							SetDlgItemText(IDC_ShareSwitch, L"恢复");
							isfrozen_7 = true;
						}
					}

				}
				RegCloseKey(KeyValue);
			}
		}
	}
}

CString EShareToolWin7::OnQueryStrValue(CString cRegKey, CString cRegName)
{
	HKEY hRegkey;
	BYTE data[128];
	memset(data, 0x00, sizeof(data));
	DWORD size;
	DWORD type = REG_SZ;
	CString value;
	long iRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, cRegKey, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, &hRegkey);
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

CString EShareToolWin7::OnQueryDWORDValue(CString cRegKey, CString cRegName)
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

BOOL EShareToolWin7::SetRegKey(CString Key, CString Value, DWORD val)
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

BOOL EShareToolWin7::WriteReg(CString path, CString key, CString val)
{
	const size_t strsize = (val.GetLength() + 1) * 2;
	char* value = new char[strsize];
	size_t sz = 0;
	wcstombs_s(&sz, value, strsize, val, _TRUNCATE);

	HKEY hKey;
	DWORD dwDisp;
	DWORD dwType = REG_SZ; //数据类型

	int ret = RegCreateKeyEx(HKEY_LOCAL_MACHINE, path, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp);
	if (ret != ERROR_SUCCESS)
	{
		//cout << "创建注册表失败" << endl;
		return 1;
	}
	ret = RegSetValueEx(hKey, key, 0, dwType, (BYTE*)value, strlen(value));
	if (ret != ERROR_SUCCESS)
	{
		//cout << "注册表中创建KEY VALUE失败" << endl;
		RegCloseKey(hKey);
		return 1;
	}
	RegCloseKey(hKey);
	return 0;
}

CString EShareToolWin7::ReadReg(CString path, CString key)
{
	char* value;
	CString val;
	HKEY hKey;
	int ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0, KEY_EXECUTE, &hKey);
	if (ret != ERROR_SUCCESS)
	{
		return L"false";
		//cout << "打开注册表失败" << endl;
		//return 1;
	}

	//读取KEY
	DWORD dwType = REG_SZ; //数据类型
	DWORD cbData = 256;
	ret = RegQueryValueEx(hKey, key, NULL, &dwType, (LPBYTE)value, &cbData);
	if (ret == ERROR_SUCCESS)
	{
		//cout << value << endl;
		val.Format(L"%S", value);
	}
	else
	{
		//cout << "读取注册表中KEY 失败" << endl;
		RegCloseKey(hKey);
		return L"false";
	}
	RegCloseKey(hKey);

	return val;
}

void EShareToolWin7::OnBnClickedApply()
{
	// TODO: 在此添加控件通知处理程序代码
	CString ssid, password;
	GetDlgItemText(IDC_SSID, ssid);
	GetDlgItemText(IDC_Password, password);

	if (ssid == L"" || password == L"")
	{
		AfxMessageBox(L"名称或密码为空!");
		SetDlgItemText(IDC_SSID, SSID);
		SetDlgItemText(IDC_Password, Password);
		return;
	}
	if (password.GetLength() < 8 || password.GetLength() > 15)
	{
		AfxMessageBox(L"密码长度为8-15位!");
		SetDlgItemText(IDC_SSID, SSID);
		SetDlgItemText(IDC_Password, Password);
		return;
	}
	if (ssid.GetLength() > 15)
	{
		AfxMessageBox(L"名称长度1-15位!");
		SetDlgItemText(IDC_SSID, SSID);
		SetDlgItemText(IDC_Password, Password);
		return;
	}

	for (int i=0; i <= ssid.GetLength(); i++)
	{
		int unicode = (int)ssid.GetAt(i);
		if (unicode > 255)
		{
			AfxMessageBox(L"含有非法字符!");
			SetDlgItemText(IDC_SSID, SSID);
			SetDlgItemText(IDC_Password, Password);
			return;
		}
	}
	for (int i = 0; i <= password.GetLength(); i++)
	{
		int unicode = (int)password.GetAt(i);
		if (unicode > 255)
		{
			AfxMessageBox(L"含有非法字符!");
			SetDlgItemText(IDC_SSID, SSID);
			SetDlgItemText(IDC_Password, Password);
			return;
		}
	}


	SSID = ssid;
	Password = password;
	WriteReg(L"SOFTWARE\\EShareTool", L"SSID", SSID);
	WriteReg(L"SOFTWARE\\EShareTool", L"Password", Password);

	ShellExecute(NULL, NULL, L"netsh", L"wlan stop hostednetwork", NULL, SW_HIDE);
	Sleep(2000);
	ShellExecute(NULL, NULL, L"netsh", L"wlan set hostednetwork mode=allow ssid=" + SSID + L" key=" + password, NULL, SW_HIDE);

	issethotspot = true;
	GetDlgItem(IDC_StartHotspot)->EnableWindow(TRUE);

}


void EShareToolWin7::OnBnClickedStarthotspot()
{
	// TODO: 在此添加控件通知处理程序代码
	if (isstarthotspot == false)
	{
		SetDlgItemText(IDC_StartHotspot, L"关闭热点");
		ShellExecute(NULL, NULL, L"netsh", L"wlan start hostednetwork", NULL, SW_HIDE);
		isstarthotspot = true;
		isfrozen_7 = true;
	}
	else if (isstarthotspot == true)
	{
		SetDlgItemText(IDC_StartHotspot, L"开启热点");
		ShellExecute(NULL, NULL, L"netsh", L"wlan stop hostednetwork", NULL, SW_HIDE);
		isstarthotspot = false; 
		isfrozen_7 = false;
	}

	HKEY hTestKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\ControlSet001\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}", 0, KEY_READ, &hTestKey) == ERROR_SUCCESS)
	{
		CheckVirtualNetAdapter(hTestKey, QueryKey_Sub);
	}
	RegCloseKey(hTestKey);
}


void EShareToolWin7::OnBnClickedHelp()
{
	// TODO: 在此添加控件通知处理程序代码
	
	MessageBox(L"请在接下来的窗口中选择连接了网络的适配器(非Microsoft Virtual Adaptor字样)\r\n右键-->属性-->共享-->勾选\"允许其他网络用户通过此计算机的Internet连接来连接\"\r\n-->在下面家庭网络选择有Microsoft Virtual字样的网络连接(没有可以跳过)-->确定\r\n然后再重新开启热点");
	ShellExecute(NULL, NULL, L"ncpa.cpl", NULL, NULL, SW_SHOWNORMAL);
	OnBnClickedStarthotspot();
}

void EShareToolWin7::OnPaint()
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
		dc.DrawIcon(x, y, m_hIconx);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}


HCURSOR EShareToolWin7::OnQueryDragIcon()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	return static_cast<HCURSOR>(m_hIconx);
}
