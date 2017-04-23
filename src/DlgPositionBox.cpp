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

// DlgPositionBox.cpp : implementation file
//

#include "stdafx.h"
#include "tinycad.h"
#include "DlgPositionBox.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgPositionBox dialog


CDlgPositionBox::CDlgPositionBox()
{
	//{{AFX_DATA_INIT(CDlgPositionBox)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CDlgPositionBox, CMFCToolBar)
//{{AFX_MSG_MAP(CDlgPositionBox)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgPositionBox message handlers

// Change the text in the Dialog
void CDlgPositionBox::SetPosition(const CString s)
{
	CToolbarLabel* position = dynamic_cast<CToolbarLabel*>(GetButton(0));
	if (position)
	{
		position->m_strText = s;
		InvalidateButton(0);
	}
}

void CDlgPositionBox::SetGridSize(double gridVal, int unitsVal)
{
	double ScaledGrid;
	if (unitsVal == 0) //Units = 0; -> mm
	{
		ScaledGrid = gridVal / static_cast<double> (PIXELSPERMM);
	}
	else //Units = 1; -> inch
	{
		ScaledGrid = (1000.0 * gridVal) / (254.0 * PIXELSPERMM);
	}

	CString s;
	s.Format(_T("%.4lg"), ScaledGrid);

	CMFCToolBarEditBoxButton* gridSize = dynamic_cast<CMFCToolBarEditBoxButton*>(GetButton(3));
	CToolbarLabel* units = dynamic_cast<CToolbarLabel*>(GetButton(4));
	if (gridSize != nullptr) {
		gridSize->SetContents(s);
	}
	if (units != nullptr) {
		if (unitsVal == 0)
		{
			units->m_strText = ".1mm";
		}
		else
		{
			units->m_strText = ".001\"";
		}
		InvalidateButton(4);
	}
}

// class definition
IMPLEMENT_SERIAL(CToolbarLabel, CMFCToolBarButton, 1)

CToolbarLabel::CToolbarLabel(UINT nID, int size)
: CToolbarLabel(nID, "", size)
{

}

CToolbarLabel::CToolbarLabel(UINT nID, CString text, int size)
{
	m_strText = text;
	m_size = size;
	m_bText = TRUE;
	m_nID = nID;
	m_iImage = -1;
}

SIZE CToolbarLabel::OnCalculateSize(CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	SIZE size = CMFCToolBarButton::OnCalculateSize(pDC, sizeDefault, bHorz);

	if (bHorz)
	{
		if (size.cx < m_size)
		{
			size.cx = m_size;
		}
	}
	return size;
}

void CToolbarLabel::CopyFrom(const CMFCToolBarButton& s)
{
	CMFCToolBarButton::CopyFrom(s);

	const CToolbarLabel& src = static_cast<const CToolbarLabel&>(s);

	m_size = src.m_size;
}

void CToolbarLabel::Serialize(CArchive& ar)
{
	CMFCToolBarButton::Serialize(ar);

	if (ar.IsLoading())
	{
		ar >> m_size;
	}
	else
	{
		ar << m_size;
	}
}

void CToolbarLabel::OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* /*pImages*/,
	BOOL /*bHorz*/, BOOL /*bCustomizeMode*/, BOOL /*bHighlight*/,
	BOOL /*bDrawBorder*/, BOOL /*bGrayDisabledButtons*/)
{
	pDC->SetTextColor(GetGlobalData()->clrBtnText);
	CRect rectText = rect;
	pDC->DrawText(m_strText, &rectText, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
}

