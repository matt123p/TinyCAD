/*
	TinyCAD program for schematic capture
	Copyright 1994/1995/2002 Matt Pyne.

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


// This is used for the construction of this object
CDrawRuler::CDrawRuler(CTinyCadDoc *pDesign, BOOL new_horiz )
	: CDrawingObject(pDesign) 
{ 
	a = CPoint(0,0); 
	horiz = new_horiz;
};


CDrawRuler::~CDrawRuler() 
{ 
}

BOOL CDrawRuler::IsConstruction()
{
	return TRUE;
}

ObjType CDrawRuler::GetType()
{ 
	return xRuler; 
}


void CDrawRuler::Paint(Context &dc,paint_options options)
{
	CPoint text_point;

	switch (options)
	{
	case draw_selected:
	case draw_handles:
	  dc.SelectPen(PS_SOLID,1,cRULER_CLK);
	  dc.SetTextColor( cRULER_CLK );
	  break;
	case draw_selectable:
	  dc.SelectPen(PS_DOT,1,cRULER_CLK);
	  dc.SetTextColor( cRULER_CLK );
	  break;
	default:
		dc.SelectPen(PS_DOT,1,cRULER);
		dc.SetTextColor( cRULER );
	}

	CString pos = m_pDesign->GetOptions()->PointToDisplay(a,horiz);
	
	// Select the font
	dc.SelectFont(*m_pDesign->GetOptions()->GetFont(fRULE),horiz ? 2 : 0, FALSE);
	
	CSize text_size = dc.GetTextExtent( pos );

	// Draw this ruler, either horizontally or
	// vertically
	if (horiz)
	{
		b = CPoint( m_pDesign->GetPageBoundries().x + 150, a.y );
		a = CPoint(dc.theTransform.DeScale( CPoint(0,0) ).x, a.y );
		dc.TextOut( a.x, a.y, pos );
		
		text_rect = CRect( a.x, a.y, a.x + text_size.cx, a.y - text_size.cy ); 
	}
	else
	{
		a = CPoint( a.x, dc.theTransform.DeScale( CPoint(0,RULER_WIDTH) ).y );
		b = CPoint( a.x, m_pDesign->GetPageBoundries().y + 150 );
		dc.TextOut( a.x, a.y + text_size.cy * 2, pos );
		text_rect = CRect( a.x, a.y, a.x - text_size.cy, a.y + text_size.cx ); 
	}
	dc.MoveTo( a );
	dc.LineTo( b );	
}


int CDrawRuler::IsInside(int left,int right,int top,int bottom)
{
	if (horiz)
	{
		return top <= a.y && bottom >= a.y;
	}
	else
	{
		return left <= a.x && right >= a.x;
	}
}

int CDrawRuler::SetCursorEdit( CPoint p )
{
	if (!IsInside(p.x,p.x,p.y,p.y))
	{
		return -1;
	}

	if (horiz)
	{
		return 4;
	}
	else
	{
		return 5;
	}
}

void CDrawRuler::Store()
{
  CDrawRuler *NewObject;

  NewObject = new CDrawRuler(m_pDesign, horiz);
  *NewObject = *this;

  m_pDesign->Add(NewObject);

  m_pDesign->SetRepeat(NULL);
}

void CDrawRuler::Save(CStream &archive)
{
	WritePoint(archive,a);
	archive << horiz;
}

void CDrawRuler::Load(CStream &archive)
{
	a = ReadPoint(archive);
	archive >> horiz;
}

void CDrawRuler::Display( BOOL erase )
{
	// Invalidate the symbol
	CRect r( a.x,a.y,b.x,b.y);	
	m_pDesign->InvalidateRect( r, erase, 4 );
	m_pDesign->InvalidateRect( text_rect, erase, 4 );
}

void CDrawRuler::Place(CPoint p,CPoint)
{
  Display();
  a=p;
  b=p;
  Store();
  Display();	// Write to screen

}

void CDrawRuler::Move(CPoint p, CPoint no_snap_p)
{
  Display();

  if (horiz)
  {
	 p.x = a.x;
	 b += a-p;
  }
  else
  {
	 p.y = a.y;
	 b += a-p;
  }
  a=p;

  Display();
}


CString CDrawRuler::GetName()
{
	return "Ruler";
}

int CDrawRuler::getMenuID()
{
	return horiz ? ID_RULER_HORIZ : ID_RULER_VERT;
}
