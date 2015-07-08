//////////////////////////////////////////////////////////////////////
//
// InitDialogBar.h: interface for the CInitDialogBar class.
//
//////////////////////////////////////////////////////////////////////

/*
 (c) 1998 Code Guru (I think?)
 Code written by Alger Pike, and presented on the codegru web-site.

 See:
 http://www.codeguru.com/Cpp/W-D/docking/article.php/c1447/
 */

#if !defined(AFX_INITDIALOGBAR_H__46B4D2B3_C982_11D1_8902_0060979C2EFD__INCLUDED_)

#define AFX_INITDIALOGBAR_H__46B4D2B3_C982_11D1_8902_0060979C2EFD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
////////////////////////////////////////////////////////////////////////////
//
// CInitDialogBar window
//
////////////////////////////////////////////////////////////////////////////

class CInitDialogBar : public CPaneDialog
{
	DECLARE_DYNAMIC( CInitDialogBar)

	// Construction / Destruction
public:
	CInitDialogBar();
	virtual ~CInitDialogBar();

	

	// Attributes
public:

	// Operations
public:

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInitDialogBar)
protected:
	//}}AFX_VIRTUAL

	// Implementation
public:

protected:
	virtual BOOL OnInitDialog();
	// Generated message map functions
protected:
	//{{AFX_MSG(CInitDialogBar)
	// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT HandleInitDialog(WPARAM, LPARAM);
};

////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_INITDIALOGBAR_H__46B4D2B3_C982_11D1_8902_0060979C2EFD__INCLUDED_)
