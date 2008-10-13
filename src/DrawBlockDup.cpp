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

#include "stdafx.h"
#include "TinyCadView.h"
#include "diag.h"
#include "colour.h"


// This defines the block edit functions

// The rectangle area slightly re-written
#define theArea CDRect(m_point_a.x,m_point_a.y,m_point_b.x,m_point_b.y)

////// The block duplicate object //////

void CDrawBlockDup::Paint(CContext &dc,paint_options options)
{

  dc.SelectBrush();
  dc.SelectPen(PS_DOT,1,cBLOCK);
	dc.SetROP2(R2_COPYPEN);

  dc.Rectangle(theArea);
}


CDrawBlockDup::CDrawBlockDup(CTinyCadDoc *pDesign)
: CDrawingObject( pDesign )
{
  m_segment=1;
  placed=0;
  m_point_a=m_point_b=CDPoint(0,0);
}

int CDrawBlockDup::SetCursor( CDPoint p )
{
	return placed ? -1 : 12;
}


void CDrawBlockDup::EndEdit()
{
  if (!m_segment | placed) 
  {
	m_pDesign->UnSelect();
	Display();
  }
}

void CDrawBlockDup::LButtonDown(CDPoint p, CDPoint)
{
  if (placed) 
  {
	Display();
	m_pDesign->BeginNewChangeSet();
	m_pDesign->SelectDup();
	m_point_a = m_point_b;
	m_pDesign->SetModifiedFlag( TRUE );
	Display();
  } else if (m_segment) {
	m_point_a=p;
	m_point_b=p;
	m_segment=0;
	placed=0;
	Display();
  } 
  else 
  {
	Display();
	m_point_b=p;
	placed=1;
	m_segment=0;

	// Now select all the objects in this rectangle
	m_pDesign->BeginNewChangeSet();
	m_pDesign->Select(m_point_a,m_point_b);
	m_pDesign->SelectDup();
  }
}


void CDrawBlockDup::Move(CDPoint p, CDPoint no_snap_p)
{
  if (placed) {
  	CDPoint r;

  	r.x=p.x-m_point_b.x;
  	r.y=p.y-m_point_b.y;

	if (r.x!=0 || r.y!=0) {
		Display();
		m_pDesign->SelectMove(r);
		m_point_a.x += r.x;
		m_point_a.y += r.y;
		m_point_b=p;
		Display();
	}
  } else if (!m_segment) {
	Display();
	m_point_b=p;
	Display();
  }
}



BOOL CDrawBlockDup::RButtonDown(CDPoint p, CDPoint s)
{
  BOOL r = m_pDesign->IsSelected();

  // If necessary remove the line from the screen
  if (!m_segment | placed) {
	Display();
	if (placed)
	{
		m_pDesign->SelectDelete();
	}
	m_segment=1;
 	placed=0;
	m_point_a = m_point_b;
	m_pDesign->Invalidate();
  }

  return r;
}


