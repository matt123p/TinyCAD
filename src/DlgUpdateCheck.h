#pragma once


// CDlgUpdateCheck dialog

class CDlgUpdateCheck : public CDialog
{
	DECLARE_DYNAMIC(CDlgUpdateCheck)

public:
	CDlgUpdateCheck(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDlgUpdateCheck();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UPDATECHECK };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CButton m_Enable_Updates;
	afx_msg void OnBnClickedCheckNow();
	afx_msg void OnClose();
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
