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

void CDrawNoteText::TagResources()
{
	m_pDesign->GetOptions()->TagFont(FontStyle);
	m_pDesign->GetOptions()->TagStyle(Style);
	m_pDesign->GetOptions()->TagFillStyle(Fill);
}

BOOL CDrawNoteText::CanEdit()
{
	return TRUE;
}

// Look for a search string in the object
CString CDrawNoteText::Find(const TCHAR *theSearchString)
{
	CString HoldString;

	HoldString = str;
	HoldString.MakeLower();

	// Now look for the search string in this string
	return (HoldString.Find(theSearchString) != -1) ? str : "";
}

void CDrawNoteText::NewOptions()
{
	CalcLayout();
	Display();
	Style = m_pDesign->GetOptions()->GetCurrentStyle(GetType());
	Fill = m_pDesign->GetOptions()->GetCurrentFillStyle(GetType());
	FontStyle = m_pDesign->GetOptions()->GetCurrentFont(GetType());
	original_width = 0;
	CalcLayout();
	Display();
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
	xml.addAttribute(_T("direction"), dir);
	xml.addAttribute(_T("style"), Style);	//Line style
	xml.addAttribute(_T("fill"), Fill);
	xml.addAttribute(_T("font"), FontStyle);
	xml.addAttribute(_T("color"), FontColour);
	xml.addAttribute(_T("border_style"), m_border_style);
	xml.addChildData(str);

	xml.closeTag();
}

void CDrawNoteText::LoadXML(CXMLReader &xml)
{
	xml.getAttribute(_T("a"), m_point_a);
	xml.getAttribute(_T("b"), m_point_b);
	xml.getAttribute(_T("direction"), dir);
	xml.getAttribute(_T("style"), Style);
	xml.getAttribute(_T("fill"), Fill);
	xml.getAttribute(_T("font"), FontStyle);
	xml.getAttribute(_T("color"), FontColour);

	int temp;	//xml reader doesn't like enums, so read border style as an int
	xml.getAttribute(_T("border_style"), temp);
	m_border_style = static_cast<BorderStyle>(temp);

	xml.getChildData(str);
	Style = m_pDesign->GetOptions()->GetNewStyleNumber(Style);

	// Calculate the Style nr here
	// This allows IsModified to correctly detect changes
	LineStyle lStyle = *m_pDesign->GetOptions()->GetStyle(Style);
	WORD line = m_pDesign->GetOptions()->AddStyle(&lStyle);
	m_pDesign->GetOptions()->SetCurrentStyle(GetType(), line);
	Style = line;

	Fill = m_pDesign->GetOptions()->GetNewFillStyleNumber(Fill);
	FontStyle = m_pDesign->GetOptions()->GetNewFontNumber(FontStyle);

	m_segment = 0;
	CalcLayout();
}

