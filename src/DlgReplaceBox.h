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

#include "afxwin.h"
#if !defined(AFX_DLGREPLACEBOX_H__6F273C89_9C24_4CE8_9C64_C5C2E0B5A1F7__INCLUDED_)
#define AFX_DLGREPLACEBOX_H__6F273C89_9C24_4CE8_9C64_C5C2E0B5A1F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgReplaceBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgReplaceBox dialog

class CDlgReplaceBox: public CDialog
{
	// Construction
public:
	CDlgReplaceBox(CWnd* pParent = NULL); // standard constructor

	CLibraryStoreSymbol *GetSymbol()
	{
		return m_Symbol;
	}

	// Dialog Data
	//{{AFX_DATA(CDlgReplaceBox)
	enum
	{
		IDD = IDD_REPLACE
	};
	CButton m_Show_Symbol;
	CListBox m_List;
	CString m_search_string;
	int m_all_symbols;
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgReplaceBox)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:

	CLibraryStoreSymbol *m_Symbol;
	void BuildSearchList();

	// Generated message map functions
	//{{AFX_MSG(CDlgReplaceBox)
	afx_msg void OnChangeSearchString();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnDblclkList();
	afx_msg void OnSelchangeList();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
DECLARE_MESSAGE_MAP()
public:
	BOOL m_keep_old_fields;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGREPLACEBOX_H__6F273C89_9C24_4CE8_9C64_C5C2E0B5A1F7__INCLUDED_)
