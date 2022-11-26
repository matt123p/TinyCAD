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
#include "LineUtils.h"
#include <math.h>

////// The Text Class //////


void CDrawLabel::TagResources()
{
	m_pDesign->GetOptions()->TagFont(FontStyle);
}

BOOL CDrawLabel::CanEdit()
{
	return TRUE;
}

// Look for a search string in the object
CString CDrawLabel::Find(const TCHAR *theSearchString)
{
	CString HoldString;

	HoldString = str;
	HoldString.MakeLower();

	// Now look for the search string in this string
	return (HoldString.Find(theSearchString) != -1) ? str : "";
}

void CDrawLabel::NewOptions()
{
	CalcLayout();
	Display();
	FontStyle = m_pDesign->GetOptions()->GetCurrentFont(GetType());
	original_width = 0;
	CalcLayout();
	Display();
}

UINT CDrawLabel::getMenuID()
{
	//		case xLabelEx2:
	return IDM_TOOLLABEL;
}

const TCHAR* CDrawLabel::GetXMLTag()
{
	return _T("LABEL");
}

// Load and save to an XML file
void CDrawLabel::SaveXML(CXMLWriter &xml)
{
	if (!IsEmpty())
	{
		xml.addTag(GetXMLTag());

		xml.addAttribute(_T("pos"), CDPoint(m_active_point));
		xml.addAttribute(_T("direction"), dir);
		xml.addAttribute(_T("font"), FontStyle);
		xml.addAttribute(_T("color"), FontColour);
		xml.addAttribute(_T("style"), m_Style);
		xml.addChildData(str);
		xml.closeTag();
	}
}

void CDrawLabel::LoadXML(CXMLReader &xml)
{
	int style = label_normal;

	xml.getAttribute(_T("pos"), m_active_point);
	xml.getAttribute(_T("direction"), dir);
	xml.getAttribute(_T("font"), FontStyle);
	xml.getAttribute(_T("color"), FontColour);
	xml.getAttribute(_T("style"), style);
	xml.getChildData(str);

	m_Style = static_cast<label_style> (style);

	FontStyle = m_pDesign->GetOptions()->GetNewFontNumber(FontStyle);

	m_segment = 0;
	CalcLayout();
}

void CDrawLabel::Load(CStream &archive, ObjType xtype)
{
	m_Style = label_normal;

	m_active_point = ReadPoint(archive);
	archive >> dir >> str;
	switch (xtype)
	{
		case xLabelEx:
			// Load the font number in
			archive >> FontStyle;

			// Convert the font number over
			FontStyle = m_pDesign->GetOptions()->GetNewFontNumber(FontStyle);
			break;
		case xLabelEx2:
			// Load the font number in
			archive >> FontStyle;

			// Convert the font number over
			FontStyle = m_pDesign->GetOptions()->GetNewFontNumber(FontStyle);

			// Load in the font colour...
			archive >> FontColour;
			break;
		case xLabel:
			FontStyle = fTEXT;
			break;
	}

	switch (xtype)
	{
		case xLabel:
		case xLabelEx:
			FontColour = m_pDesign->GetOptions()->GetUserColor().Get(CUserColor::LABEL);
			break;
	}

	CDSize size = m_pDesign->GetTextExtent(str.GetLength() ? str : " ", FontStyle);

	if (dir >= 2)
	{
		m_point_b = CDPoint(m_point_a.x + size.cx, m_point_a.y - size.cy);
	}
	else
	{
		m_point_b = CDPoint(m_point_a.x - size.cy, m_point_a.y - size.cx);
	}

	m_segment = 0;
	CalcLayout();
}

ObjType CDrawLabel::GetType()
{
	return xLabelEx2;
}

CString CDrawLabel::GetName() const
{
	return "Label";
}

void CDrawLabel::SetScalingWidths()
{
	if (dir >= 2)
	{
		original_box_width = m_point_b.x - m_point_a.x;
	}
	else
	{
		original_box_width = m_point_b.y - m_point_a.y;
	}
	target_box_width = original_box_width;
	original_width = m_pDesign->GetOptions()->GetFontWidth(FontStyle);
}

