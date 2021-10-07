
// EShareToolDlg.h: 头文件
//

#pragma once

#define QueryKey_Sub WM_USER+100
#define QueryKey_Value WM_USER+101

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 1024

// CEShareToolDlg 对话框
class CEShareToolDlg : public CDialogEx
{
// 构造
public:
	CEShareToolDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ESHARETOOL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedShareswitch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	BOOL ifWin10();
	BOOL ifWin7();
	void CheckVirtualNetAdapter(HKEY hKey, DWORD mode);
	CString OnQueryStrValue(CString cRegKey, CString cRegName);
	CString OnQueryDWORDValue(CString cRegKey, CString cRegName);
	BOOL SetRegKey(CString Key, CString Value, DWORD val);
};
