/*
 TinyCAD program for schematic capture
 Copyright 1994/1995/2002 Matt Pyne.

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

#include "stdafx.h"
#include "TinyCadView.h"
#include "revision.h"
#include "colour.h"
#include "context.h"
#include "option.h"
#include "ruler.h"

BEGIN_MESSAGE_MAP( Ruler, CWnd )
	ON_WM_PAINT()
END_MESSAGE_MAP()

// The constructor for a ruler
Ruler::Ruler(CMultiSheetDoc *pDesign, int nDir, CRect nSize, CWnd *parent, BOOL origin)
{
	m_pDesign = pDesign;
	oldPosition = CPoint(0, 0);
	dir = nDir;
	hasOrigin = origin;

	if (dir == 2)
	{
		// Now create the ruler
		Create(NULL, _T(""), WS_CHILD | WS_VISIBLE, nSize, parent, NULL);

		if (hasOrigin)
		{
			button.Create(_T("+"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW, CRect(0, 0, RULER_WIDTH, RULER_WIDTH), parent, IDM_TOOLORIGIN);
		}

	}
	else
	{
		// Now create the ruler
		Create(NULL, _T(""), WS_CHILD | WS_VISIBLE, nSize, parent, NULL);
	}

	OnNewSize(nSize);
}

void Ruler::OnNewSize(CRect nSize)
{
	Size = nSize;

	if (dir == 2)
	{
		Size = CRect(Size.left + (hasOrigin ? RULER_WIDTH : 0), Size.top, Size.right, Size.top + RULER_WIDTH);
		theHeight = Size.right - Size.left;
		MoveWindow(Size);
		Size = CRect(0, 0, theHeight, RULER_WIDTH);
	}
	else
	{
		Size = CRect(Size.right - RULER_WIDTH, Size.top + RULER_WIDTH, Size.right, Size.bottom);
		MoveWindow(Size);
		theHeight = Size.bottom - Size.top;
		Size = CRect(0, 0, RULER_WIDTH, theHeight);
	}

}

Transform& Ruler::GetTransform()
{
	return m_pDesign->GetCurrentSheet()->GetTransform();
}

// Draw the line on the ruler to show position
void Ruler::ShowPosition(CPoint r)
{
	CClientDC dc(this);

	CPen *oldPen = (CPen *) (dc.SelectStockObject(BLACK_PEN));
	dc.SetROP2(R2_NOTXORPEN);
	dc.SetBkMode(OPAQUE);

	int offset = hasOrigin ? RULER_WIDTH : 0;

	// Remove the old marker
	if (dir == 2)
	{
		dc.MoveTo(oldPosition.x - offset, Size.top + RULER_WIDTH);
		dc.LineTo(oldPosition.x - offset, Size.top);
	}
	else
	{
		dc.MoveTo(Size.right - RULER_WIDTH, oldPosition.y - RULER_WIDTH);
		dc.LineTo(Size.right, oldPosition.y - RULER_WIDTH);
	}

	// Now draw in the new marker
	if (dir == 2)
	{
		dc.MoveTo(r.x - offset, Size.top + RULER_WIDTH);
		dc.LineTo(r.x - offset, Size.top);
	}
	else
	{
		dc.MoveTo(Size.right - RULER_WIDTH, r.y - RULER_WIDTH);
		dc.LineTo(Size.right, r.y - RULER_WIDTH);
	}

	// Store when the marker is currently drawn
	oldPosition = r;

	dc.SelectObject(oldPen);
}

// Draw the detail in the ruler
void Ruler::OnPaint()
{
	CPaintDC dc(this);

	CFont theFont;
	theFont.CreateFont(14, 0, (dir == 2) ? 0 : 2700, 0, 400, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS | CLIP_LH_ANGLES, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, _T("Courier New"));

	// Select the objects to draw with
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextAlign(TA_CENTER | TA_BASELINE | TA_NOUPDATECP);
	dc.SetTextColor(RGB(0,128,64));
	dc.SetROP2(R2_COPYPEN);

	CFont *oldFont = (CFont *) (dc.SelectObject(&theFont));
	CPen *oldPen = (CPen *) (dc.SelectStockObject(BLACK_PEN));
	CBrush *oldBrush = (CBrush *) (dc.SelectStockObject(LTGRAY_BRUSH));

	int offset = hasOrigin ? RULER_WIDTH : 0;

	double Height;
	if (dir == 2) Height = GetTransform().doubleDeScale(theHeight + offset);
	else Height = GetTransform().doubleDeScale(theHeight + RULER_WIDTH);

	CDPoint origin = m_pDesign->GetCurrentSheet()->GetOptions()->GetOrigin();
	int Scaling = m_pDesign->GetCurrentSheet()->GetOptions()->GetUnits() == 0 ? (PIXELSPERMM * 10) : (PIXELSPERMM * 254) / 10;
	int Step = Scaling;
	while (GetTransform().doubleScale(Step) < 25)
		Step *= 2;

	LONG Origin;
	LONG org;
	if (dir == 2)
	{
		Origin = static_cast<int> (GetTransform().GetOrigin().x);
		org = (long) m_pDesign->GetCurrentSheet()->GetOptions()->GetOrigin().x;
	}
	else
	{
		Origin = static_cast<int> (GetTransform().GetOrigin().y);
		org = (long) m_pDesign->GetCurrentSheet()->GetOptions()->GetOrigin().y;
	}
	org *= 10;

	Origin *= 10;
	Origin -= ( (Origin - org) % Step);

	// Draw the bounding rectangle
	dc.Rectangle(Size);

	//Origin -= 100;//m_pDesign->GetCurrentSheet()->GetOptions()->GetOrigin().y;

	// Draw the lines and numbers
	for (LONG Pos = Origin; Pos < Origin + Height * 10; Pos += Step)
	{
		int PosReal;
		BOOL UnitMarker = ( (Pos - org) % (Step * 10)) == 0;
		BOOL HalfMarker = ( (Pos - org) % (Step * 5)) == 0;

		int Len = UnitMarker || HalfMarker ? RULER_WIDTH / 2 : RULER_WIDTH / 4;

		if (dir == 2)
		{
			PosReal = GetTransform().Scale(CDPoint( (Pos) / 10, 0)).x - offset;
			dc.MoveTo(Size.left + PosReal, Size.top + RULER_WIDTH);
			dc.LineTo(Size.left + PosReal, Size.top + RULER_WIDTH - Len);
		}
		else
		{
			PosReal = GetTransform().Scale(CDPoint(0, (Pos) / 10)).y - RULER_WIDTH;
			dc.MoveTo(Size.right - RULER_WIDTH, Size.top + PosReal);
			dc.LineTo(Size.right - RULER_WIDTH + Len, Size.top + PosReal);
		}
		if (UnitMarker)
		{
			CString Buffer;
			Buffer.Format(_T("%d"), (Pos - org) / (m_pDesign->GetCurrentSheet()->GetOptions()->GetUnits() == 0 ? Scaling : Scaling * 10));

			if (dir == 2) dc.TextOut(Size.left + PosReal, Size.top + RULER_WIDTH - Len, Buffer);
			else dc.TextOut(Size.right - RULER_WIDTH + Len, Size.top + PosReal, Buffer);
		}
	}

	dc.SetROP2(R2_NOTXORPEN);

	if (dir == 2)
	{
		dc.MoveTo(oldPosition.x - offset, Size.top + RULER_WIDTH);
		dc.LineTo(oldPosition.x - offset, Size.top);
	}
	else
	{
		dc.MoveTo(Size.right - RULER_WIDTH, oldPosition.y - RULER_WIDTH);
		dc.LineTo(Size.right, oldPosition.y - RULER_WIDTH);
	}

	// Unselect the objects in use before deleting them
	dc.SelectObject(oldFont);
	dc.SelectObject(oldPen);
	dc.SelectObject(oldBrush);
}

BEGIN_MESSAGE_MAP( COriginButton, CButton )
	ON_BN_CLICKED(IDM_TOOLORIGIN, OnClick)
END_MESSAGE_MAP()

void COriginButton::OnClick()
{
	GetParent()->SendMessage(BN_CLICKED, IDM_TOOLORIGIN);
}

void COriginButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC); //Get device context object
	CRect rt;
	rt = lpDrawItemStruct->rcItem; //Get button rect

	dc.FillSolidRect(rt, RGB(192, 192, 192)); //Fill button with blue color

	UINT state = lpDrawItemStruct->itemState; //Get state of the button
	UINT offset = 0;
	if ( (state & ODS_SELECTED)) // If it is pressed
	{
		dc.DrawEdge(rt, EDGE_SUNKEN, BF_RECT); // Draw a sunken face
		offset = 1;
	}
	else
	{
		dc.DrawEdge(rt, EDGE_RAISED, BF_RECT); // Draw a raised face
	}

	dc.SelectStockObject(BLACK_PEN);
	dc.MoveTo(9 + offset, 4);
	dc.LineTo(9 + offset, 15);
	dc.MoveTo(4 + offset, 9);
	dc.LineTo(15 + offset, 9);

	dc.Detach();
}
