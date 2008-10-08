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
#include "colour.h"
#include "option.h"
#include "LineUtils.h"
#include <math.h>


////// The Junction Class ////



CDrawJunction::CDrawJunction(CTinyCadDoc *pDesign)
: CDrawingObject( pDesign )
{
	m_point_a=m_point_b=CDPoint(0,0);
	m_segment=1;
	is_stuck = FALSE;
}


void CDrawJunction::Load(CStream &archive)
{
  m_point_a = ReadPoint(archive);
  m_point_b=m_point_a;
  m_segment=0;
}

const TCHAR* CDrawJunction::GetXMLTag()
{
	return _T("JUNCTION");
}

// Load and save to an XML file
void CDrawJunction::SaveXML( CXMLWriter &xml )
{
	xml.addTag(GetXMLTag());
	xml.addAttribute( _T("pos"), CDPoint(m_point_a) );
	xml.closeTag();
}

void CDrawJunction::LoadXML( CXMLReader &xml )
{
	xml.getAttribute( _T("pos"), m_point_a );
	m_point_b = m_point_a;
}



BOOL CDrawJunction::IsInside(double left,double right,double top,double bottom)
{
  return (m_point_a.x>=left && m_point_a.x<=right && m_point_a.y>=top && m_point_a.y<=bottom);
}


ObjType CDrawJunction::GetType()
{
  return xJunction;
}


CString CDrawJunction::GetName() const
{
  return "Junction";
}

void CDrawJunction::Move(CDPoint p, CDPoint no_snap_p)
{
  Display();

  BOOL is_junction;
  p = m_pDesign->GetStickyPoint(p,FALSE,TRUE,is_stuck,is_junction );

  m_point_a=p;
  m_point_b=m_point_a;
  m_segment=0;
  Display();
}

void CDrawJunction::LButtonDown(CDPoint p, CDPoint)
{
  // New undo level for each placement...
  m_pDesign->BeginNewChangeSet();

  Display();
  BOOL is_junction;
  p = m_pDesign->GetStickyPoint(p,FALSE,TRUE,is_stuck,is_junction );
  m_point_a=p;
  m_point_b=p;
  Store();
  Display();	// Write to screen
}

void CDrawJunction::Display( BOOL erase )
{
	// Invalidate the symbol
	CDRect r( m_point_a.x,m_point_a.y,m_point_b.x,m_point_b.y);
	m_pDesign->InvalidateRect( r, erase, JUNCTION_SIZE + 1 );

   if (is_stuck)
   {
	 r.left = m_point_a.x - HIGHLIGHT_SIZE;
	 r.right = m_point_a.x + HIGHLIGHT_SIZE;
	 r.top = m_point_a.y - HIGHLIGHT_SIZE;
	 r.bottom = m_point_a.y + HIGHLIGHT_SIZE;
	 m_pDesign->InvalidateRect( r, erase, 3 );
   } 

}


void CDrawJunction::Paint(CContext &dc,paint_options options)
{
  int js=JUNCTION_SIZE;
  CDPoint br,tl;
  br=CDPoint(m_point_a.x+js+1,m_point_a.y+js+1);
  tl=CDPoint(m_point_a.x-js,m_point_a.y-js);

	dc.SetROP2(R2_COPYPEN);


  switch (options)
  {
  case draw_selected:
	  dc.SelectPen(PS_SOLID,1,cSELECT);
      dc.SelectBrush(cSELECT);
	  break;
  case draw_selectable:
	  dc.SelectPen(PS_SOLID,1,cJUNCTION_CLK);
      dc.SelectBrush(cJUNCTION_CLK);
	  break;
  default:
		dc.SelectPen(PS_SOLID,1,m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::JUNCTION) );
		dc.SelectBrush(m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::JUNCTION));
  }

  dc.Ellipse(CDRect(tl.x,tl.y,br.x,br.y));

  if (is_stuck)
  {
	  // Draw a nice circle to show the stickness...
	  dc.PaintConnectPoint( m_point_b );
  }
}


// Store the junction in the drawing
CDrawingObject* CDrawJunction::Store()
{
  CDrawJunction *NewObject;

  is_stuck = FALSE;
  NewObject = new CDrawJunction(m_pDesign);
  *NewObject = *this;

  m_pDesign->Add(NewObject);

  return NewObject;
}

double CDrawJunction::DistanceFromPoint( CDPoint p )
{
	double dx = (m_point_a.x - p.x);
	double dy = (m_point_a.y - p.y);
	double d = sqrt(dx*dx + dy*dy);

	if (d < JUNCTION_SIZE)
	{
		d = 0.0;
	}

	return d;
}