void CDrawLabel::BeginEdit(BOOL re_edit)
{
	g_EditToolBar.m_LabelEdit.Open(m_pDesign, this);
	m_pDesign->GetOptions()->SetCurrentFont(GetType(), FontStyle);
	SetScalingWidths();
}

void CDrawLabel::EndEdit()
{
	RButtonDown(CDPoint(0, 0), CDPoint(0, 0));
	g_EditToolBar.m_LabelEdit.Close();
}

void CDrawLabel::Shift(CDPoint r)
{
	CDrawingObject::Shift(r);
	m_active_point += r;
	CalcLayout();
}

void CDrawLabel::Move(CDPoint p, CDPoint no_snap_p)
{
	Display();

	// Now stick it...
	m_active_point = m_pDesign->GetStickyPoint(no_snap_p, FALSE, TRUE, is_stuck, is_junction);
	if (m_Style == label_normal)
	{
		is_junction = FALSE;
	}
	CalcLayout();

	m_segment = 0;
	Display();
}

void CDrawLabel::MoveField(int w, CDPoint r)
{
	Display();

	CDRect rect;

	if (w == CRectTracker::hitMiddle || w == 11)
	{
		m_active_point += r;
	}
	else
	{
		if (dir >= 2)
		{
			rect = CDRect(m_point_a.x, m_point_b.y, m_point_a.x + target_box_width, m_point_a.y);
		}
		else
		{
			rect = CDRect(m_point_b.x, m_point_a.y + target_box_width, m_point_a.x, m_point_a.y);
		}

		switch (w)
		{
			case CRectTracker::hitTopLeft:
				rect.left += r.x;
				rect.top += r.y;
				r.y = -r.y;
				break;
			case CRectTracker::hitTopRight:
				rect.right += r.x;
				rect.top += r.y;
				r.y = -r.y;
				r.x = -r.x;
				break;
			case CRectTracker::hitBottomRight:
				rect.right += r.x;
				rect.bottom += r.y;
				r.x = -r.x;
				break;
			case CRectTracker::hitBottomLeft:
				rect.left += r.x;
				rect.bottom += r.y;
				break;
			case CRectTracker::hitTop:
				rect.top += r.y;
				r.y = -r.y;
				break;
			case CRectTracker::hitRight:
				rect.right += r.x;
				r.x = -r.x;
				break;
			case CRectTracker::hitBottom:
				rect.bottom += r.y;
				break;
			case CRectTracker::hitLeft:
				rect.left += r.x;
				break;
		}

		if (m_Style == label_normal)
		{
			if (dir >= 2)
			{
				m_active_point.x = rect.left;
				m_active_point.y = rect.bottom;
			}
			else
			{
				m_active_point.y = rect.bottom;
				m_active_point.x = rect.right;
			}
		}
		else
		{
		}

		if (dir >= 2)
		{
			target_box_width = rect.Width();
		}
		else
		{
			target_box_width = -rect.Height();
		}

		if (original_box_width > 0)
		{
			FontStyle = m_pDesign->GetOptions()->ChangeFontSize(FontStyle, dir < 2 ? r.x : -r.y, (original_width * target_box_width) / original_box_width);
		}

		CalcLayout();
		Display();
	}
}

void CDrawLabel::LButtonDown(CDPoint p, CDPoint no_snap_p)
{
	// We can only store, if we have some text!
	if (str.IsEmpty())
	{
		g_EditToolBar.m_LabelEdit.ReFocus();
		return;
	}

	// New undo level for each placement...
	m_pDesign->BeginNewChangeSet();

	Display();
	p = m_pDesign->GetStickyPoint(no_snap_p, FALSE, TRUE, is_stuck, is_junction);
	if (m_Style == label_normal)
	{
		is_junction = FALSE;
	}
	m_point_a = p;
	Store();
	Display(); // Write to screen

	// Now increment the number as necessary
	int hold = str.FindOneOf(_T("0123456789-"));
	// Only increment if old value was a number at end of string
	if (hold != -1)
	{
		TCHAR NewNum[17];
		_itot_s(_tstoi(str.Mid(hold)) + m_pDesign->GetNameDir(), NewNum, 10);
		str = str.Left(hold) + NewNum;
	}
	g_EditToolBar.m_LabelEdit.ReFocus();
	Display();
}

