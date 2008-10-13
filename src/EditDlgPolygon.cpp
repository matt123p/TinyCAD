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

// EditDlgPolygon.cpp : implementation file
//

#include "stdafx.h"
#include "tinycad.h"
#include "colour.h"
#include "EditDlgPolygon.h"
#include "EditToolbar.h"


/////////////////////////////////////////////////////////////////////////////
// CEditDlgPolygon dialog


CEditDlgPolygon::CEditDlgPolygon(CWnd* pParent /*=NULL*/)
{
	//{{AFX_DATA_INIT(CEditDlgPolygon)
	//}}AFX_DATA_INIT
}


void CEditDlgPolygon::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditDlgPolygon)
	DDX_Control(pDX, IDC_SPIN1, m_Spin1);
	DDX_Control(pDX, IDC_FILL_COLOUR, m_Fill_Colour);
	DDX_Control(pDX, IDC_FILL, m_Fill);
	DDX_Control(pDX, LINETHICKBOX_THICKNESS, m_Line_Thickness);
	DDX_Control(pDX, IDC_INDEX, m_Fill_Index);
	DDX_Control(pDX, LINETHICKBOX_STYLE, m_Line_Style);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditDlgPolygon, CDialog)
	//{{AFX_MSG_MAP(CEditDlgPolygon)
	ON_BN_CLICKED(IDC_FILL_COLOUR, OnFillColour)
	ON_BN_CLICKED(IDC_LINE_COLOUR, OnLineColour)
	ON_WM_DRAWITEM()
	ON_CBN_SELCHANGE(LINETHICKBOX_STYLE, OnSelchangeStyle)
	ON_EN_CHANGE(LINETHICKBOX_THICKNESS, OnChangeThickness)
	ON_CBN_SELCHANGE(IDC_INDEX, OnSelchangeIndex)
	ON_BN_CLICKED(IDC_FILL, OnFill)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditDlgPolygon message handlers


static const int PenStyles[] = { PS_SOLID, PS_DASH, PS_DOT, PS_DASHDOT, PS_DASHDOTDOT };

// Create this window
void CEditDlgPolygon::Create()
{
    CDialog::Create( IDD_POLYGON, &g_EditToolBar );

}


void CEditDlgPolygon::Open(CTinyCadDoc *pDesign, CDrawingObject *pObject)
{
	Show( pDesign, pObject );


	// Get the current style
	hFILL fill = m_pDesign->GetOptions()->GetCurrentFillStyle( getObject()->GetType() );
	m_fStyle = *m_pDesign->GetOptions()->GetFillStyle( fill );

	hSTYLE line = m_pDesign->GetOptions()->GetCurrentStyle(getObject()->GetType());
	m_lStyle = *m_pDesign->GetOptions()->GetStyle( line );

	// Now select the correct for the line style..
	for (int i=0;i<5;i++)
	{
		if (m_lStyle.Style == PenStyles[i])
		{
			m_Line_Style.SetCurSel(i);
			break;
		}
	}


	if (m_fStyle.Index == -1)
	{
		m_fStyle.Index = 0;
	}

	m_Fill_Index.SetCurSel( m_fStyle.Index );

	m_do_fill = fill != fsNONE;

	m_Fill_Index.EnableWindow( m_do_fill );
	m_Fill_Colour.EnableWindow( m_do_fill );
	m_Fill.SetCheck( m_do_fill ? 1 : 0 );

	CString s;
	s.Format( _T("%d"), m_lStyle.Thickness );
	m_Line_Thickness.SetWindowText( s );
}


void CEditDlgPolygon::OnFillColour() 
{
	// Bring up the colour dialogue...
	if (CTinyCadApp::ChooseColor( m_fStyle.Colour ))
	{
		UpdateOptions();	
	}	
}

void CEditDlgPolygon::OnLineColour() 
{
	// Bring up the colour dialogue...
	if (CTinyCadApp::ChooseColor( m_lStyle.Colour ))
	{
		UpdateOptions();
	}

}

