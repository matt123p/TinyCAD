////////////////////////////////////////////////////////////////////////////
//
// InitDialogBar.cpp: implementation of the CInitDialogBar class.
//
//////////////////////////////////////////////////////////////////////
/*
 (c) 1998 Code Guru (I think?)
 Code written by Alger Pike, and presented on the codegru web-site.

 See:
 http://www.codeguru.com/Cpp/W-D/docking/article.php/c1447/
 */
#include "stdafx.h"
#include "InitDialogBar.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CInitDialogBar, CPaneDialog)

BEGIN_MESSAGE_MAP(CInitDialogBar, CPaneDialog)
//{{AFX_MSG_MAP(CInitDialogBar)
// NOTE - the ClassWizard will add and remove mapping macros here.
//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, &CInitDialogBar::HandleInitDialog)
END_MESSAGE_MAP()

CInitDialogBar::CInitDialogBar()
{
	// In derived classes set intial
	// state of control(s) here
}

CInitDialogBar::~CInitDialogBar()
{

}

BOOL CInitDialogBar::OnInitDialog()
{
	// Support for the MFC DDX model 
	// If you do not want this do not call the base class
	// from derived classes
	UpdateData(FALSE);

	return TRUE;
}

LRESULT CInitDialogBar::HandleInitDialog(WPARAM wParam, LPARAM lParam)
{
	OnInitDialog();

	return CPaneDialog::HandleInitDialog(wParam, lParam);
}