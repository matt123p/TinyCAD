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
#include "JunctionUtils.h"
#include "TinyCadMultiDoc.h"

////// The Line Class //////


CDrawLine::CDrawLine(CTinyCadDoc *pDesign, ObjType NewType)
: CDrawingObject( pDesign )
, m_drag_utils_a( pDesign )
, m_drag_utils_b( pDesign )
{
	has_placed = FALSE;
	m_segment=1;
	xtype=NewType;
	is_stuck = FALSE;
	is_junction = FALSE;
	m_re_edit = FALSE;

	switch (xtype) 
	{
		case xBus:
			m_use_default_style = TRUE;
			m_style = fLINE;
			break;
		case xLineEx2:
			m_use_default_style = FALSE;
			m_style = m_pDesign->GetOptions()->GetCurrentStyle(GetType());
			break;
		case xDash:
			m_use_default_style = FALSE;
			m_style = fDASH;
			break;
		case xWire:
			m_use_default_style = TRUE;
			m_style = fLINE;
			break;
		default:
			m_use_default_style = FALSE;
			m_style = fLINE;
	}	
}


void CDrawLine::TagResources()
{
	if (!m_use_default_style)
	{
		m_pDesign->GetOptions()->TagStyle(m_style);
	}
}


const TCHAR* CDrawLine::GetXMLTag(ObjType t)
{
	switch (t) 
	{
	case xBus:
		return _T("BUS");
	case xWire:
		return _T("WIRE");
	default:
		return _T("LINE");
	}	
}


// Load and save to an XML file
void CDrawLine::SaveXML( CXMLWriter &xml )
{
	xml.addTag(GetXMLTag(xtype));

	switch (xtype) 
	{
	case xBus:
	case xWire:
		if (!m_use_default_style)
		{
			xml.addAttribute( _T("style"), m_style );
		}
		break;
	default:
		xml.addAttribute( _T("style"), m_style );
		break;
	}	

	xml.addAttribute( _T("a"), m_point_a );
	xml.addAttribute( _T("b"), m_point_b );
	xml.closeTag();
}

void CDrawLine::LoadXML( CXMLReader &xml )
{
	switch (xtype) 
	{
	case xBus:
	case xWire:
		if (xml.getAttribute( _T("style"), m_style ))
		{
			m_style = m_pDesign->GetOptions()->GetNewStyleNumber(m_style);
			m_use_default_style = FALSE;
		}
		else
		{
			m_use_default_style = TRUE;
		}
		break;
	default:
		m_use_default_style = FALSE;
		xml.getAttribute( _T("style"), m_style );
		m_style = m_pDesign->GetOptions()->GetNewStyleNumber(m_style);
		break;
	}	

	xml.getAttribute( _T("a"), m_point_a );
	xml.getAttribute( _T("b"), m_point_b );
}


// Load the line from a file
void CDrawLine::Load(CStream& archive )
{
 	m_point_a = ReadPoint(archive);
   	m_point_b = ReadPoint(archive);

	switch (xtype) {
		case xDash:
		case xLine:
			// Premote this type
			m_use_default_style = FALSE;
			xtype = xLineEx2;
			break;
		case xLineEx:
			// Load the line thickness
			BYTE LineThickness;
			archive >> LineThickness;
			LineStyle l;
			l.Colour = cBLACK;
			l.Style = PS_SOLID;
			l.Thickness = LineThickness;
			m_use_default_style = FALSE;
			m_style = m_pDesign->GetOptions()->AddStyle(&l);
			m_style = m_pDesign->GetOptions()->GetNewStyleNumber(m_style);
			xtype = xLineEx2;
			break;
		case xLineEx2:
			m_use_default_style = FALSE;
			archive >> m_style;
			m_style = m_pDesign->GetOptions()->GetNewStyleNumber(m_style);
			break;
		default:
			m_use_default_style = TRUE;
			break;
	}	
}


