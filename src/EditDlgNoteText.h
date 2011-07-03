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

#pragma once
#include "diag.h"


// CEditDlgNoteText dialog

//class CEditDlgNoteText : public CDialog
class CEditDlgNoteText : public CEditDlg
{
	DECLARE_DYNAMIC(CEditDlgNoteText)
	BOOL stop;

public:
	CEditDlgNoteText(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditDlgNoteText();
	void Create();
	void Open(CTinyCadDoc *pDesign, CDrawingObject *pObject);
	afx_msg void OnChange();

// Dialog Data
	enum 
	{
		IDD = IDD_TOOLNOTETEXT
	};
	CSpinButtonCtrl m_Spin1;
	CButton m_Fill_Colour;
	CButton m_Fill;
	CEdit m_Line_Thickness;
	CComboBox m_Fill_Index;
	CComboBox m_Line_Style;
	CButton m_border_style_rectangular;
	CButton m_border_style_roundedRect;
	CButton m_border_style_noBorder;
	CEdit m_tab_width;
	CButton m_text_dir_up;
	CButton m_text_dir_right;

protected:
	LineStyle m_lStyle;	//Line style
	FillStyle m_fStyle;	//Rectangle fill style
	BOOL m_do_fill;		//Use fill on rectangle
	//CString str;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void CEditDlgNoteText::UpdateOptions();

	// Generated message map functions
	//{{AFX_MSG(CEditDlgNoteText)
	afx_msg void OnFillColour();
	afx_msg void OnLineColour();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeStyle();
	afx_msg void OnChangeThickness();
	afx_msg void OnSelchangeIndex();
	afx_msg void OnFill();
	afx_msg void OnTextColour();
	afx_msg void OnTextFont();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedNotetextBorderStyle();
	afx_msg void OnEnChangeNotetextTabwidth();
};
