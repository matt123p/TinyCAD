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

// The NoteText Class //

// The Constructor
CDrawNoteText::CDrawNoteText(CTinyCadDoc *pDesign, ObjType type) :
	CDrawRectOutline(pDesign)
{
	m_type = type;
	m_segment = 1;
	Style = m_pDesign->GetOptions()->GetCurrentStyle(GetType());
	Fill = m_pDesign->GetOptions()->GetCurrentFillStyle(GetType());
	Font = m_pDesign->GetOptions()->GetCurrentFont(GetType());
	m_note_text = _T("Default sample text\nwith multiple lines.\nAnd a 3rd line.\nAnd a 4th extra long line.");		//Default text for the note.
}

ObjType CDrawNoteText::GetType()
{
	return m_type;
}

CString CDrawNoteText::GetName() const
{
	return "NoteText";
}

void CDrawNoteText::BeginEdit(BOOL re_edit)
{
	m_segment = 1;

	m_pDesign->GetOptions()->SetCurrentStyle(GetType(), Style);
	m_pDesign->GetOptions()->SetCurrentFillStyle(GetType(), Fill);

	m_re_edit = re_edit;
	if (re_edit)
	{
		g_EditToolBar.m_PolygonEdit.Open(m_pDesign, this);
	}
}

void CDrawNoteText::EndEdit()
{
	Display();
	if (m_re_edit)
	{
		g_EditToolBar.m_PolygonEdit.Close();
	}

}

// Place the object into the design
void CDrawNoteText::LButtonDown(CDPoint p, CDPoint s)
{
	// New undo level for each placement...
	m_pDesign->BeginNewChangeSet();

	if (m_segment)
	{
		m_point_a = p;
		m_point_b = p;
		m_segment = !m_segment;
		Display();
	}
	else
	{
		m_point_b = p;
		Display();
		CDrawingObject *pObject = Store();

		RButtonDown(p, s);

		// Now select the top object and switch back
		// to the Edit tool
		m_pDesign->UnSelect();
		m_pDesign->Select(pObject);
		m_pDesign->SelectObject(new CDrawEditItem(m_pDesign));
	}
}

void CDrawNoteText::LButtonUp(CDPoint p, CDPoint)
{
	// If the user has made a significant movement,
	// then consider this a placement....
	double offset_x = m_point_a.x - m_point_b.x;
	double offset_y = m_point_a.y - m_point_b.y;

	if (offset_x < 0)
	{
		offset_x = -offset_x;
	}
	if (offset_y < 0)
	{
		offset_y = -offset_y;
	}

	if (offset_x > 10 || offset_y > 10)
	{
		// Second line..
		LButtonDown(p, p);
	}
}

void CDrawNoteText::TagResources()
{
	m_pDesign->GetOptions()->TagStyle(Style);
	m_pDesign->GetOptions()->TagFillStyle(Fill);
}

BOOL CDrawNoteText::RButtonDown(CDPoint p, CDPoint s)
{
	BOOL r = !m_segment;

	Display();
	m_segment = 1;

	return r;
}

const TCHAR* CDrawNoteText::GetXMLTag()
{
	return _T("NOTE_TEXT");
}

// Load and save to an XML file
void CDrawNoteText::SaveXML(CXMLWriter &xml)
{
	xml.addTag(GetXMLTag());

	xml.addAttribute(_T("a"), CDPoint(m_point_a));
	xml.addAttribute(_T("b"), CDPoint(m_point_b));
	xml.addAttribute(_T("style"), Style);
	xml.addAttribute(_T("fill"), Fill);
	xml.addAttribute(_T("note_text"), m_note_text);

	xml.closeTag();
}

void CDrawNoteText::LoadXML(CXMLReader &xml)
{
	xml.getAttribute(_T("a"), m_point_a);
	xml.getAttribute(_T("b"), m_point_b);
	xml.getAttribute(_T("style"), Style);
	xml.getAttribute(_T("fill"), Fill);
	xml.getAttribute(_T("note_text"), m_note_text);
	Style = m_pDesign->GetOptions()->GetNewStyleNumber(Style);

	// Calculate the Style nr here
	// This allows IsModified to correctly detect changes
	LineStyle lStyle = *m_pDesign->GetOptions()->GetStyle(Style);
	WORD line = m_pDesign->GetOptions()->AddStyle(&lStyle);
	m_pDesign->GetOptions()->SetCurrentStyle(GetType(), line);
	Style = line;

	Fill = m_pDesign->GetOptions()->GetNewFillStyleNumber(Fill);
}

