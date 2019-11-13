// DlgUpdateCheck.cpp : implementation file
//

#include "StdAfx.h"
#include "TinyCad.h"
#include "DlgUpdateCheck.h"
#include "afxdialogex.h"
#include "TinyCadRegistry.h"


// CDlgUpdateCheck dialog

IMPLEMENT_DYNAMIC(CDlgUpdateCheck, CDialog)

CDlgUpdateCheck::CDlgUpdateCheck(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_UPDATECHECK, pParent)
{
}

CDlgUpdateCheck::~CDlgUpdateCheck()
{
}

void CDlgUpdateCheck::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK1, m_Enable_Updates);
}

BOOL CDlgUpdateCheck::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_Enable_Updates.SetCheck(CTinyCadRegistry::GetAutomaticUpdatesOn() != 0 ? BST_CHECKED : BST_UNCHECKED);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}



BEGIN_MESSAGE_MAP(CDlgUpdateCheck, CDialog)
	ON_BN_CLICKED(ID_CHECK_NOW, &CDlgUpdateCheck::OnBnClickedCheckNow)
	ON_BN_CLICKED(IDOK, &CDlgUpdateCheck::OnBnClickedOk)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


void CDlgUpdateCheck::OnBnClickedCheckNow()
{
	CTinyCadRegistry::SetAutomaticUpdatesOn(m_Enable_Updates.GetCheck() == BST_CHECKED);
	EndDialog(ID_CHECK_NOW);
}


void CDlgUpdateCheck::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnClose();
}


void CDlgUpdateCheck::OnBnClickedOk()
{
	CTinyCadRegistry::SetAutomaticUpdatesOn(m_Enable_Updates.GetCheck() == BST_CHECKED);
	CDialog::OnOK();
}