// Load the note text object from a file
void CDrawNoteText::Load(CStream& archive)
{
	// Load the version number
	WORD version;
	archive >> version;

	m_point_a = ReadPoint(archive);
	m_point_b = ReadPoint(archive);

	archive >> dir >> Style >> Fill >> FontStyle >> FontColour >> m_border_style;

	// Convert the font number, font colour, and fill styles over from externally stored numbers to actual internal in-use numbers
	FontStyle = m_pDesign->GetOptions()->GetNewFontNumber(FontStyle);
	Style = m_pDesign->GetOptions()->GetNewStyleNumber(Style);
	Fill = m_pDesign->GetOptions()->GetNewFillStyleNumber(Fill);

	//TRACE("CDrawNoteText::Load() - GetNewFillStyleNumber=0x%08X\n", Fill);
	CDSize size = m_pDesign->GetTextExtent(str, FontStyle);

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

ObjType CDrawNoteText::GetType()
{
	return xNoteText;
}

CString CDrawNoteText::GetName() const
{
	return "NoteText";
}

void CDrawNoteText::SetScalingWidths()
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

void CDrawNoteText::BeginEdit(BOOL re_edit)
{
	m_segment = 1;

	m_pDesign->GetOptions()->SetCurrentStyle(GetType(), Style);
	m_pDesign->GetOptions()->SetCurrentFillStyle(GetType(), Fill);
	m_pDesign->GetOptions()->SetCurrentFont(GetType(), FontStyle);
	//TRACE("CDrawNoteText::BeginEdit() - For type=%d, set current fill style to 0x%08LX\n", GetType(), Fill);
	SetScalingWidths();

	m_re_edit = re_edit;
	if (re_edit)
	{	//This code is executed if a NoteText object already exists and it is selected, thus allowing "re-editing" of the object
		g_EditToolBar.m_NoteTextEdit.Open(m_pDesign, this);
	}
}

void CDrawNoteText::EndEdit()
{
	Display();
	if (m_re_edit)
	{	//This code is executed if a NoteText object already exists and it is selected, thus allowing "re-editing" of the object
		g_EditToolBar.m_NoteTextEdit.Close();
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

BOOL CDrawNoteText::RButtonDown(CDPoint p, CDPoint s)
{
	BOOL r = !m_segment;

	Display();
	m_segment = 1;

	return r;
}

// The Constructor
CDrawNoteText::CDrawNoteText(CTinyCadDoc *pDesign, ObjType type) :
	CDrawRectOutline(pDesign)
{
	//m_type = type = xNoteText;
	m_type = type;
	dir = 3;
	str = _T("Note");		//Default text for the note so that a new note doesn't get visually lost.
	m_segment = 0;
	m_point_a = m_point_b = CDPoint(0, 0);
	original_width = 0;
	original_box_width = 0;
	target_box_width = 0;

	FontStyle = m_pDesign->GetOptions()->GetCurrentFont(GetType());
	FontColour = m_pDesign->GetOptions()->GetUserColor().Get(CUserColor::NOTETEXT_TEXT);

	m_tab_width_in_avg_char_widths = 4;		//This sets the default tab width in average character width units.
	m_border_style = BS_RoundedRectangle;	//This sets the default border style

	//There isn't a "UserFill", only "UserColor"'s so we must make a fill style that lets us specify the user's preferred default color choice for the background fill.
	FillStyle tempFill;
	tempFill.Index = 0;	//solid fill with a default color of black (taken from criminally cryptic comment in FillStyle)
	tempFill.Colour = m_pDesign->GetOptions()->GetUserColor().Get(CUserColor::NOTETEXT_FILL);	//get the user's preferred notetext background fill color
	Fill = m_pDesign->GetOptions()->AddFillStyle(&tempFill);	//Add the newly created style, if it is unique and not already added
	m_pDesign->GetOptions()->SetCurrentFillStyle(GetType(), Fill);	//Set the current fill style to this new style

	//Create a line style used to draw the note's enclosing rectangle
	LineStyle tempLineStyle;
	tempLineStyle.Style = PS_SOLID;	//default line style
	tempLineStyle.Colour = m_pDesign->GetOptions()->GetUserColor().Get(CUserColor::NOTETEXT_LINE);
	tempLineStyle.Thickness = 1;
	//Write back the newly created line style
	Style = m_pDesign->GetOptions()->AddStyle(&tempLineStyle);
	m_pDesign->GetOptions()->SetCurrentStyle(GetType(), Style);
//	Style = m_pDesign->GetOptions()->GetCurrentStyle(GetType());	//Line style - use the last style set
}

// Change the setting due to the current options
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

void CDrawNoteText::CalcLayout()
{
	CDSize size = m_pDesign->GetTextExtent(str, FontStyle);

	if (original_width == 0)
	{
		SetScalingWidths();
	}
}

void CDrawNoteText::Paint(CContext &dc, paint_options options)
{
	CalcLayout();

	//Establish context for this dc
	if (Fill != fsNONE && options != draw_selectable)
	{	//Select a brush that is appropriate for filled rectangles.  If the mouse is hovering over the rectangle, then it is "selectable" and the rectangle will be drawn as unfilled.
		dc.SelectBrush(m_pDesign->GetOptions()->GetFillStyle(Fill));
	}
	else
	{	//Select a brush that is appropriate for non-filled rectangles or for rectangles that are "selectable"
		dc.SelectBrush();
	}

	dc.SelectPen(m_pDesign->GetOptions()->GetStyle(Style), options);	//SelectPen takes care of the Paint options regarding selected, selectable, or normal

	CDRect outerBorderRectangle(m_point_a.x, m_point_a.y, m_point_b.x, m_point_b.y);
	outerBorderRectangle.NormalizeRect();

	//Form a decorative single line rectangle around the text and draw it.
	//The Z aspect must be respected here, or filled backgrounds will overwrite objects so draw objects from largest to smallest in order.
	CDPoint radius(10,10);	//Used to draw rounded rectangles with radius in units of pixels
	CDSize rectangleReductionDelta(-3,-3);	//text drawing area is 3 pixels smaller than outer border in y direction.  x direction is controlled by left and right margins.

	CDRect textRectangle = outerBorderRectangle;
	textRectangle.InflateRect(rectangleReductionDelta);
	if (!textRectangle.IsNormalized()) 
	{	//Can't make text border rectangle so much smaller that it no longer forms a normalized rectangle
		textRectangle = outerBorderRectangle;
	}

	if (m_border_style == BS_Rectangle) {
		dc.Rectangle(outerBorderRectangle);
	}
	else if (m_border_style == BS_RoundedRectangle) {
		//Set the radius of the rounded rectangle to 10% of the width and height of the rectangle
		dc.RoundRect(outerBorderRectangle, radius);
	}
	else
	{	//djl - had trouble getting this to work, but original code is here for future efforts.  The problem seems to be that NULL_PEN cannot be directly used as a style.  Must create a line style that uses the NULL_PEN pen instead.
		//Draw no border at all, but keep the fill property by drawing rectangle with an invisible pen
		//dc.SelectPen(m_pDesign->GetOptions()->GetStyle(NULL_PEN), options);
		//dc.Rectangle(outerBorderRectangle);
		//dc.SelectPen(m_pDesign->GetOptions()->GetStyle(Style), options);
	}

	//Now draw the text itself
	dc.SetROP2(R2_COPYPEN);	//Select the desired raster operation
	dc.SelectFont(*m_pDesign->GetOptions()->GetFont(FontStyle), dir);

	dc.SetTextColor(FontColour);
	dc.SetBkMode(TRANSPARENT);
	DRAWTEXTPARAMS formatOptions;
	formatOptions.cbSize = sizeof(DRAWTEXTPARAMS);
	formatOptions.iLeftMargin = 0;
	formatOptions.iRightMargin = 0;
	formatOptions.iTabLength = m_tab_width_in_avg_char_widths;
	formatOptions.uiLengthDrawn = 0;

	dc.DrawTextExW(str, textRectangle, &formatOptions);	//Now draw the note text on top of the border rectangle
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


BOOL CDrawNoteText::IsEmpty()
{
	if (str.IsEmpty())
	{
		return TRUE;
	}

	return FALSE;
}
