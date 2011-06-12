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


//IDD_TOOLNOTETEXT DIALOGEX 4, 16, 432, 151
//STYLE DS_SETFONT | DS_3DLOOK | DS_FIXEDSYS | WS_CHILD
//FONT 8, "MS Shell Dlg", 0, 0, 0x0
//BEGIN
//    PUSHBUTTON      "Colour",IDC_FILL_COLOUR,163,24,50,14
//    COMBOBOX        IDC_INDEX,221,25,87,79,CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP
//    CONTROL         "Background Fill",IDC_FILL,"Button",BS_AUTOCHECKBOX | WS_GROUP | WS_TABSTOP,165,10,63,10
//    COMBOBOX        LINETHICKBOX_STYLE,105,25,45,42,CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED | CBS_SORT | CBS_HASSTRINGS | WS_VSCROLL | WS_TABSTOP
//    EDITTEXT        LINETHICKBOX_THICKNESS,67,24,31,14,ES_AUTOHSCROLL
//    CONTROL         "Spin1",IDC_SPIN1,"msctls_updown32",UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS,91,24,11,14
//    PUSHBUTTON      "Colour",IDC_LINE_COLOUR,9,24,50,14
//    EDITTEXT        TEXTBOX_TEXT,6,47,423,100,ES_MULTILINE | WS_VSCROLL | WS_HSCROLL | WS_GROUP
//    PUSHBUTTON      "Font",IDC_TEXT_FONT,321,24,50,14
//    PUSHBUTTON      "Colour",IDC_TEXT_COLOUR,373,24,50,14
//    CONTROL         "Right",TEXTBOX_LEFT,"Button",BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP,322,10,31,10
//    CONTROL         "Up",TEXTBOXL_UP,"Button",BS_AUTORADIOBUTTON,374,10,24,10
//    GROUPBOX        "Border Style",IDC_STATIC,5,0,150,41/
//    GROUPBOX        "Rectangle",IDC_STATIC,157,0,155,41
//    GROUPBOX        "Text Characteristics",IDC_STATIC,315,0,114,42
//    CONTROL         "Rectangle",IDC_NOTETEXT_RECTANGLE,"Button",BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP,10,11,46,10
//    CONTROL         "Rounded Rect",IDC_NOTETEXT_ROUNDEDRECT,"Button",BS_AUTORADIOBUTTON,59,11,55,10
//    LISTBOX         IDC_NOTETEXT_TABSPACING,266,8,21,12,LBS_NOINTEGRALHEIGHT | LBS_WANTKEYBOARDINPUT | WS_VSCROLL | WS_TABSTOP
//    LTEXT           "Units",IDC_UNITSTRING,291,11,17,8
//    LTEXT           "Tab Stops",IDC_STATIC,230,11,33,8
//    CONTROL         "None",IDC_NOTETEXT_NOBORDER,"Button",BS_AUTORADIOBUTTON,118,11,29,10
//END


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
};
