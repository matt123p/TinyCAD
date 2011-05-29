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

// This handles the actual drawing of objects

#include "stdafx.h"
#include <math.h>

#include "TinyCadView.h"
#include "diag.h"
#include "colour.h"
#include "option.h"
#include "ruler.h"

// This is used for the construction of this object
CDrawRuler::CDrawRuler(CTinyCadDoc *pDesign, BOOL new_horiz) :
	CDrawingObject(pDesign)
{
	m_point_a = CDPoint(0, 0);
	horiz = new_horiz;
}

CDrawRuler::~CDrawRuler()
{
}

BOOL CDrawRuler::IsConstruction()
{
	return TRUE;
}

ObjType CDrawRuler::GetType()
{
	return xRuler;
}

void CDrawRuler::Paint(CContext &dc, paint_options options)
{
	CDPoint text_point;

	switch (options)
	{
		case draw_selected:
			dc.SelectPen(PS_SOLID, 1, cRULER_CLK);
			dc.SetTextColor(cRULER_CLK);
			break;
		case draw_selectable:
			dc.SelectPen(PS_DOT, 1, cRULER_CLK);
			dc.SetTextColor(cRULER_CLK);
			break;
		default:
			dc.SelectPen(PS_DOT, 1, cRULER);
			dc.SetTextColor(cRULER);
	}

	CString pos = m_pDesign->GetOptions()->PointToDisplay(m_point_a - m_pDesign->GetOptions()->GetOrigin(), horiz);

	// Select the font
	dc.SelectFont(*m_pDesign->GetOptions()->GetFont(fRULE), horiz ? 2 : 0);

	CDSize text_size = dc.GetTextExtent(pos);

	// Draw this ruler, either horizontally or vertically
	if (horiz)
	{
		m_point_b = CDPoint(m_pDesign->GetDetails().GetPageBoundsAsPoint().x + 150, m_point_a.y);
		m_point_a = CDPoint(dc.GetTransform().DeScale(CPoint(0, 0)).x, m_point_a.y);
		dc.TextOut(m_point_a.x, m_point_a.y, pos);

		text_rect = CDRect(m_point_a.x, m_point_a.y, m_point_a.x + text_size.cx, m_point_a.y - text_size.cy);
	}
	else
	{
		m_point_a = CDPoint(m_point_a.x, dc.GetTransform().DeScale(CPoint(0, RULER_WIDTH)).y);
		m_point_b = CDPoint(m_point_a.x, m_pDesign->GetDetails().GetPageBoundsAsPoint().y + 150);
		dc.TextOut(m_point_a.x, m_point_a.y + text_size.cx, pos);
		text_rect = CDRect(m_point_a.x, m_point_a.y, m_point_a.x - text_size.cy, m_point_a.y + text_size.cx);
	}
	dc.MoveTo(m_point_a);
	dc.LineTo(m_point_b);
}

double CDrawRuler::DistanceFromPoint(CDPoint p)
{
	if (horiz)
	{
		return fabs(p.y - m_point_a.y);
	}
	else
	{
		return fabs(p.x - m_point_a.x);
	}
}

BOOL CDrawRuler::IsInside(double left, double right, double top, double bottom)
{
	if (horiz)
	{
		return top <= m_point_a.y && bottom >= m_point_a.y;
	}
	else
	{
		return left <= m_point_a.x && right >= m_point_a.x;
	}
}

int CDrawRuler::SetCursorEdit(CDPoint p)
{
	if (!IsInside(p.x, p.x, p.y, p.y))
	{
		return -1;
	}

	if (horiz)
	{
		return 4;
	}
	else
	{
		return 5;
	}
}

CDrawingObject *CDrawRuler::Store()
{
	CDrawRuler *NewObject;

	NewObject = new CDrawRuler(m_pDesign, horiz);
	*NewObject = *this;

	m_pDesign->Add(NewObject);

	return NewObject;
}

const TCHAR* CDrawRuler::GetXMLTag()
{
	return _T("RULER");
}

// Load and save to an XML file
void CDrawRuler::SaveXML(CXMLWriter &xml)
{
	xml.addTag(GetXMLTag());

	xml.addAttribute(_T("pos"), CDPoint(m_point_a));
	xml.addAttribute(_T("horizontal"), horiz);

	xml.closeTag();

}

void CDrawRuler::LoadXML(CXMLReader &xml)
{
	xml.getAttribute(_T("pos"), m_point_a);
	xml.getAttribute(_T("horizontal"), horiz);

	m_point_b = m_point_a;
}

void CDrawRuler::Load(CStream &archive)
{
	m_point_a = ReadPoint(archive);
	archive >> horiz;
}

void CDrawRuler::Display(BOOL erase)
{
	// Invalidate the symbol
	CDRect r(m_point_a.x, m_point_a.y, m_point_b.x, m_point_b.y);
	m_pDesign->InvalidateRect(r, erase, 4);
	m_pDesign->InvalidateRect(text_rect, erase, 4);
}

void CDrawRuler::LButtonDown(CDPoint p, CDPoint)
{
	// New undo level for each placement...
	m_pDesign->BeginNewChangeSet();

	Display();
	m_point_a = p;
	m_point_b = p;
	Store();
	Display(); // Write to screen

}

void CDrawRuler::Move(CDPoint p, CDPoint no_snap_p)
{
	Display();

	if (horiz)
	{
		p.x = m_point_a.x;
		m_point_b += m_point_a - p;
	}
	else
	{
		p.y = m_point_a.y;
		m_point_b += m_point_a - p;
	}
	m_point_a = p;

	Display();
}

CString CDrawRuler::GetName() const
{
	return "Ruler";
}

int CDrawRuler::getMenuID()
{
	return horiz ? ID_RULER_HORIZ : ID_RULER_VERT;
}
