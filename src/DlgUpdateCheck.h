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
};