// The constructors

CDrawLabel::CDrawLabel(CTinyCadDoc *pDesign) :
	CDrawRectOutline(pDesign)
{
	is_stuck = FALSE;
	is_junction = FALSE;
	dir = 3;
	str = "";
	m_segment = 1;
	m_point_a = m_point_b = CDPoint(0, 0);
	original_width = 0;
	original_box_width = 0;
	target_box_width = 0;
	FontStyle = m_pDesign->GetOptions()->GetCurrentFont(GetType());
	FontColour = m_pDesign->GetOptions()->GetUserColor().Get(CUserColor::LABEL);
	m_Style = label_normal;
}

void CDrawLabel::Display(BOOL erase)
{
	CDRect r(m_point_a.x, m_point_a.y, m_point_b.x, m_point_b.y);
	m_pDesign->InvalidateRect(r, erase, 8);
	if (is_stuck)
	{
		r.left = m_active_point.x - HIGHLIGHT_SIZE;
		r.right = m_active_point.x + HIGHLIGHT_SIZE;
		r.top = m_active_point.y - HIGHLIGHT_SIZE;
		r.bottom = m_active_point.y + HIGHLIGHT_SIZE;
		m_pDesign->InvalidateRect(r, erase, 3);
	}

}

void CDrawLabel::CalcLayout()
{
	CDSize size = m_pDesign->GetTextExtent(str.GetLength() ? str : " ", FontStyle);

	if (original_width == 0)
	{
		SetScalingWidths();
	}

	// Now draw the surrounding lines for input & output labels
	if (m_Style != label_normal)
	{
		double spacing = size.cy / 5;
		double arrow_distance = size.cy;
		double extra_width = 0;
		switch (m_Style)
		{
			case label_in:
			case label_out:
				extra_width += arrow_distance + spacing;
				break;
			case label_io:
				extra_width += arrow_distance * 2;
				break;
		}

		switch (dir)
		{
			case 0: // Up
				m_point_a = CDPoint(m_active_point.x + size.cy / 2, m_active_point.y + size.cx + extra_width);
				m_point_b = CDPoint(m_point_a.x - size.cy, m_point_a.y - size.cx - extra_width);
				break;
			case 1: // Down
				m_point_a = CDPoint(m_active_point.x + size.cy / 2, m_active_point.y - size.cx - extra_width);
				m_point_b = CDPoint(m_point_a.x - size.cy, m_point_a.y + size.cx + extra_width);
				break;
			case 2: // Left
				m_point_a = CDPoint(m_active_point.x, m_active_point.y + size.cy / 2);
				m_point_b = CDPoint(m_point_a.x + size.cx + extra_width, m_active_point.y - size.cy / 2);
				break;
			case 3: // Right
				m_point_a = CDPoint(m_active_point.x - size.cx - extra_width, m_active_point.y + size.cy / 2);
				m_point_b = CDPoint(m_point_a.x + size.cx + extra_width, m_active_point.y - size.cy / 2);
				break;
		}
	}
	else
	{
		m_point_a = m_active_point;
		switch (dir)
		{
			case 0: // Up
				m_point_b = CDPoint(m_point_a.x - size.cy, m_point_a.y - size.cx);
				break;
			case 1: // Down
				m_point_b = CDPoint(m_point_a.x - size.cy, m_point_a.y + size.cx);
				break;
			case 2: // Left
				m_point_b = CDPoint(m_point_a.x - size.cx, m_point_a.y - size.cy);
				break;
			case 3: // Right
				m_point_b = CDPoint(m_point_a.x + size.cx, m_point_a.y - size.cy);
				break;
		}
	}

}

