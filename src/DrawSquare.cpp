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

// The Rectangle Class //

// The Constructor
CDrawSquare::CDrawSquare( CTinyCadDoc *pDesign, ObjType type )
: CDrawRectOutline( pDesign )
{
	m_type = type;
	m_segment=1;
	Style = m_pDesign->GetOptions()->GetCurrentStyle(GetType());
	Fill = m_pDesign->GetOptions()->GetCurrentFillStyle(GetType());
}



ObjType CDrawSquare::GetType()
{
  return m_type;
}

CString CDrawSquare::GetName() const
{
  return "Rectangle";
}

void CDrawSquare::BeginEdit(BOOL re_edit)
{
  m_segment=1;

  m_pDesign->GetOptions()->SetCurrentStyle(GetType(), Style);
  m_pDesign->GetOptions()->SetCurrentFillStyle(GetType(), Fill);

  m_re_edit = re_edit;
  if (re_edit)
  {
	  g_EditToolBar.m_PolygonEdit.Open(m_pDesign,this);
  }
}

void CDrawSquare::EndEdit()
{
  Display();
  if (m_re_edit)
  {
	g_EditToolBar.m_PolygonEdit.Close();
  }

}

// Place the object into the design
void CDrawSquare::LButtonDown(CDPoint p, CDPoint s)
{
  // New undo level for each placement...
  m_pDesign->BeginNewChangeSet();

  if (m_segment) {
	m_point_a=p;
	m_point_b=p;
	m_segment=!m_segment;
	Display();
  } else {
	m_point_b=p;
	Display();
	CDrawingObject *pObject = Store();

	RButtonDown(p, s);

	// Now select the top object and switch back
	// to the Edit tool
	m_pDesign->UnSelect();
	m_pDesign->Select( pObject );
	m_pDesign->SelectObject( new CDrawEditItem(m_pDesign) );
  }
}


void CDrawSquare::LButtonUp(CDPoint p, CDPoint)
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
		LButtonDown(p,p);
	}
}



void CDrawSquare::TagResources()
{
   m_pDesign->GetOptions()->TagStyle(Style);
   m_pDesign->GetOptions()->TagFillStyle(Fill);
}


BOOL CDrawSquare::RButtonDown(CDPoint p, CDPoint s)
{
  BOOL r = !m_segment;

  Display();
  m_segment=1;
  
  return r;
}

const TCHAR* CDrawSquare::GetXMLTag( BOOL isSquare, BOOL hasNoteText )
{
	if (isSquare)
	{
		return _T("RECTANGLE");
	}
	else if (hasNoteText)
	{
		return _T("NOTE_TEXT");
	}
	else
	{
		return _T("ELLIPSE");
	}
}

// Load and save to an XML file
void CDrawSquare::SaveXML( CXMLWriter &xml )
{
	xml.addTag(GetXMLTag(IsSquare(), HasNoteText()));

	xml.addAttribute( _T("a"), CDPoint(m_point_a) );
	xml.addAttribute( _T("b"), CDPoint(m_point_b) );
	xml.addAttribute( _T("style"), Style );
	xml.addAttribute( _T("fill"), Fill );
	if (HasNoteText()) {
		xml.addAttribute(_T("note_text"), NoteText);
	}

	xml.closeTag();
}

void CDrawSquare::LoadXML( CXMLReader &xml )
{
	xml.getAttribute( _T("a"), m_point_a );
	xml.getAttribute( _T("b"), m_point_b );
	xml.getAttribute( _T("style"), Style );
	xml.getAttribute( _T("fill"), Fill );

    Style = m_pDesign->GetOptions()->GetNewStyleNumber(Style);

	// Calculate the Style nr here
	// This allows IsModified to correctly detect changes
	LineStyle lStyle = *m_pDesign->GetOptions()->GetStyle( Style );
	WORD line = m_pDesign->GetOptions()->AddStyle(&lStyle);
	m_pDesign->GetOptions()->SetCurrentStyle( GetType(), line );
	Style = line;

    Fill = m_pDesign->GetOptions()->GetNewFillStyleNumber(Fill);
}


// Load the rectange from a file
void CDrawSquare::Load(CStream& archive )
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

// Load the rectange from a file
void CDrawSquare::OldLoad2(CStream& archive )
{
 	m_point_a = ReadPoint(archive);
   	m_point_b = ReadPoint(archive);

  archive >> Style;
  Style = m_pDesign->GetOptions()->GetNewStyleNumber(Style);
  Fill = fsNONE;
}


// The old version of this load
void CDrawSquare::OldLoad(CStream& archive, int tp )
{
  BYTE LineThickness = 1;
  m_point_a = ReadPoint(archive);
  m_point_b = ReadPoint(archive);

  if (tp == xSquareEx) {
  	archive >> LineThickness;
  }

  LineStyle l;
  l.Colour = cBLACK;
  l.Style = PS_SOLID;
  l.Thickness = LineThickness;
  Style = m_pDesign->GetOptions()->AddStyle(&l);
  Fill = fsNONE;
}

// Change the setting due to the current options
void CDrawSquare::NewOptions()
{
 	Display();
	Style = m_pDesign->GetOptions()->GetCurrentStyle(GetType());
	Fill = m_pDesign->GetOptions()->GetCurrentFillStyle(GetType());
	Display();
}

BOOL CDrawSquare::PointInEllipse( CDPoint p )
{
	return TRUE;
}

