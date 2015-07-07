/*
 TinyCAD program for schematic capture
 Copyright 1994/1995/2002-2005 Matt Pyne.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#if !defined(AFX_DLGRENAMESHEET_H__75620E13_149F_4EA4_A4E4_0CB6D4C5C184__INCLUDED_)
#define AFX_DLGRENAMESHEET_H__75620E13_149F_4EA4_A4E4_0CB6D4C5C184__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgRenameSheet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgRenameSheet dialog

class CDlgRenameSheet: public CDialog
{
	// Construction
public:
	CDlgRenameSheet(CWnd* pParent = NULL); // standard constructor

	// Dialog Data
	//{{AFX_DATA(CDlgRenameSheet)
	enum
	{
		IDD = IDD_RENAME_SHEET
	};
	CString m_Name;
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgRenameSheet)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgRenameSheet)
	// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGRENAMESHEET_H__75620E13_149F_4EA4_A4E4_0CB6D4C5C184__INCLUDED_)
