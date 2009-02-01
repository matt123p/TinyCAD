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
#define theArea CDRect(a.x,a.y,b.x,b.y)




////// The block import object //////


CDrawBlockImport::CDrawBlockImport(CTinyCadDoc *pDesign)
: CDrawingObject( pDesign )
{
  m_segment=1;
}

void CDrawBlockImport::Import()
{  
  if (m_pDesign->IsSelected()) 
  {
	m_point_a=m_point_b=CDPoint(0,0);

	selectIterator it = m_pDesign->GetSelectBegin();
	while ( it != m_pDesign->GetSelectEnd() ) 
	{
		CDrawingObject *obj=*it;

		m_point_b.x = max(m_point_b.x,max(obj->m_point_a.x,obj->m_point_b.x));
		m_point_b.y = max(m_point_b.y,max(obj->m_point_a.y,obj->m_point_b.y));
		obj->Display();

		++ it;
	}

	m_point_b = m_pDesign->m_snap.Snap( m_point_b );
	m_segment = 0;

  }

  m_pDesign->PostPaste();

  m_pDesign->MarkSelectChangeForUndo();
}


void CDrawBlockImport::EndEdit()
{
  RButtonDown(CDPoint(0,0), CDPoint(0,0));
}


void CDrawBlockImport::LButtonDown(CDPoint p, CDPoint)
{
  // New undo level for each placement...
  m_pDesign->BeginNewChangeSet();


  if (!m_segment)
  {
	m_pDesign->UnSelect();
	m_segment=1;

	// Now de-select back to the edit item
	m_pDesign->SelectObject( new CDrawEditItem( m_pDesign ) );
  }
}


void CDrawBlockImport::Move(CDPoint p, CDPoint no_snap_p)
{
  if (!m_segment) {
  	CDPoint r;

  	r.x=p.x-m_point_b.x;
  	r.y=p.y-m_point_b.y;

	if (r.x!=0 || r.y!=0) 
	{
		m_pDesign->SelectMove(r);
		m_point_b=p;
	}
  }
}



BOOL CDrawBlockImport::RButtonDown(CDPoint p, CDPoint s)
{
  // If necessary remove the line from the screen
  if (!m_segment) {
	m_pDesign->SelectDelete();
	m_pDesign->Invalidate();
	m_segment=1;
	m_point_a = m_point_b;
  }

  return FALSE;
}