void CEditDlgPolygon::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{

	int which = lpDrawItemStruct->itemID;

	CDC dc;
	dc.Attach( lpDrawItemStruct->hDC );

	if (which != -1) {
	    COLORREF clrBackground = GetSysColor(lpDrawItemStruct->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW);
		COLORREF oldBackground = dc.SetBkColor(clrBackground);

		// Draw the background box
		CBrush theBrush(clrBackground);
		CBrush *oldBrush = dc.SelectObject(&theBrush);
		CPen *oldPen = (CPen *)(dc.SelectStockObject(NULL_PEN));
		dc.Rectangle(&(lpDrawItemStruct->rcItem));

		CPen thePen(PenStyles[which],1,lpDrawItemStruct->itemState & ODS_SELECTED ? RGB(255,255,255) : RGB(0,0,0));
		dc.SelectObject(&thePen);
		int y = lpDrawItemStruct->rcItem.top + (lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top)/2;
		int width = (lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left)-6;

    	dc.MoveTo(3,y);
		dc.LineTo(width,y);

		// Restore the dc to it's old state
		dc.SelectObject(oldPen);
		dc.SelectObject(oldBrush);
		dc.SetBkColor(oldBackground);
	}


   // If the item has the focus, draw focus rectangle
   if (lpDrawItemStruct->itemState & ODS_FOCUS)
		DrawFocusRect(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem);

   dc.Detach();
	
	// DON'T: CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

BOOL CEditDlgPolygon::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_Spin1.SetRange( 1, 100 );
	
	// Now populate the line style list box
	TCHAR hold[8];
	hold[1] = 0;
	hold[0] = 'a';
	for (int lp = 0; lp != 5; lp++) 
	{
		m_Line_Style.AddString(hold);
		hold[0] ++;
	}


	m_Fill_Index.AddString(_T("Solid"));
	m_Fill_Index.AddString(_T("Horizontal"));
	m_Fill_Index.AddString(_T("Vertical"));
	m_Fill_Index.AddString(_T("Downwards"));
	m_Fill_Index.AddString(_T("Upwards"));
	m_Fill_Index.AddString(_T("Crosshatch"));
	m_Fill_Index.AddString(_T("Diagonal Crosshatch"));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditDlgPolygon::OnSelchangeIndex() 
{
	if (m_pDesign)
	{
		m_fStyle.Index = m_Fill_Index.GetCurSel();
		UpdateOptions();
	}	
}


void CEditDlgPolygon::OnSelchangeStyle() 
{
	if (m_pDesign)
	{
		m_lStyle.Style = PenStyles[ m_Line_Style.GetCurSel() ];
		UpdateOptions();
	}
}

void CEditDlgPolygon::OnChangeThickness() 
{
	if (m_pDesign)
	{
		CString s,s2;
		m_Line_Thickness.GetWindowText( s );
		m_lStyle.Thickness = _tstoi( s );
		if (m_lStyle.Thickness < 1)
		{
			m_lStyle.Thickness = 1;
		}

		s2.Format(_T("%d"),m_lStyle.Thickness);
		if (s2 != s)
		{
			m_Line_Thickness.SetWindowText( s );
		}

		UpdateOptions();
	}
	
}


void CEditDlgPolygon::UpdateOptions()
{
	// Update the window state
	m_Fill_Colour.EnableWindow( m_do_fill );
	m_Fill_Index.EnableWindow( m_do_fill );

	// Write back the fill
	if (!m_do_fill)
	{
		// No fill
		m_pDesign->GetOptions()->SetCurrentFillStyle( getObject()->GetType(), fsNONE );
	}
	else
	{
		// Fill
		WORD fill = m_pDesign->GetOptions()->AddFillStyle(&m_fStyle);
		m_pDesign->GetOptions()->SetCurrentFillStyle( getObject()->GetType(), fill );
	}

	// Write back the line style
	WORD line = m_pDesign->GetOptions()->AddStyle(&m_lStyle);
	m_pDesign->GetOptions()->SetCurrentStyle( getObject()->GetType(), line );

	getObject()->NewOptions();
}


void CEditDlgPolygon::OnFill() 
{
	m_do_fill = (m_Fill.GetState()&3) != 0;
	UpdateOptions();
}
