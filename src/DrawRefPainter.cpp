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




////// The Symbol reference painter object //////

ObjType CDrawRefPainter::GetType()
{
  return xNoDraw;
}


// Add a new reference to the symbol
void CDrawRefPainter::LButtonDown(CDPoint p, CDPoint no_snap_p)
{
	m_point_a = m_point_b = p;

	m_pDesign->BeginNewChangeSet();

	// No, so see if any of the other objects can be moved...
	drawingIterator it = m_pDesign->GetDrawingBegin();
	while (it != m_pDesign->GetDrawingEnd()) 
	{
		CDrawingObject *pointer = *it;

		if (pointer->IsInside(no_snap_p.x,no_snap_p.x,no_snap_p.y,no_snap_p.y))
		{
			// If this is a method then we can reference it
			if (pointer->GetType() == xMethodEx3) 
			{
				CDrawMethod *pMethod = (CDrawMethod *)pointer;
				m_pDesign->MarkChangeForUndo( pMethod );
				pMethod->AddReference( m_starting_point, false );
				pMethod->Display( TRUE );
			}
		}

		++ it;
	}
}

// Remove a reference from the symbol
void CDrawRefPainter::RButtonUp(CDPoint p, CDPoint no_snap_p )
{
	m_point_a = m_point_b = p;

	m_pDesign->BeginNewChangeSet();

	// No, so see if any of the other objects can be moved...
	drawingIterator it = m_pDesign->GetDrawingBegin();
	while (it != m_pDesign->GetDrawingEnd()) 
	{
		CDrawingObject *pointer = *it;

		if (pointer->IsInside(no_snap_p.x,no_snap_p.x,no_snap_p.y,no_snap_p.y))
		{
			// If this is a method then we can reference it
			if (pointer->GetType() == xMethodEx3) 
			{
				CDrawMethod *pMethod = (CDrawMethod *)pointer;
				m_pDesign->MarkChangeForUndo( pMethod );
				pMethod->RemoveReference();
				pMethod->Display( TRUE );
			}
		}
		
		++ it;
	}
}


void CDrawRefPainter::Display( BOOL erase )
{
}

BOOL CDrawRefPainter::RButtonDown(CDPoint p, CDPoint s)
{
	return TRUE;
}

