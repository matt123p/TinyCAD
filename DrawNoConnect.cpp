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




////// The No Connect Class //////


CDrawNoConnect::CDrawNoConnect(CTinyCadDoc *pDesign)
: CDrawingObject( pDesign )
{
	m_point_a=m_point_b=CDPoint(0,0);
	m_segment=1;
	is_stuck = FALSE;
}

const TCHAR* CDrawNoConnect::GetXMLTag()
{
	return _T("NOCONNECT");
}

// Load and save to an XML file
void CDrawNoConnect::SaveXML( CXMLWriter &xml )
{
	xml.addTag(GetXMLTag());
	xml.addAttribute( _T("pos"), CDPoint(m_point_a) );
	xml.closeTag();
}

void CDrawNoConnect::LoadXML( CXMLReader &xml )
{
	xml.getAttribute( _T("pos"), m_point_a );
	m_point_b = m_point_a;
}



void CDrawNoConnect::Load(CStream &archive)
{
 	m_point_a = ReadPoint(archive);
  m_point_b=m_point_a;
  m_segment=0;
}


double CDrawNoConnect::DistanceFromPoint( CDPoint p )
{
	return sqrt( static_cast<double>((m_point_a.x-p.x)*(m_point_a.x-p.x) + (m_point_a.y-p.y)*(m_point_a.y-p.y)) );
}


BOOL CDrawNoConnect::IsInside(double left,double right,double top,double bottom)
{
  return (m_point_a.x>=left && m_point_a.x<=right && m_point_a.y>=top && m_point_a.y<=bottom);
}


ObjType CDrawNoConnect::GetType()
{
  return xNoConnect;
}

CString CDrawNoConnect::GetName() const
{
  return "No Connect";
}


void CDrawNoConnect::Move(CDPoint p, CDPoint no_snap_p)
{
  Display();
  BOOL is_junction;
  p = m_pDesign->GetStickyPoint(p,TRUE,FALSE,is_stuck,is_junction);
  m_point_a=p;
  m_point_b=p;
  m_segment=0;
  Display();
}

void CDrawNoConnect::LButtonDown(CDPoint p, CDPoint)
{
  // New undo level for each placement...
  m_pDesign->BeginNewChangeSet();

  Display();
  BOOL is_junction;
  p = m_pDesign->GetStickyPoint(p,TRUE,FALSE,is_stuck,is_junction);
  m_point_a=p;
  m_point_b=p;
  Store();
  Display();	// Write to screen
}

void CDrawNoConnect::Display( BOOL erase )
{
	// Invalidate the symbol
	CDRect r( m_point_a.x,m_point_a.y,m_point_b.x,m_point_b.y);
	m_pDesign->InvalidateRect( r, erase, CONNECT_SIZE + 2 );

   if (is_stuck)
   {
	 r.left = m_point_a.x - HIGHLIGHT_SIZE;
	 r.right = m_point_a.x + HIGHLIGHT_SIZE;
	 r.top = m_point_a.y - HIGHLIGHT_SIZE;
	 r.bottom = m_point_a.y + HIGHLIGHT_SIZE;
	 m_pDesign->InvalidateRect( r, erase, 3 );
   } 

}



void CDrawNoConnect::Paint(CContext &dc,paint_options options)
{
  switch (options)
  {
  case draw_selected:
	  dc.SelectPen(PS_SOLID,1,cSELECT);
	  break;
  case draw_selectable:
	  dc.SelectPen(PS_SOLID,1,cNOCONNECT_CLK);
	  break;
  default:
	  dc.SelectPen(PS_SOLID,1,m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::NOCONNECT));
  }

  
  

	dc.SetROP2(R2_COPYPEN);

  CDPoint LineTo1=CDPoint(m_point_a.x+CONNECT_SIZE,m_point_a.y+CONNECT_SIZE);
  CDPoint LineTo2=CDPoint(m_point_a.x+CONNECT_SIZE,m_point_a.y-CONNECT_SIZE);

  dc.MoveTo(CDPoint(m_point_a.x-CONNECT_SIZE,m_point_a.y-CONNECT_SIZE));
  dc.LineTo(CDPoint(LineTo1.x+1,LineTo1.y));
  dc.MoveTo(CDPoint(m_point_a.x-CONNECT_SIZE,m_point_a.y+CONNECT_SIZE));
  dc.LineTo(CDPoint(LineTo2.x+1,LineTo2.y));

  if (is_stuck)
  {
	  // Draw a nice circle to show the stickness...
  	  dc.PaintConnectPoint( m_point_b );
  }
}


// Store the no connect in the drawing
CDrawingObject* CDrawNoConnect::Store()
{
  CDrawNoConnect *NewObject;

  is_stuck = FALSE;

  NewObject = new CDrawNoConnect(m_pDesign);

  *NewObject = *this;

  m_pDesign->Add(NewObject);

  return NewObject;
}