void CDrawLine::ToAngle()
{
  double FromX,FromY,FromC;

  switch (g_EditToolBar.m_DrawLineEdit.GetAngle()) {
	case LINEBOX_90:
		if (m_point_a.x==m_point_b.x)
			g_EditToolBar.m_DrawLineEdit.mode=2;
		if (m_point_a.y==m_point_b.y)
			g_EditToolBar.m_DrawLineEdit.mode=1;
		if (g_EditToolBar.m_DrawLineEdit.mode==0)
			g_EditToolBar.m_DrawLineEdit.mode=1;
		break;
	case LINEBOX_45:
		g_EditToolBar.m_DrawLineEdit.mode=0;
		// Which 45degree slope is it closest to?
		FromX=fabs(m_point_a.x-m_point_b.x);
		FromY=fabs(m_point_a.y-m_point_b.y);
		FromC=fabs(FromX-FromY);
		// Is it closest to the X axis?
		if (FromX<FromY && FromX<FromC)
			m_point_b.x=m_point_a.x;
		// Is it closest to the Y axis?
		else if (FromY<FromC)
			m_point_b.y=m_point_a.y;
		// Must be closest to the 45 degree line
		else
			m_point_b.x=m_point_a.x+(m_point_b.x>m_point_a.x ? FromY : -FromY);
		break;
	default:
		g_EditToolBar.m_DrawLineEdit.mode=0;
  }
}


BOOL CDrawLine::RButtonDown(CDPoint p, CDPoint s)
{
  BOOL r = !m_segment;


  // Remove any junctions we may have had in place...
  CJunctionUtils j(m_pDesign);
  j.CheckJunctionRequirement( m_point_a, false );
  j.CheckJunctionRequirement( m_point_b, false );

  Display();
 
  m_segment=1;
  m_point_a = m_point_b;
  has_placed = FALSE;
  is_stuck = FALSE;

  return r;
}


void CDrawLine::DblLButtonDown(CDPoint p,CDPoint s)
{
	// Second line..
	LButtonDown(p,s);

	// .. and end
	RButtonDown(p,s);
}


void CDrawLine::LButtonUp(CDPoint p, CDPoint)
{
	// Only do this if this is the first
	// m_segment we have placed...
	if (has_placed)
	{
		return;
	}

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
		// Ok, we can place
		if (!m_segment && !is_stuck)
		{
			// First line
			LButtonDown(p,p);
		}
		// Second line..
		LButtonDown(p,p);

		// .. and end
		RButtonDown(p,p);
	}
	else
	{
		has_placed = TRUE;
	}
}


void CDrawLine::NewOptions()
{
	Display();

	if (m_re_edit)
	{
		bool changed = false;

		if (	m_use_default_style != g_EditToolBar.m_LineEdit.m_use_default_style
			 || m_style != m_pDesign->GetOptions()->GetCurrentStyle(GetType()) )
		{
			m_use_default_style = g_EditToolBar.m_LineEdit.m_use_default_style;
			if (!m_use_default_style) 
			{
				m_style = m_pDesign->GetOptions()->GetCurrentStyle(GetType());
			}

			// Do we effect the entire net?
			if (g_EditToolBar.m_LineEdit.m_bEntire_net)
			{
				// We must make the latest network..
				CNetList n;
				n.MakeNet( static_cast<CTinyCadMultiDoc*>(m_pDesign->GetParent()) );

				// Yep, so better update it...
				drawingIterator i = m_pDesign->GetDrawingBegin();
				while (i != m_pDesign->GetDrawingEnd())
				{
					if ((*i)->GetType() == GetType() && *i != this)
					{
						CDrawLine *pLine = static_cast<CDrawLine*>(*i);

						if (pLine->getNetwork() == getNetwork())
						{
							pLine->m_use_default_style = m_use_default_style;
							pLine->m_style = m_style;
							pLine->Display();
						}
					}
					++ i;
				}
			}
		}
	}

	Display();
}

int  CDrawLine::GetContextMenu()
{
	return IDR_LINETOOL_EDIT;
}

// For the context menu
void CDrawLine::ContextMenu( CDPoint p, UINT id )
{
	if (m_segment)
	{
		// Determine the nearest end to the pointer
		CLineUtils l( m_point_a, m_point_b );
		double d;
		l.IsPointOnLine( p, d );

		switch (id)
		{
		case ID_CONTEXT_MAKEVERTICAL:
			if (m_point_a.y != m_point_b.y)
			{
				if (d < 0.5)
				{
					MoveField( 2, CDPoint( m_point_a.x-m_point_b.x, 0) );
				}
				else
				{
					MoveField( 1, CDPoint( m_point_b.x-m_point_a.x, 0) );
				}
			}
			break;
		case ID_CONTEXT_MAKEHORIZONTAL:
			if (m_point_a.x != m_point_b.x)
			{
				if (d < 0.5)
				{
					MoveField( 2, CDPoint( 0,m_point_a.y-m_point_b.y) );
				}
				else
				{
					MoveField( 1, CDPoint( 0,m_point_b.y-m_point_a.y) );
				}

			}
			break;
		}
	}
}


