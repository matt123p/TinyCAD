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
#include "TinyCadView.h"
#include "diag.h"
#include "colour.h"
#include "option.h"

// The editing dialogs
extern CDlgERCListBox theERCListBox;

/////// The Object classes /////////


// The Error Class //

#define ERRORSIZE	6

CDrawError::CDrawError(CTinyCadDoc *pDesign, CDPoint NewA, int q) :
	CDrawingObject(pDesign)
{
	m_point_a = NewA;
	m_point_b = NewA;
	m_segment = 0;
	ErrorNumber = q;
}

ObjType CDrawError::GetType()
{
	return xError;
}

CString CDrawError::GetName() const
{
	return "Error";
}

BOOL CDrawError::IsInside(double left, double right, double top, double bottom)
{
	return ! ( (m_point_a.x - ERRORSIZE < left && m_point_a.x + ERRORSIZE <= left) || (m_point_a.x - ERRORSIZE > right && m_point_a.x + ERRORSIZE >= right) || (m_point_a.y - ERRORSIZE < top && m_point_a.y + ERRORSIZE <= top) || (m_point_a.y - ERRORSIZE > bottom && m_point_a.y + ERRORSIZE >= bottom));
}

void CDrawError::Paint(CContext &dc, paint_options options)
{
	dc.SelectBrush();

	switch (options)
	{
		case draw_selected:
			dc.SelectPen(PS_SOLID, 5, m_pDesign->GetOptions()->GetUserColor().Get(CUserColor::BUS));
			break;
		case draw_selectable:
			dc.SelectPen(PS_SOLID, 5, cBUS_CLK);
			break;
		default:
			dc.SelectPen(PS_SOLID, 5, cERROR);
	}

	dc.SetROP2(R2_COPYPEN);

	dc.Ellipse1(CDRect(m_point_a.x + ERRORSIZE, m_point_a.y + ERRORSIZE, m_point_a.x - ERRORSIZE, m_point_a.y - ERRORSIZE));
}

void CDrawError::BeginEdit(BOOL re_edit)
{
	theERCListBox.SetSelect(ErrorNumber);
	Display(FALSE);
}

void CDrawError::EndEdit()
{
}

BOOL CDrawError::CanEdit()
{
	return TRUE;
}

// Store the error in the design
CDrawingObject* CDrawError::Store()
{
	CDrawError *NewObject;

	NewObject = new CDrawError(*this);

	m_pDesign->Add(NewObject);

	m_segment = 1;

	return NewObject;
}

void CDrawError::Display(BOOL erase)
{
	// Invalidate the symbol
	CDRect r(m_point_a.x, m_point_a.y, m_point_b.x, m_point_b.y);
	m_pDesign->InvalidateRect(r, erase, 10);
}

////// The tag object type //////


// Load the object to a file
void CDrawTag::Load(CStream& archive)
{
	m_point_a = m_point_b = ReadPoint(archive);
	archive >> TagName;
}

const TCHAR* CDrawTag::GetXMLTag()
{
	return _T("TAG");
}

// Load and save to an XML file
void CDrawTag::SaveXML(CXMLWriter &xml)
{
	xml.addTag(GetXMLTag());

	xml.addAttribute(_T("pos"), CDPoint(m_point_a));
	xml.addChildData(TagName);

	xml.closeTag();

}

void CDrawTag::LoadXML(CXMLReader &xml)
{
	xml.getAttribute(_T("pos"), m_point_a);
	xml.getChildData(TagName);

	m_point_b = m_point_a;
}

