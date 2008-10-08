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


void CDrawSquare::LButtonUp(CDPoint p)
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

const TCHAR* CDrawSquare::GetXMLTag( BOOL isSquare )
{
	if (isSquare)
	{
		return _T("RECTANGLE");
	}
	else
	{
		return _T("ELLIPSE");
	}
}

// Load and save to an XML file
void CDrawSquare::SaveXML( CXMLWriter &xml )
{
	xml.addTag(GetXMLTag(IsSquare()));

	xml.addAttribute( _T("a"), CDPoint(m_point_a) );
	xml.addAttribute( _T("b"), CDPoint(m_point_b) );
	xml.addAttribute( _T("style"), Style );
	xml.addAttribute( _T("fill"), Fill );

	xml.closeTag();
}

void CDrawSquare::LoadXML( CXMLReader &xml )
{
	xml.getAttribute( _T("a"), m_point_a );
	xml.getAttribute( _T("b"), m_point_b );
	xml.getAttribute( _T("style"), Style );
	xml.getAttribute( _T("fill"), Fill );

    Style = m_pDesign->GetOptions()->GetNewStyleNumber(Style);
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

	// Determine the distance of this point from the
	// edge of the ellipse...

	// Determine the centre point and the two diameters
	CDPoint c = CDPoint((m_point_a.x+m_point_b.x)/2, (m_point_a.y+m_point_b.y)/2);
	double da = abs(m_point_a.x - m_point_b.x)/2;
	double db = abs(m_point_a.y - m_point_b.y)/2;

	// Now determine the angle of our point to
	// the centre
	double u = atan2( static_cast<double>(p.x - c.x), static_cast<double>(p.y - c.y) );

	// ... and determine the location of the point
	// on the ellipse at this location
	CDPoint pe = CDPoint(c.x + da * sin(u), c.y + db * cos(u) );

	// confirm the distance from this point...
	double d1 = sqrt(static_cast<double>((pe.x - p.x)*(pe.x - p.x)
		+ (pe.y - p.y)*(pe.y - p.y)));

	double d2 = sqrt(static_cast<double>((c.x - p.x)*(c.x - p.x)
		+ (c.y - p.y)*(c.y - p.y)));

	double d3 = sqrt(static_cast<double>((c.x - pe.x)*(c.x - pe.x)
		+ (c.y - pe.y)*(c.y - pe.y)));

	IsInside = d2 < d3;

	return d1 * 2;
}

double CDrawSquare::DistanceFromPoint( CDPoint p )
{
	// Are we filled?
	if (IsSquare())
	{
		if (Fill == fsNONE)
		{
			// Ok, so check for distance from one of our lines...
			double closest_distance = 100.0;

			CLineUtils l1(CDPoint(m_point_a.x, m_point_a.y), CDPoint(m_point_b.x, m_point_a.y));
			CLineUtils l2(CDPoint(m_point_b.x, m_point_a.y), CDPoint(m_point_b.x, m_point_b.y));
			CLineUtils l3(CDPoint(m_point_b.x, m_point_b.y), CDPoint(m_point_a.x, m_point_b.y));
			CLineUtils l4(CDPoint(m_point_a.x, m_point_b.y), CDPoint(m_point_a.x, m_point_a.y));

			CDPoint d;
			closest_distance = min( closest_distance, l1.DistanceFromPoint( p, d ) );
			closest_distance = min( closest_distance, l2.DistanceFromPoint( p, d ) );
			closest_distance = min( closest_distance, l3.DistanceFromPoint( p, d ) );
			closest_distance = min( closest_distance, l4.DistanceFromPoint( p, d ) );

			return closest_distance;
		}
		else
		{
			return IsInside(p.x,p.x,p.y,p.y) ? 0 : 100.0;
		}
	}
	else if (Fill == fsNONE && !IsSquare())
	{
		BOOL r;
		return EllipseDistanceFromPoint( p, r );
	}
	else
	{
		// There is a fill, so just use the normal isinside routine...
		BOOL r;
		EllipseDistanceFromPoint( p, r );
		return r ? 0 : 100.0;
	}

	return 100.0;
}


BOOL CDrawSquare::IsInside(double left,double right,double top,double bottom)
{
	LineStyle *theStyle = m_pDesign->GetOptions()->GetStyle(Style);
	int width = max(2,theStyle->Thickness);

	if (IsSquare())
	{
		if (Fill != fsNONE)
		{
			// Filled rectangle, use the normal is inside..
			return CDrawRectOutline::IsInside(left,right,top,bottom);
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
		// Determine the ellipse is entirely inside the
		// rectangle...
		CDRect r(m_point_a.x,m_point_a.y,m_point_b.x,m_point_b.y);
		r.NormalizeRect();
		if (r.left >= left && r.right <= right 
		    && r.top >= top && r.bottom <= bottom)
		{
			return TRUE;
		}

		// Or is the rectangle entirely outside the ellipse?
		if (!CDrawRectOutline::IsInside(left,right,top,bottom))
		{
			return FALSE;
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
  else
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