CString CDrawLine::GetName() const
{

  switch (xtype) {
	case xWire:
		return "Wire";
	case xBus:
		return "Bus";
	default:
		return "Line";
  }
}

int CDrawLine::getMenuID() 
{ 
  switch (xtype) {
	case xWire:
		return IDM_TOOLWIRE;
	case xBus:
		return IDM_TOOLBUS;
	default:
		return IDM_TOOLPOLYGON;
  }
}


void CDrawLine::LButtonDown(CDPoint p, CDPoint no_snap_p)
{
  // New undo level for each placement...
  m_pDesign->BeginNewChangeSet();


  p = GetStickyPoint( no_snap_p );

  if (m_segment) 
  {
	m_point_a=p;
	m_point_b=p;
	m_segment=!m_segment;

	  if (is_junction)
	  {
		  m_pDesign->AddNewJunction( m_point_a );
	  }

	  // If there was an object of the same type under this point,
	  // then take it's colour & style
	  drawingIterator i = m_pDesign->GetDrawingBegin();
	  while (i != m_pDesign->GetDrawingEnd())
	  {
		  if ((*i)->GetType() == xtype)
		  {
			  // Is this within our start range?
			  CDrawLine *line = static_cast<CDrawLine*>(*i);
			  CLineUtils l( line->m_point_a, line->m_point_b );
			  double d;
			  if (l.IsPointOnLine( p,d ))
			  {
				  m_use_default_style = line->m_use_default_style;
				  m_style = line->m_style;
				  break;
			  }
		  }
		  ++ i;
	  }
  } 
  else 
  {
	m_point_b=p;
	ToAngle();
	// Remove from screen
	Display();
	CDPoint c=m_point_b;

	switch (g_EditToolBar.m_DrawLineEdit.mode) {
		case 1: m_point_b=CDPoint(m_point_b.x,m_point_a.y);
			break;
		case 2: m_point_b=CDPoint(m_point_a.x,m_point_b.y);
			break;
	}
	
	BOOL was_stuck = is_stuck;
	if (is_stuck && g_EditToolBar.m_DrawLineEdit.mode > 0)
	{
		int old_is_junction = is_junction;
		is_junction = FALSE;

		// Place two lines
		is_stuck = FALSE;
		if (m_point_a != m_point_b)
		{
			// Store and then Check for merge & junctions
			m_drag_utils_a.MergeLinePoint( Store() );
		}
		m_point_a = m_point_b;
		m_point_b = c;

		is_junction = old_is_junction;
	}
	is_stuck = FALSE;
	if (m_point_a != m_point_b)
	{
	    // Store and then Check for merge & junctions
	    m_drag_utils_a.MergeLinePoint( Store() );
	}
	Display();	// Write to screen

	if (!was_stuck)
	{
		m_point_a=m_point_b;
		m_point_b=c;
		m_segment=0;
		ToAngle();
	}
	else
	{
		RButtonDown(p,no_snap_p);
	}
  }
  Display();
}


void CDrawLine::Move(CDPoint p, CDPoint no_snap_p)
{
  if (!m_segment) 
  {
	  BOOL new_is_stuck = FALSE;
	  BOOL new_is_junction = FALSE;

	  // Are we a line to be snapped to pins?
	  if (xtype == xWire)
	  {
		  p = m_pDesign->GetStickyPoint(no_snap_p, TRUE,TRUE,new_is_stuck,new_is_junction);
	  }


	  if (p != m_point_b)
	  {
		Display();
		is_stuck = new_is_stuck;
		is_junction = new_is_junction;
		m_point_b=p;
		ToAngle();
		Display();
	  }
  }
  else
  {
	  Display();
	  m_point_b = GetStickyPoint( no_snap_p );
	  m_point_a = m_point_b;
	  Display();
  }
}


