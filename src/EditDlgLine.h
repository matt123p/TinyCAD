/*
 TinyCAD program for schematic capture
 Copyright 1994/1995/2002,2003 Matt Pyne.

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

#if !defined(AFX_EditDlgLine_H__4BFECAA0_EF6E_4C81_AFC6_7E92E56283BD__INCLUDED_)
#define AFX_EditDlgLine_H__4BFECAA0_EF6E_4C81_AFC6_7E92E56283BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditDlgLine.h : header file
//

#include "diag.h"

/////////////////////////////////////////////////////////////////////////////
// CEditDlgLine dialog

class CEditDlgLine: public CEditDlg
{
	// Construction
public:
	CEditDlgLine(CWnd* pParent = NULL); // standard constructor

	void Create();
	void Open(CTinyCadDoc *pDesign, CDrawingObject *pObject);

	BOOL m_setup;
	BOOL m_use_default_style;
	BOOL m_bEntire_net;

	// Dialog Data
	//{{AFX_DATA(CEditDlgLine)
	enum
	{
		IDD = IDD_LINE
	};
	CSpinButtonCtrl m_Spin1;
	CEdit m_Line_Thickness;
	CComboBox m_Line_Style;
	CButton m_Entire_net;
	CButton m_Default_Style;
	CButton m_Colour;
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditDlgLine)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:

	LineStyle m_lStyle;
	void UpdateOptions();

	// Generated message map functions
	//{{AFX_MSG(CEditDlgLine)
	afx_msg void OnEntireNet();
	afx_msg void OnLineColour();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeStyle();
	afx_msg void OnChangeThickness();
	afx_msg void OnDefaultStyle();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EditDlgLine_H__4BFECAA0_EF6E_4C81_AFC6_7E92E56283BD__INCLUDED_)
