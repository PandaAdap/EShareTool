#pragma once

#define QueryKey_Sub WM_USER+100
#define QueryKey_Value WM_USER+101

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 1024

// EShareToolWin7 对话框

class EShareToolWin7 : public CDialogEx
{
	DECLARE_DYNAMIC(EShareToolWin7)

public:
	EShareToolWin7(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~EShareToolWin7();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EShareTool_Win7 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	HICON m_hIconx;

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void CheckVirtualNetAdapter(HKEY hKey, DWORD mode);
	CString OnQueryStrValue(CString cRegKey, CString cRegName);
	CString OnQueryDWORDValue(CString cRegKey, CString cRegName);
	BOOL SetRegKey(CString Key, CString Value, DWORD val);
	BOOL WriteReg(CString path, CString key, CString value);
	CString ReadReg(CString path, CString key);
	afx_msg void OnBnClickedApply();
	afx_msg void OnBnClickedStarthotspot();
	afx_msg void OnBnClickedHelp();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
};