// Display the text on the screen!
void CDrawLabel::Paint(CContext &dc, paint_options options)
{
	CalcLayout();

	dc.SelectFont(*m_pDesign->GetOptions()->GetFont(FontStyle), dir);
	CDSize size = dc.GetTextExtent(str.GetLength() ? str : " ");

	// Now draw the surrounding lines for input & output labels
	if (m_Style != label_normal)
	{
		// Draw the top and bottom lines
		dc.SelectPen(PS_SOLID, 1, RGB(0,0,0), options);
		double spacing = size.cy / 5;

		double arrow_distance = size.cy;
		double i_spacing = 0;
		double o_spacing = 0;
		switch (dir)
		{
			case 0: // Up
			case 3: // Right
				i_spacing = ! (m_Style == label_in || m_Style == label_io) ? 0 : -arrow_distance;
				o_spacing = ! (m_Style == label_out || m_Style == label_io) ? 0 : -arrow_distance;
				break;
			case 1: // Down
			case 2: // Left
				i_spacing = ! (m_Style == label_in || m_Style == label_io) ? 0 : arrow_distance;
				o_spacing = ! (m_Style == label_out || m_Style == label_io) ? 0 : arrow_distance;
				break;
		}

		switch (dir)
		{
			case 0: // Up
				// Top Line
				dc.MoveTo(CDPoint(m_point_b.x, m_point_a.y + o_spacing));
				dc.LineTo(CDPoint(m_point_b.x, m_point_b.y - i_spacing));

				// Bottom line
				dc.MoveTo(CDPoint(m_point_a.x, m_point_a.y + o_spacing));
				dc.LineTo(CDPoint(m_point_a.x, m_point_b.y - i_spacing));
				break;
			case 1: // Down
				// Top Line
				dc.MoveTo(CDPoint(m_point_b.x, m_point_a.y + o_spacing));
				dc.LineTo(CDPoint(m_point_b.x, m_point_b.y - i_spacing));

				// Bottom line
				dc.MoveTo(CDPoint(m_point_a.x, m_point_a.y + o_spacing));
				dc.LineTo(CDPoint(m_point_a.x, m_point_b.y - i_spacing));
				break;
			case 2: // Left
				// Top Line
				dc.MoveTo(CDPoint(m_point_b.x - o_spacing, m_point_b.y));
				dc.LineTo(CDPoint(m_point_a.x + i_spacing, m_point_b.y));

				// Bottom line
				dc.MoveTo(CDPoint(m_point_b.x - o_spacing, m_point_a.y));
				dc.LineTo(CDPoint(m_point_a.x + i_spacing, m_point_a.y));
				break;
			case 3: // Right
				// Top Line
				dc.MoveTo(CDPoint(m_point_a.x - o_spacing, m_point_b.y));
				dc.LineTo(CDPoint(m_point_b.x + i_spacing, m_point_b.y));

				// Bottom line
				dc.MoveTo(CDPoint(m_point_a.x - o_spacing, m_point_a.y));
				dc.LineTo(CDPoint(m_point_b.x + i_spacing, m_point_a.y));
				break;

		}

		// Draw the "input" arrow if this is an input
		switch (dir)
		{
			case 0: // Up
				if (m_Style == label_in || m_Style == label_io)
				{
					dc.MoveTo(CDPoint(m_point_a.x, m_point_b.y + arrow_distance));
					dc.LineTo(CDPoint(m_point_a.x - arrow_distance / 2, m_point_b.y));
					dc.LineTo(CDPoint(m_point_b.x, m_point_b.y + arrow_distance));
				}
				else
				{
					dc.MoveTo(CDPoint(m_point_a.x, m_point_b.y));
					dc.LineTo(CDPoint(m_point_b.x, m_point_b.y));
				}
				break;
			case 1: // Down
				if (m_Style == label_in || m_Style == label_io)
				{
					dc.MoveTo(CDPoint(m_point_a.x, m_point_b.y - arrow_distance));
					dc.LineTo(CDPoint(m_point_a.x - arrow_distance / 2, m_point_b.y));
					dc.LineTo(CDPoint(m_point_b.x, m_point_b.y - arrow_distance));
				}
				else
				{
					dc.MoveTo(CDPoint(m_point_a.x, m_point_b.y));
					dc.LineTo(CDPoint(m_point_b.x, m_point_b.y));
				}
				break;
			case 2: // Left
				if (m_Style == label_in || m_Style == label_io)
				{
					dc.MoveTo(CDPoint(m_point_a.x + arrow_distance, m_point_b.y));
					dc.LineTo(CDPoint(m_point_a.x, m_point_a.y - arrow_distance / 2));
					dc.LineTo(CDPoint(m_point_a.x + arrow_distance, m_point_a.y));
				}
				else
				{
					dc.MoveTo(CDPoint(m_point_a.x, m_point_a.y));
					dc.LineTo(CDPoint(m_point_a.x, m_point_b.y));
				}
				break;
			case 3: // Right
				if (m_Style == label_in || m_Style == label_io)
				{
					dc.MoveTo(CDPoint(m_point_b.x - arrow_distance, m_point_a.y));
					dc.LineTo(CDPoint(m_point_b.x, m_point_a.y - arrow_distance / 2));
					dc.LineTo(CDPoint(m_point_b.x - arrow_distance, m_point_b.y));
				}
				else
				{
					dc.MoveTo(CDPoint(m_point_b.x, m_point_a.y));
					dc.LineTo(CDPoint(m_point_b.x, m_point_b.y));
				}
				break;

		}

		// Draw the "ouput" arrow if this is an input
		switch (dir)
		{
			case 0: // Up
				if (m_Style == label_out || m_Style == label_io)
				{
					dc.MoveTo(CDPoint(m_point_a.x, m_point_a.y - arrow_distance));
					dc.LineTo(CDPoint(m_point_a.x - arrow_distance / 2, m_point_a.y));
					dc.LineTo(CDPoint(m_point_b.x, m_point_a.y - arrow_distance));
				}
				else
				{
					dc.MoveTo(CDPoint(m_point_a.x, m_point_a.y));
					dc.LineTo(CDPoint(m_point_b.x, m_point_a.y));
				}
				break;
			case 1: // Down
				if (m_Style == label_out || m_Style == label_io)
				{
					dc.MoveTo(CDPoint(m_point_a.x, m_point_a.y + arrow_distance));
					dc.LineTo(CDPoint(m_point_a.x - arrow_distance / 2, m_point_a.y));
					dc.LineTo(CDPoint(m_point_b.x, m_point_a.y + arrow_distance));
				}
				else
				{
					dc.MoveTo(CDPoint(m_point_a.x, m_point_a.y));
					dc.LineTo(CDPoint(m_point_b.x, m_point_a.y));
				}
				break;
			case 2: // Left
				if (m_Style == label_out || m_Style == label_io)
				{
					dc.MoveTo(CDPoint(m_point_b.x - arrow_distance, m_point_b.y));
					dc.LineTo(CDPoint(m_point_b.x, m_point_a.y - arrow_distance / 2));
					dc.LineTo(CDPoint(m_point_b.x - arrow_distance, m_point_a.y));
				}
				else
				{
					dc.MoveTo(CDPoint(m_point_b.x, m_point_a.y));
					dc.LineTo(CDPoint(m_point_b.x, m_point_b.y));
				}
				break;
			case 3: // Right
				if (m_Style == label_out || m_Style == label_io)
				{
					dc.MoveTo(CDPoint(m_point_a.x + arrow_distance, m_point_a.y));
					dc.LineTo(CDPoint(m_point_a.x, m_point_a.y - arrow_distance / 2));
					dc.LineTo(CDPoint(m_point_a.x + arrow_distance, m_point_b.y));
				}
				else
				{
					dc.MoveTo(CDPoint(m_point_a.x, m_point_a.y));
					dc.LineTo(CDPoint(m_point_a.x, m_point_b.y));
				}
				break;

		}

		dc.SetTextColor(FontColour);

		if (o_spacing == 0)
		{
			switch (dir)
			{
				case 3:
				case 0:
					o_spacing = -spacing;
					break;
				default:
					o_spacing = spacing;
			}
		}
		if (i_spacing == 0)
		{
			i_spacing = spacing;
		}

		switch (dir)
		{
			case 0: // Up
				dc.TextOut(str, CDPoint(m_point_a.x, m_point_a.y + o_spacing), options, dir);
				break;
			case 1: // Down
				dc.TextOut(str, CDPoint(m_point_a.x, m_point_a.y + o_spacing), options, dir);
				break;
			case 2: // Left
				dc.TextOut(str, CDPoint(m_point_b.x - o_spacing, m_point_a.y), options, dir);
				break;
			case 3: // Right
				dc.TextOut(str, CDPoint(m_point_a.x - o_spacing, m_point_a.y), options, dir);
				break;
		}
	}
	else
	{	//normal net label
		dc.SetTextColor(FontColour);
		dc.TextOut(str, m_point_a, options, dir);
	}

	// Draw a little blob, so the user knows where it
	// is stuck to
	dc.SelectBrush();
	dc.SelectPen(PS_SOLID, 1, cBOLD);
	dc.Rectangle(CDRect(m_active_point.x - 2, m_active_point.y - 2, m_active_point.x + 2, m_active_point.y + 2));

	if (is_stuck)
	{
		// Draw a nice circle to show the stickness...
		dc.PaintConnectPoint(m_active_point);

		// Do we need a junction
		if (is_junction)
		{
			int js = JUNCTION_SIZE;
			CDPoint br, tl;
			br = CDPoint(m_active_point.x + js, m_active_point.y + js);
			tl = CDPoint(m_active_point.x - js, m_active_point.y - js);

			dc.SetROP2(R2_COPYPEN);

			dc.SelectPen(PS_SOLID, 1, m_pDesign->GetOptions()->GetUserColor().Get(CUserColor::JUNCTION));
			dc.SelectBrush(m_pDesign->GetOptions()->GetUserColor().Get(CUserColor::JUNCTION));
			dc.Ellipse1(CDRect(tl.x, tl.y, br.x, br.y));
		}
	}

}