// Snap to a pin if we are drawing a wire...
//
CDPoint CDrawLine::GetStickyPoint( CDPoint no_snap_q )
{
  // Are we a line to be snapped to pins?
  if (xtype != xWire)
  {
	  return m_pDesign->m_snap.Snap(no_snap_q);
  }

  return m_pDesign->GetStickyPoint(no_snap_q, TRUE,TRUE,is_stuck,is_junction);
}



ObjType CDrawLine::GetType()
{
  return xtype;
}

void CDrawLine::Display( BOOL erase )
{
	LineStyle *theStyle = m_pDesign->GetOptions()->GetStyle(m_style);
	int width = max(2,theStyle->Thickness) * 2;


  CDPoint r = m_point_a;
  if (!m_segment) 
  {
	  switch (g_EditToolBar.m_DrawLineEdit.mode) {
		case 1: 
			{
				r = CDPoint(m_point_b.x,m_point_a.y);
				CLineUtils l(m_point_a, r);
				l.SplitForDisplay( m_pDesign,erase, width );
			}
			break;
		case 2:
			{
				r = CDPoint(m_point_a.x,m_point_b.y);
				CLineUtils l(m_point_a, r);
				l.SplitForDisplay( m_pDesign,erase, width );
			}
			break;
	  }
  }

  CLineUtils l( r, m_point_b );
  l.SplitForDisplay( m_pDesign, erase, width );

 if (is_stuck)
 {
	 CDRect r;
	 r.left = m_point_b.x - HIGHLIGHT_SIZE;
	 r.right = m_point_b.x + HIGHLIGHT_SIZE;
	 r.top = m_point_b.y - HIGHLIGHT_SIZE;
	 r.bottom = m_point_b.y + HIGHLIGHT_SIZE;
	 m_pDesign->InvalidateRect( r, erase, 3 );
 }
}


void CDrawLine::PaintHandles( CContext &dc )
{
	dc.SelectBrush(cBLACK);
	dc.SelectPen(PS_SOLID,0,cBLACK);

	CDRect r1(m_point_a.x-2,m_point_a.y-2,m_point_a.x+2,m_point_a.y+2);
	CDRect r2(m_point_b.x-2,m_point_b.y-2,m_point_b.x+2,m_point_b.y+2);
	dc.Rectangle(r1);
	dc.Rectangle(r2);
}


// Move fields of this object about
int CDrawLine::IsInsideField(CDPoint p)
{
	CLineUtils l(m_point_a,m_point_b);
	CDRect r1(m_point_a.x-2,m_point_a.y-2,m_point_a.x+2,m_point_a.y+2);
	CDRect r2(m_point_b.x-2,m_point_b.y-2,m_point_b.x+2,m_point_b.y+2);

	if (r1.PtInRect(p))
	{
		return 1;
	}
	if (r2.PtInRect(p))
	{
		return 2;
	}

	return -1;
}

BOOL CDrawLine::CanControlAspect()
{
	return FALSE;
}

void CDrawLine::MoveField(int w, CDPoint r)
{
	Display();

	if (r != CDPoint(0,0))
	{
		// Is the control key down?
		BOOL do_move = GetAsyncKeyState(VK_CONTROL) < 0;
		if (!m_pDesign->GetOptions()->GetAutoDrag())
		{
		  do_move = !do_move;
		}

		// Do we snap back onto the grid?
		if (m_pDesign->m_snap.GetGridSnap())
		{
			// Ok, force back onto the grid...
			if (w == 1)
			{
				CDPoint q = m_pDesign->m_snap.Snap(m_point_a + r);
				r = q - m_point_a;
			}
			if (w == 2)
			{
				CDPoint q = m_pDesign->m_snap.Snap(m_point_b + r);
				r = q - m_point_b;
			}

		}


		if (do_move)
		{
			CJunctionUtils j( m_pDesign );
			j.AddObjectToTodo( this );

			if (w == 1)
			{
				m_point_a += r;
			}
			if (w == 2)
			{
				m_point_b += r;
			}

			j.AddObjectToTodo( this );
			j.CheckTodoList( false );
		}
		else
		{
			if (w == 1)
			{
				m_drag_utils_a.Drag( r );
			}
			if (w == 2)
			{
				m_drag_utils_b.Drag( r );
			}
		}
	}

	Display();
}

