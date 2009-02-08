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




/////// The CDrawRectOutline class /////////




// Redraw this object (including the handles)
void CDrawRectOutline::Display( BOOL erase  )
{
	CDRect r( m_point_a.x,m_point_a.y,m_point_b.x,m_point_b.y);
	m_pDesign->InvalidateRect( r, erase, 10 );
}

void CDrawRectOutline::PaintHandles( CContext &dc )
{
	// Put some handles around this object
	CDRect r(m_point_a.x,m_point_a.y,m_point_b.x,m_point_b.y);

	dc.PaintTracker( r );
}


// Move fields of this object about
int CDrawRectOutline::IsInsideField(CDPoint p)
{
	CPoint q(m_pDesign->GetTransform().Scale(p));
	CRect rect(m_pDesign->GetTransform().Scale( CDRect(m_point_a.x,m_point_a.y,m_point_b.x,m_point_b.y) ));

	rect.NormalizeRect();
	CRectTracker	tracker( rect, CRectTracker::dottedLine | CRectTracker::resizeOutside  );
	int r = tracker.HitTest( q );

	if (r == 8)
	{
		r = 11;
	}
	return r;
}

void CDrawRectOutline::MoveField(int w, CDPoint r)
{
	Display();

	CDRect rect( m_point_a.x, m_point_a.y, m_point_b.x, m_point_b.y );
	rect.NormalizeRect();

	switch (w)
	{
	case CRectTracker::hitTopLeft:
		rect.left += r.x;
		rect.top += r.y;
		break;
	case CRectTracker::hitTopRight:
		rect.right += r.x;
		rect.top += r.y;
		break;
	case CRectTracker::hitBottomRight:
		rect.right += r.x;
		rect.bottom += r.y;
		break;
	case CRectTracker::hitBottomLeft:
		rect.left += r.x;
		rect.bottom += r.y;
		break;	
	case CRectTracker::hitTop:
		rect.top += r.y;
		break;		
	case CRectTracker::hitRight:
		rect.right += r.x;
		break;		
	case CRectTracker::hitBottom:
		rect.bottom += r.y;
		break;		
	case CRectTracker::hitLeft:
		rect.left += r.x;
		break;
	case 11:
	case CRectTracker::hitMiddle:
		rect += r;
		break;		
	}

	if (m_point_a.x < m_point_b.x)
	{
		m_point_a.x = rect.left;
		m_point_b.x = rect.right;
	}
	else
	{
		m_point_b.x = rect.left;
		m_point_a.x = rect.right;
	}

	if (m_point_a.y < m_point_b.y)
	{
		m_point_a.y = rect.top;
		m_point_b.y = rect.bottom;
	}
	else
	{
		m_point_b.y = rect.top;
		m_point_a.y = rect.bottom;
	}

	Display();
}

int CDrawRectOutline::SetCursorEdit( CDPoint p )
{
	return IsInsideField( p );
}


/////// The Object classes /////////