double CDrawSquare::EllipseDistanceFromPoint( CDPoint p, BOOL &IsInside )
{

	// Determine the distance of this point to the
	// edge of the ellipse...

	// Determine the centre point and the two diameters
	CDPoint c = CDPoint((m_point_a.x+m_point_b.x)/2, (m_point_a.y+m_point_b.y)/2);
	double da = abs(m_point_a.x - m_point_b.x)/2;
	double db = abs(m_point_a.y - m_point_b.y)/2;

	// Transform ellipse to circle with radius 1 and centre at (0,0)
	c -= p;
	c.x /= da;
	c.y /= db;

	// Distance from origin (0,0)
	double d = sqrt(c.x*c.x + c.y*c.y);

	IsInside = (d <= 1.0);

	// return real distance from edge of ellipse
	return abs(d - 1.0) * sqrt(da*da + db*db);	
}

double CDrawSquare::DistanceFromPoint( CDPoint p )
{

	if (IsSquare() || HasNoteText())
	{
		// Are we filled?
		if (Fill != fsNONE || HasNoteText())	//note text is treated as if the rectangle is filled
		{
			if (IsInside(p.x,p.x,p.y,p.y))
			{
				return 0.0;
			}
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
	else
	{
		BOOL r;
		double distance = EllipseDistanceFromPoint( p, r );
		// Inside filled ellipse?
		if (r && Fill != fsNONE)
		{
			return 0.0;
		}

		LineStyle *theStyle = m_pDesign->GetOptions()->GetStyle(Style);
		double width = min(0, theStyle->Thickness);// + (10 / (m_pDesign->GetTransform().GetZoomFactor()));
		return distance - width;
	}
}


BOOL CDrawSquare::IsInside(double left,double right,double top,double bottom)
{
	// Use fast cut-off to see if the bounding box is inside the intersection box
	if ( (m_point_a.x<left && m_point_b.x<=left) || (m_point_a.x>right && m_point_b.x>=right)
      || (m_point_a.y<top && m_point_b.y<=top) || (m_point_a.y>bottom && m_point_b.y>=bottom) )
	{
		return FALSE;
	}

	if (IsSquare() || HasNoteText())
	{
		if (Fill != fsNONE || (left==right && top==bottom) || HasNoteText())
		{
			// Filled rectangle
			return TRUE;
		}
		else
		{
			CLineUtils l1(CDPoint(m_point_a.x, m_point_a.y), CDPoint(m_point_b.x, m_point_a.y));
			CLineUtils l2(CDPoint(m_point_b.x, m_point_a.y), CDPoint(m_point_b.x, m_point_b.y));
			CLineUtils l3(CDPoint(m_point_b.x, m_point_b.y), CDPoint(m_point_a.x, m_point_b.y));
			CLineUtils l4(CDPoint(m_point_a.x, m_point_b.y), CDPoint(m_point_a.x, m_point_a.y));
			return l1.IsInside(left,right,top,bottom)
				|| l2.IsInside(left,right,top,bottom)
				|| l3.IsInside(left,right,top,bottom)
				|| l4.IsInside(left,right,top,bottom);
		}
	}
	else
	{
		// Determine if the ellipse is entirely inside the
		// rectangle...
		CDRect r(m_point_a.x,m_point_a.y,m_point_b.x,m_point_b.y);
		r.NormalizeRect();
		if (r.left >= left && r.right <= right && 
		    r.top >= top && r.bottom <= bottom)
		{
			return TRUE;
		}

		// Ok does one of the lines cut the ellipse?
		CDPoint l1p, l2p, l3p, l4p;

		CLineUtils l1(CDPoint(left, top), CDPoint(right, top));
		CLineUtils l2(CDPoint(right, top), CDPoint(right, bottom));
		CLineUtils l3(CDPoint(right, bottom), CDPoint(left, bottom));
		CLineUtils l4(CDPoint(left, bottom), CDPoint(left, top));
		CDPoint c = CDPoint((m_point_a.x+m_point_b.x)/2, (m_point_a.y+m_point_b.y)/2);

		l1.DistanceFromPoint( c, l1p);
		l2.DistanceFromPoint( c, l2p);
		l3.DistanceFromPoint( c, l3p);
		l4.DistanceFromPoint( c, l4p);

		// Are any of these points inside the ellipse?
		BOOL r1,r2,r3,r4;
		EllipseDistanceFromPoint( l1p, r1 );
		EllipseDistanceFromPoint( l2p, r2 );
		EllipseDistanceFromPoint( l3p, r3 );
		EllipseDistanceFromPoint( l4p, r4 );

		return r1 || r2 || r3 || r4;
	}
}

void CDrawSquare::Paint(CContext &dc,paint_options options)
{

  CDPoint sma = m_point_a;
  CDPoint smb = m_point_b;

  if (Fill != fsNONE && options != draw_selectable)
  {
	dc.SelectBrush(m_pDesign->GetOptions()->GetFillStyle(Fill));
  }
  else
  {
	  dc.SelectBrush();
  }
  dc.SelectPen(m_pDesign->GetOptions()->GetStyle(Style),options);
  dc.SetROP2(R2_COPYPEN);

  if (IsSquare())
  {
		dc.Rectangle(CDRect(sma.x,sma.y,smb.x,smb.y));
  }
  else if (HasNoteText())
  {
		dc.Rectangle(CDRect(sma.x,sma.y,smb.x,smb.y));
		//Now draw the note text on top of the rectangle
  }
  else	//Must be an ellipse
  {
	  dc.Ellipse(CDRect(sma.x,sma.y,smb.x,smb.y));
  }
}


// Store the rectangle in the drawing
CDrawingObject *CDrawSquare::Store()
{
  CDrawSquare *NewObject;

  NewObject = new CDrawSquare(m_pDesign,m_type);
  *NewObject = *this;

  m_pDesign->Add(NewObject);
  
  m_segment=1;

  return NewObject;
}