// Store the text in the drawing
CDrawingObject* CDrawLabel::Store()
{
	CDrawLabel *NewObject;

	NewObject = new CDrawLabel(m_pDesign);
	*NewObject = *this;
	NewObject->is_stuck = FALSE;
	NewObject->is_junction = FALSE;

	m_pDesign->Add(NewObject);
	if (is_junction)
	{
		m_pDesign->AddNewJunction(m_point_a);
	}

	return NewObject;
}

int CDrawLabel::DoRotate(int olddir, int newdir)
{
	//New rotation=>2  3  4
	//           Current dir ..\/..
	int table[] = {2, 3, 0, // 0 (Up)
	               3, 2, 1, // 1 (Down)
	               1, 0, 3, // 2 (Left)
	               0, 1, 2, // 3 (Right)
	        };

	return table[ (newdir - 2) + olddir * 3];
}

// Rotate this object about a point
void CDrawLabel::Rotate(CDPoint p, int ndir)
{
	// Rotate bounding box only if we have a centre point
	if (p != CDPoint(0, 0))
	{
		// Translate this point so the rotational point is the origin
		m_active_point = CDPoint(m_active_point.x - p.x, m_active_point.y - p.y);

		// Perfrom the rotation
		switch (ndir)
		{
			case 2: // Left
				m_active_point = CDPoint(m_active_point.y, -m_active_point.x);
				break;
			case 3: // Right
				m_active_point = CDPoint(-m_active_point.y, m_active_point.x);
				break;
			case 4: // Mirror
				m_active_point = CDPoint(-m_active_point.x, m_active_point.y);
				break;
		}

		// Re-translate the points back to the original location
		m_active_point = CDPoint(m_active_point.x + p.x, m_active_point.y + p.y);
	}

	dir = (BYTE) DoRotate(dir, ndir);

	CalcLayout();
}

// Extract the netlist/active points from this object
void CDrawLabel::GetActiveListFirst(CActiveNode &a)
{
}

bool CDrawLabel::GetActive(CActiveNode &a)
{
	if (a.m_sent > 0 || m_Style == label_normal)
	{
		return false;
	}

	a.m_a = m_active_point;
	++a.m_sent;
	return true;
}

int CDrawLabel::IsInsideField(CDPoint p)
{
	int r = CDrawRectOutline::IsInsideField(p);
	if (r == CRectTracker::hitMiddle || r == 11)
	{
		r = -1;
	}

	return r;
}

BOOL CDrawLabel::IsEmpty()
{
	if (m_Style == label_normal && str.IsEmpty())
	{
		return TRUE;
	}

	return FALSE;
}