// Load the rectange from a file
void CDrawNoteText::Load(CStream& archive)
{
	// Load the version number
	WORD version;
	archive >> version;

	m_point_a = ReadPoint(archive);
	m_point_b = ReadPoint(archive);

	archive >> Style;
	archive >> Fill;
	Style = m_pDesign->GetOptions()->GetNewStyleNumber(Style);
	Fill = m_pDesign->GetOptions()->GetNewFillStyleNumber(Fill);
}

// Load the rectangle from a file
//void CDrawNoteText::OldLoad2(CStream& archive)
//{
//	m_point_a = ReadPoint(archive);
//	m_point_b = ReadPoint(archive);
//
//	archive >> Style;
//	Style = m_pDesign->GetOptions()->GetNewStyleNumber(Style);
//	Fill = fsNONE;
//}
//
//// The old version of this load
//void CDrawNoteText::OldLoad(CStream& archive, int tp)
//{
//	BYTE LineThickness = 1;
//	m_point_a = ReadPoint(archive);
//	m_point_b = ReadPoint(archive);
//
//	if (tp == xSquareEx)
//	{
//		archive >> LineThickness;
//	}
//
//	LineStyle l;
//	l.Colour = cBLACK;
//	l.Style = PS_SOLID;
//	l.Thickness = LineThickness;
//	Style = m_pDesign->GetOptions()->AddStyle(&l);
//	Fill = fsNONE;
//}

// Change the setting due to the current options
void CDrawNoteText::NewOptions()
{
	Display();
	Style = m_pDesign->GetOptions()->GetCurrentStyle(GetType());
	Fill = m_pDesign->GetOptions()->GetCurrentFillStyle(GetType());
	Font = m_pDesign->GetOptions()->GetCurrentFont(GetType());
	Display();
}

BOOL CDrawNoteText::PointInEllipse(CDPoint p)
{
	return TRUE;
}

double CDrawNoteText::EllipseDistanceFromPoint(CDPoint p, BOOL &IsInside)
{

	// Determine the distance of this point to the
	// edge of the ellipse...

	// Determine the centre point and the two diameters
	CDPoint c = CDPoint( (m_point_a.x + m_point_b.x) / 2, (m_point_a.y + m_point_b.y) / 2);
	double da = abs(m_point_a.x - m_point_b.x) / 2;
	double db = abs(m_point_a.y - m_point_b.y) / 2;

	// Transform ellipse to circle with radius 1 and centre at (0,0)
	c -= p;
	c.x /= da;
	c.y /= db;

	// Distance from origin (0,0)
	double d = sqrt(c.x * c.x + c.y * c.y);

	IsInside = (d <= 1.0);

	// return real distance from edge of ellipse
	return abs(d - 1.0) * sqrt(da * da + db * db);
}

double CDrawNoteText::DistanceFromPoint(CDPoint p)
{

	// Always treat NoteText as if it is filled whether it is or not
	if (IsInside(p.x, p.x, p.y, p.y))
	{
		return 0.0;
	}

	// Ok, so check for distance from one of our lines...
	double distance = 100.0;

	CLineUtils l1(CDPoint(m_point_a.x, m_point_a.y), CDPoint(m_point_b.x, m_point_a.y));
	CLineUtils l2(CDPoint(m_point_b.x, m_point_a.y), CDPoint(m_point_b.x, m_point_b.y));
	CLineUtils l3(CDPoint(m_point_b.x, m_point_b.y), CDPoint(m_point_a.x, m_point_b.y));
	CLineUtils l4(CDPoint(m_point_a.x, m_point_b.y), CDPoint(m_point_a.x, m_point_a.y));

	CDPoint d;
	distance = min( distance, l1.DistanceFromPoint( p, d ) );
	distance = min( distance, l2.DistanceFromPoint( p, d ) );
	distance = min( distance, l3.DistanceFromPoint( p, d ) );
	distance = min( distance, l4.DistanceFromPoint( p, d ) );

	LineStyle *theStyle = m_pDesign->GetOptions()->GetStyle(Style);
	double width = min(0, theStyle->Thickness);// + (10 / (m_pDesign->GetTransform().GetZoomFactor()));
	return distance - width;
}

