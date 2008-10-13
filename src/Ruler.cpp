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
Ruler::Ruler(CMultiSheetDoc *pDesign, int nDir, CRect nSize, CWnd *parent)
{
	m_pDesign = pDesign;
	oldPosition = CPoint(0,0);
	dir = nDir;
	Size = nSize;

	if (dir == 2) {
		Size = CRect(Size.left, Size.top, Size.right, Size.top + RULER_WIDTH);
		theHeight = Size.right-Size.left;
		// Now create the ruler
		Create(NULL, _T(""), WS_CHILD | WS_VISIBLE , Size, parent, NULL);
	} else	{
		Size = CRect(Size.right - RULER_WIDTH, Size.top + RULER_WIDTH, Size.right, Size.bottom );
		theHeight = Size.bottom-Size.top;
		// Now create the ruler
		Create(NULL, _T(""), WS_CHILD | WS_VISIBLE , Size, parent, NULL);
		Size = CRect(0,0,RULER_WIDTH,theHeight);
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

	CPen *oldPen = (CPen *)(dc.SelectStockObject(BLACK_PEN));
	dc.SetROP2(R2_NOTXORPEN);
	dc.SetBkMode(OPAQUE);

	// Remove the old marker
	if (dir == 2) {
		dc.MoveTo(oldPosition.x ,Size.top + RULER_WIDTH);
		dc.LineTo(oldPosition.x ,Size.top);
	} else {
		dc.MoveTo(Size.right - RULER_WIDTH, oldPosition.y - RULER_WIDTH);
		dc.LineTo(Size.right, oldPosition.y  - RULER_WIDTH);
	}

	// Now draw in the new marker
	if (dir == 2) {
		dc.MoveTo(r.x ,Size.top + RULER_WIDTH);
		dc.LineTo(r.x ,Size.top);
	} else {
		dc.MoveTo(Size.right - RULER_WIDTH, r.y  - RULER_WIDTH);
		dc.LineTo(Size.right, r.y  - RULER_WIDTH);
	}

	// Store when the marker is currently drawn
	oldPosition = r;
	
	dc.SelectObject(oldPen);
}


void Ruler::OnNewSize(CRect nSize)
{
	Size = nSize;

	if (dir == 2) {
		Size = CRect(Size.left, Size.top, Size.right, Size.top + RULER_WIDTH);
		theHeight = Size.right-Size.left;
		MoveWindow(Size);
	} else	{
		Size = CRect(Size.right - RULER_WIDTH, Size.top + RULER_WIDTH, Size.right, Size.bottom);
		MoveWindow(Size);
		theHeight = Size.bottom-Size.top;
		Size = CRect(0,0,RULER_WIDTH,theHeight);
	}

}


// Draw the detail in the ruler
void Ruler::OnPaint()
{
	CPaintDC dc(this);

	CFont theFont;
	theFont.CreateFont(14,0,(dir == 2) ? 0 : 2700,0,400,FALSE,FALSE,FALSE,
		ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS | CLIP_LH_ANGLES,
		DEFAULT_QUALITY,DEFAULT_PITCH | FF_MODERN,_T("Courier New"));

	// Select the objects to draw with
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextAlign(TA_CENTER | TA_BASELINE | TA_NOUPDATECP);
	dc.SetTextColor(RGB(0,128,64));
	dc.SetROP2(R2_COPYPEN);
	
	CFont *oldFont =   (CFont *) (dc.SelectObject(&theFont));
	CPen *oldPen =     (CPen *)  (dc.SelectStockObject(BLACK_PEN));
	CBrush *oldBrush = (CBrush *)(dc.SelectStockObject(LTGRAY_BRUSH));

	double Height;
	if (dir == 2)
		Height = GetTransform().doubleDeScale(theHeight);
	else
		Height = GetTransform().doubleDeScale(theHeight + RULER_WIDTH);


  	int Scaling = m_pDesign->GetCurrentSheet()->GetOptions()->GetUnits()==0 ? (PIXELSPERMM*10) : (PIXELSPERMM * 254)/10;
	int Step = Scaling;
	while (GetTransform().doubleScale(Step) < 25)
		Step *= 2;

	
	LONG Origin;
	if (dir == 2)
		Origin = static_cast<int>(GetTransform().GetOrigin().x);
	else
		Origin = static_cast<int>(GetTransform().GetOrigin().y);
	Origin = Origin*10;
	Origin = Origin - (Origin % Step);


	// Draw the bounding rectangle
	dc.Rectangle(Size);

	// Draw the lines and numbers
	for (LONG Pos = Origin; Pos < Origin+Height*10; Pos += Step) {
		int PosReal;
		BOOL UnitMarker = (Pos % (Step*10)) == 0;

		int Len = UnitMarker ? RULER_WIDTH/2 : RULER_WIDTH/4;
		if (dir == 2) {
			PosReal = GetTransform().Scale(CDPoint(Pos / 10, 0)).x;
			dc.MoveTo(Size.left + PosReal,Size.top + RULER_WIDTH);
			dc.LineTo(Size.left + PosReal,Size.top + RULER_WIDTH - Len);
		} else {
			PosReal = GetTransform().Scale(CDPoint(0,Pos / 10)).y - RULER_WIDTH;
			dc.MoveTo(Size.right - RULER_WIDTH, Size.top + PosReal);
			dc.LineTo(Size.right - RULER_WIDTH + Len, Size.top + PosReal);
		}
		if (UnitMarker) 
		{
			CString Buffer;
  			Buffer.Format(_T("%d"),Pos/ (m_pDesign->GetCurrentSheet()->GetOptions()->GetUnits()==0 ? Scaling : Scaling * 10) );

			if (dir == 2)
				dc.TextOut(Size.left + PosReal, Size.top + RULER_WIDTH - Len, Buffer);
			else
				dc.TextOut(Size.right - RULER_WIDTH + Len, Size.top + PosReal, Buffer);
		}
	}

	dc.SetROP2(R2_NOTXORPEN);

	if (dir == 2) {
		dc.MoveTo(oldPosition.x ,Size.top + RULER_WIDTH);
		dc.LineTo(oldPosition.x ,Size.top);
	} else {
		dc.MoveTo(Size.right - RULER_WIDTH, oldPosition.y - RULER_WIDTH);
		dc.LineTo(Size.right, oldPosition.y - RULER_WIDTH);
	}
	
	
	// Unselect the objects in use before deleting them
	dc.SelectObject(oldFont);
	dc.SelectObject(oldPen);
	dc.SelectObject(oldBrush);
}