int CDrawLine::SetCursorEdit( CDPoint p )
{
	int s = IsInsideField( p );

	if (s != -1)
	{
		// Always display the same cursor
		s = 8;
	}
	else if (IsInside(p.x,p.x,p.y,p.y))
	{
		s = 11;
	}

	return s;
}



// Display the line on the screen!
void CDrawLine::Paint(CContext &dc,paint_options options)
{
	if (m_use_default_style)
	{
		switch (xtype)
		{
		case xWire:
			dc.SelectPen( PS_SOLID,1, m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::WIRE ), options );
			break;
		case xBus:
			dc.SelectPen( PS_SOLID,5, m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::BUS ), options );
			break;
		default:
			dc.SelectPen(m_pDesign->GetOptions()->GetStyle(m_style), options);
			break;
		}
	}
	else
	{
		dc.SelectPen(m_pDesign->GetOptions()->GetStyle(m_style), options);
	}

  dc.SetROP2(R2_COPYPEN);

  dc.MoveTo(m_point_a);

  if (!m_segment) {
	  switch (g_EditToolBar.m_DrawLineEdit.mode) {
		case 1: dc.LineTo(CDPoint(m_point_b.x,m_point_a.y));
			break;
		case 2:	dc.LineTo(CDPoint(m_point_a.x,m_point_b.y));
			break;
	  }
  }
  dc.LineTo(m_point_b);

  if (is_stuck)
  {
	  // Draw a nice circle to show the stickness...
	  dc.PaintConnectPoint( m_point_b );

	  // Do we need a junction
	  if (is_junction)
	  {
	    int js=JUNCTION_SIZE;
		CDPoint br,tl;
		br=CDPoint(m_point_b.x+js+1,m_point_b.y+js+1);
		tl=CDPoint(m_point_b.x-js,m_point_b.y-js);

		dc.SetROP2(R2_COPYPEN);

		dc.SelectPen(PS_SOLID,1,m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::JUNCTION) );
		dc.SelectBrush(m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::JUNCTION));
  	    dc.Ellipse(CDRect(tl.x,tl.y,br.x,br.y));
	  }
  }

}


// Store the line in the drawing
CDrawingObject* CDrawLine::Store()
{
  // Only do this is we do not have zero length
  CDrawLine *NewObject;
  NewObject = new CDrawLine(m_pDesign, xtype);

  // Copy the details
  m_segment=1;
  *NewObject = *this;
  NewObject->is_junction = FALSE;
  NewObject->is_stuck = FALSE;

  m_pDesign->Add(NewObject);

  return NewObject;
}


void CDrawLine::BeginEdit(BOOL re_editi)
{
	m_re_edit = re_editi;

	if (!m_use_default_style)
	{
		m_pDesign->GetOptions()->SetCurrentStyle(GetType(), m_style);
	}

	if (m_re_edit)
	{
		m_drag_utils_a.Begin( m_point_a,m_point_a );
		m_drag_utils_b.Begin( m_point_b,m_point_b );
		g_EditToolBar.m_LineEdit.m_use_default_style = m_use_default_style;
		g_EditToolBar.m_LineEdit.Open(m_pDesign,this);
	}
	else
	{
		m_segment=1;
		g_EditToolBar.m_DrawLineEdit.Open(m_pDesign,this);
		g_EditToolBar.m_DrawLineEdit.mode=0;
	}
}

void CDrawLine::EndEdit()
{
  Display();
  if (!m_re_edit)
  {
	  g_EditToolBar.m_DrawLineEdit.Close();
  }
  else
  {
	  g_EditToolBar.m_LineEdit.Close();

	  m_drag_utils_a.End( true );
	  m_drag_utils_b.End( true );

	  CDragUtils q( m_pDesign );
	  q.MergeLinePoint( this );
  }
}

BOOL CDrawLine::IsInside(double left,double right,double top,double bottom)
{
	CLineUtils l(m_point_a,m_point_b);
	return l.IsInside(left,right,top,bottom);
}

double CDrawLine::DistanceFromPoint( CDPoint p )
{
	CLineUtils l(m_point_a,m_point_b);
	CDPoint d;
	return l.DistanceFromPoint(p, d );
}