BOOL CDrawNoteText::IsInside(double left, double right, double top, double bottom)
{
	// Use fast cut-off to see if the bounding box is inside the intersection box
	if ( (m_point_a.x<left && m_point_b.x<=left) || (m_point_a.x>right && m_point_b.x>=right)
      || (m_point_a.y<top && m_point_b.y<=top) || (m_point_a.y>bottom && m_point_b.y>=bottom) )
	{
		return FALSE;
	}

	return TRUE;
}

void CDrawNoteText::Paint(CContext &dc, paint_options options)
{

	CDPoint sma = m_point_a;
	CDPoint smb = m_point_b;

	//Establish context for this dc
	if (Fill != fsNONE && options != draw_selectable)
	{
		dc.SelectBrush(m_pDesign->GetOptions()->GetFillStyle(Fill));
	}
	else
	{
		dc.SelectBrush();
	}
	dc.SelectPen(m_pDesign->GetOptions()->GetStyle(Style), options);
	dc.SetROP2(R2_COPYPEN);

	LOGFONT& lf = *(m_pDesign->GetOptions()->GetFont(Font));
	dc.SelectFont(lf, options);

	CDRect r;

	r.left = sma.x;
	r.right = smb.x;
	r.top = sma.y;
	r.bottom = smb.y;
	r.NormalizeRect();	//This is the rectangle that the text will actually be drawn in

	//Form a larger decorative double rectangle around the text and draw it.
	//This also provides a clear margin for the text.
	//The size of the decorative border must be enough larger than the radius selected for the rounded rectangle that the
	//text doesn't display on top of the radiused corners.
	//Also, the Z aspect must be respected here, or filled backgrounds will overwrite objects so draw objects from largest to smallest in order.
	CDSize innerBorderRectangleDelta;
	innerBorderRectangleDelta.cx = r.Width() * 0.02;	//2% wider than the text box
	innerBorderRectangleDelta.cy = r.Height() * 0.02;	//2% taller than the text box

	innerBorderRectangleDelta.ForceLargerSize();	//Enlarge the delta size to the larger of the width or height
	innerBorderRectangleDelta.ForceMinSize(5);	//but not smaller than this amount

	CDSize outerBorderRectangleDelta=innerBorderRectangleDelta;
	outerBorderRectangleDelta += CDSize(5,5);
	CDRect border;
	
	//Draw the outer most nested rectangle as a flourish
	border=r;
	border.InflateRect(outerBorderRectangleDelta);
//		dc.Rectangle(border);

	//Set the radius of the rounded rectangle to 10% of the width and height of the rectangle
	CDPoint radius;
	radius.x = border.Width() * 0.1;
	radius.y = border.Height() * 0.1;
	radius.ForceLargerSize();
	dc.RoundRect(border, radius);

	//Draw the inner most nested rectangle as a flourish
	border = r;
	border.InflateRect(innerBorderRectangleDelta);
//		dc.Rectangle(border);
	radius.x = border.Width() * 0.1;
	radius.y = border.Height() * 0.1;
	radius.ForceLargerSize();
	//TRACE("left=%f, right=%f, Width=%f, radius.x=%f, bottom=%f, top=%f, Height=%f, radius.y=%f\n", border.left, border.right, border.Width(), radius.x, border.bottom, border.top, border.Height(), radius.y);
	dc.RoundRect(border, radius);

	//Now draw the text itself - this is just sample code waiting for full integration with user specified text!
	dc.SetTextColor(cBLUE);	//force color for now
	int backgroundMode = dc.GetBkMode();	//Save the current background mode
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(m_note_text, r);	//Now draw the note text on top of the inner rectangle
	dc.SetBkMode(backgroundMode);	//Restore the previous background mode
}

// Store the NoteText in the drawing
CDrawingObject *CDrawNoteText::Store()
{
	CDrawNoteText *NewObject;

	NewObject = new CDrawNoteText(m_pDesign, m_type);
	*NewObject = *this;

	m_pDesign->Add(NewObject);

	m_segment = 1;

	return NewObject;
}
