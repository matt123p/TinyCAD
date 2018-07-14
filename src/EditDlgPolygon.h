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

#if !defined(AFX_EDITDLGPOLYGON_H__4BFECAA0_EF6E_4C81_AFC6_7E92E56283BD__INCLUDED_)
#define AFX_EDITDLGPOLYGON_H__4BFECAA0_EF6E_4C81_AFC6_7E92E56283BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditDlgPolygon.h : header file
//

#include "diag.h"
#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// CEditDlgPolygon dialog

class CEditDlgPolygon: public CEditDlg
{
	// Construction
public:
	CEditDlgPolygon(CWnd* pParent = NULL); // standard constructor

	void Create();
	void Open(CTinyCadDoc *pDesign, CDrawingObject *pObject);

	// Dialog Data
	//{{AFX_DATA(CEditDlgPolygon)
	enum
	{
		IDD = IDD_POLYGON
	};
	CSpinButtonCtrl m_Spin1;
	CButton m_Fill_Colour;
	CButton m_Fill;
	CEdit m_Line_Thickness;
	CComboBox m_Fill_Index;
	CComboBox m_Line_Style;
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditDlgPolygon)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:

	LineStyle m_lStyle;
	FillStyle m_fStyle;
	BOOL m_do_fill;
	BOOL m_close_polygon;

	void UpdateOptions();

	// Generated message map functions
	//{{AFX_MSG(CEditDlgPolygon)
	afx_msg void OnFillColour();
	afx_msg void OnLineColour();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeStyle();
	afx_msg void OnChangeThickness();
	afx_msg void OnSelchangeIndex();
	afx_msg void OnFill();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedClose();
	CButton m_Close;
 };

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITDLGPOLYGON_H__4BFECAA0_EF6E_4C81_AFC6_7E92E56283BD__INCLUDED_)
