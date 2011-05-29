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

// EditDlgLine.cpp : implementation file
//

#include "stdafx.h"
#include "tinycad.h"
#include "colour.h"
#include "EditDlgLine.h"
#include "EditToolbar.h"

/////////////////////////////////////////////////////////////////////////////
// CEditDlgLine dialog


CEditDlgLine::CEditDlgLine(CWnd* pParent /*=NULL*/)
{
	//{{AFX_DATA_INIT(CEditDlgLine)
	//}}AFX_DATA_INIT
	m_bEntire_net = TRUE;
	m_use_default_style = TRUE;
	m_setup = TRUE;
}

void CEditDlgLine::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditDlgLine)
	DDX_Control(pDX, IDC_SPIN1, m_Spin1);
	DDX_Control(pDX, LINETHICKBOX_THICKNESS, m_Line_Thickness);
	DDX_Control(pDX, LINETHICKBOX_STYLE, m_Line_Style);
	DDX_Control(pDX, IDC_ENTIRE_NET, m_Entire_net);
	DDX_Control(pDX, IDC_DEFAULT_STYLE, m_Default_Style);
	DDX_Control(pDX, IDC_LINE_COLOUR, m_Colour);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEditDlgLine, CDialog)
	//{{AFX_MSG_MAP(CEditDlgLine)
	ON_BN_CLICKED(IDC_LINE_COLOUR, OnLineColour)
	ON_WM_DRAWITEM()
	ON_CBN_SELCHANGE(LINETHICKBOX_STYLE, OnSelchangeStyle)
	ON_EN_CHANGE(LINETHICKBOX_THICKNESS, OnChangeThickness)
	ON_BN_CLICKED( IDC_ENTIRE_NET, OnEntireNet )
	ON_BN_CLICKED( IDC_DEFAULT_STYLE, OnDefaultStyle )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditDlgLine message handlers


static const int PenStyles[] = {PS_SOLID, PS_DASH, PS_DOT, PS_DASHDOT, PS_DASHDOTDOT};

// Create this window
void CEditDlgLine::Create()
{
	CDialog::Create(IDD, &g_EditToolBar);

}

void CEditDlgLine::Open(CTinyCadDoc *pDesign, CDrawingObject *pObject)
{
	m_setup = TRUE;

	Show(pDesign, pObject);

	hSTYLE line = m_pDesign->GetOptions()->GetCurrentStyle(pObject->GetType());
	m_lStyle = *m_pDesign->GetOptions()->GetStyle(line);

	// Now select the correct for the line style..
	for (int i = 0; i < 5; i++)
	{
		if (m_lStyle.Style == PenStyles[i])
		{
			m_Line_Style.SetCurSel(i);
			break;
		}
	}

	CString s;
	s.Format(_T("%d"), m_lStyle.Thickness);
	m_Line_Thickness.SetWindowText(s);

	m_Entire_net.SetCheck(m_bEntire_net ? 1 : 0);
	m_Default_Style.SetCheck(m_use_default_style ? 1 : 0);

	m_Spin1.EnableWindow(!m_use_default_style);
	m_Line_Thickness.EnableWindow(!m_use_default_style);
	m_Line_Style.EnableWindow(!m_use_default_style);
	m_Colour.EnableWindow(!m_use_default_style);

	m_setup = FALSE;
}

void CEditDlgLine::OnLineColour()
{
	// Bring up the colour dialogue...
	if (CTinyCadApp::ChooseColor(m_lStyle.Colour))
	{
		UpdateOptions();
	}

}

void CEditDlgLine::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{

	int which = lpDrawItemStruct->itemID;

	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);

	if (which != -1)
	{
		COLORREF clrBackground = GetSysColor(lpDrawItemStruct->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW);
		COLORREF oldBackground = dc.SetBkColor(clrBackground);

		// Draw the background box
		CBrush theBrush(clrBackground);
		CBrush *oldBrush = dc.SelectObject(&theBrush);
		CPen *oldPen = (CPen *) (dc.SelectStockObject(NULL_PEN));
		dc.Rectangle(& (lpDrawItemStruct->rcItem));

		CPen thePen(PenStyles[which], 1, lpDrawItemStruct->itemState & ODS_SELECTED ? RGB(255,255,255) : RGB(0,0,0));
		dc.SelectObject(&thePen);
		int y = lpDrawItemStruct->rcItem.top + (lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top) / 2;
		int width = (lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left) - 6;

		dc.MoveTo(3, y);
		dc.LineTo(width, y);

		// Restore the dc to it's old state
		dc.SelectObject(oldPen);
		dc.SelectObject(oldBrush);
		dc.SetBkColor(oldBackground);
	}

	// If the item has the focus, draw focus rectangle
	if (lpDrawItemStruct->itemState & ODS_FOCUS) DrawFocusRect(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem);

	dc.Detach();

	// DON'T: CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

BOOL CEditDlgLine::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_Spin1.SetRange(1, 100);

	// Now populate the line style list box
	TCHAR hold[8];
	hold[1] = 0;
	hold[0] = 'a';
	for (int lp = 0; lp != 5; lp++)
	{
		m_Line_Style.AddString(hold);
		hold[0]++;
	}

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CEditDlgLine::OnSelchangeStyle()
{
	if (m_pDesign)
	{
		m_lStyle.Style = PenStyles[m_Line_Style.GetCurSel()];
		UpdateOptions();
	}
}

void CEditDlgLine::OnChangeThickness()
{
	if (m_pDesign)
	{
		CString s, s2;
		m_Line_Thickness.GetWindowText(s);

		m_lStyle.Thickness = _tstoi(s); //returns 0 if unable to convert such as if s is empty
		if (m_lStyle.Thickness < 1) //User tried to enter 0 or deleted the default - this will cause a crash if UpdateOptions() is called
		{
			m_lStyle.Thickness = 1;
		}

		s2.Format(_T("%d"), m_lStyle.Thickness);
		if (s2 != s)
		{
			m_Line_Thickness.SetWindowText(s2); //djl - fixed crash on line thickness deletion here
		}

		UpdateOptions();
	}

}

void CEditDlgLine::UpdateOptions()
{
	if (m_setup)
	{
		return;
	}

	// Write back the line style
	WORD line = m_pDesign->GetOptions()->AddStyle(&m_lStyle);
	m_pDesign->GetOptions()->SetCurrentStyle(getObject()->GetType(), line);

	getObject()->NewOptions();
}

void CEditDlgLine::OnEntireNet()
{
	m_bEntire_net = m_Entire_net.GetCheck() != 0;
	UpdateOptions();
}

void CEditDlgLine::OnDefaultStyle()
{
	m_use_default_style = m_Default_Style.GetCheck() != 0;

	m_Spin1.EnableWindow(!m_use_default_style);
	m_Line_Thickness.EnableWindow(!m_use_default_style);
	m_Line_Style.EnableWindow(!m_use_default_style);
	m_Colour.EnableWindow(!m_use_default_style);

	UpdateOptions();
}
