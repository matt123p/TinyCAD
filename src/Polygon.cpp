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
#include <math.h>
#include "TinyCadView.h"
#include "diag.h"
#include "colour.h"
#include "option.h"
#include "LineUtils.h"

#define OUTLINE_SPACER 5


////// The Polygon Class //////


CDrawPolygon::CDrawPolygon(CTinyCadDoc *pDesign, ObjType NewType)
: CDrawingObject( pDesign )
{
	m_segment=1;
	xtype=NewType;
	m_re_edit = FALSE;

	switch (xtype) 
	{
	case xPolygon:
	case xLineEx2:
		Style = m_pDesign->GetOptions()->GetCurrentStyle(GetType());
		Fill = fsNONE;
		break;
	case xDash:
		Style = fDASH;
		Fill = fsNONE;
		break;
	default:
		Style = fLINE;
		Fill = fsNONE;
	}
}



void CDrawPolygon::TagResources()
{
   m_pDesign->GetOptions()->TagStyle(Style);
   m_pDesign->GetOptions()->TagFillStyle(Fill);
}

const TCHAR* CDrawPolygon::GetXMLTag()
{
	return _T("POLYGON");
}

// Load and save to an XML file
void CDrawPolygon::SaveXML( CXMLWriter &xml )
{
	if (!IsEmpty())
	{
		xml.addTag(GetXMLTag());

		xml.addAttribute( _T("pos"), m_point_a );
		xml.addAttribute( _T("style"), Style );
		xml.addAttribute( _T("fill"), Fill );

		arcpointCollection::iterator it = m_handles.begin();
		while (it != m_handles.end())
		{
			xml.addTag(_T("POINT"));
			xml.addAttribute( _T("pos"), 
				CDPoint( (*it).x, (*it).y ) );
			xml.addAttribute( _T("arc"), (*it).arc );
			xml.closeTag();
			++ it;
		}

		xml.closeTag();
	}
}

void CDrawPolygon::LoadXML( CXMLReader &xml )
{
	xml.getAttribute( _T("pos"), m_point_a );
	xml.getAttribute( _T("style"), Style );
	xml.getAttribute( _T("fill"), Fill );


	m_handles.erase( m_handles.begin(), m_handles.end() );

	xml.intoTag();

	CString name;
	while (xml.nextTag(name))
	{
		if (name == _T("POINT"))
		{
			CDPoint p;
			int arc = 0;
			xml.getAttribute( _T("pos"), p );
			xml.getAttribute( _T("arc"), arc );

			CArcPoint q;
			q.x = p.x;
			q.y = p.y;
			q.arc = static_cast<CArcPoint::arc_type>(arc);
			m_handles.push_back( q );
		}
	}

	xml.outofTag();

	Style = m_pDesign->GetOptions()->GetNewStyleNumber(Style);

	// Calculate the Style nr here
	// This allows IsModified to correctly detect changes
	LineStyle lStyle = *m_pDesign->GetOptions()->GetStyle( Style );
	WORD line = m_pDesign->GetOptions()->AddStyle(&lStyle);
	m_pDesign->GetOptions()->SetCurrentStyle( GetType(), line );
	Style = line;

	Fill = m_pDesign->GetOptions()->GetNewFillStyleNumber(Fill);

	xtype = xArcEx2;

	CalcBoundingRect();
}


// Load the line from a file
void CDrawPolygon::Load(CStream& archive )
{
	m_handles.erase( m_handles.begin(), m_handles.end() );

	if (xtype != xPolygon)
	{
	 	m_point_a = ReadPoint(archive);
   		m_point_b = ReadPoint(archive);
		m_handles.push_back(CArcPoint(0,0));

		CDPoint p( m_point_b - m_point_a );
		if (xtype == xArcEx || xtype == xArcEx2)
		{
			m_handles.push_back(CArcPoint(p.x,p.y, CArcPoint::Arc_out));
		}
		else
		{
			m_handles.push_back(CArcPoint(p));
		}
	}

	switch (xtype) 
	{
		case xDash:
		case xLine:
			// Premote this type
			xtype = xLineEx2;
			break;
		case xArcEx:
		case xLineEx:
			// Load the line thickness
			BYTE LineThickness;
			archive >> LineThickness;
			LineStyle l;
			l.Colour = cBLACK;
			l.Style = PS_SOLID;
			l.Thickness = LineThickness;
			Style = m_pDesign->GetOptions()->AddStyle(&l);
			Style = m_pDesign->GetOptions()->GetNewStyleNumber(Style);
			Fill = fsNONE;
			if (xtype == xLine)
			{
				xtype = xLineEx2;
			}
			if (xtype == xArcEx)
			{
				xtype = xArcEx2;
			}
			break;
		case xArcEx2:
		case xLineEx2:
			archive >> Style;
			Fill = fsNONE;
			Style = m_pDesign->GetOptions()->GetNewStyleNumber(Style);
			break;
		case xPolygon:
			// Write out the points for this polygon
			{
				DWORD version;
				DWORD lines;  	
				archive >> version;
				archive >> lines;
				m_point_a = ReadPoint(archive);
				while (lines > 0)
				{
					CArcPoint q;
					if (version == 0)
					{
						q = ReadPoint( archive );
					}
					else
					{
						q.ReadPoint( archive );
					}
					m_handles.push_back( q );
					lines --;
				}
				archive >> Style;
				archive >> Fill;  	
				Style = m_pDesign->GetOptions()->GetNewStyleNumber(Style);
				Fill = m_pDesign->GetOptions()->GetNewFillStyleNumber(Fill);

			}
			break;
		case xArc:
			// Premote this type
			xtype = xArcEx2;
			break;
	}

	// Calculate the Style nr here
	// This allows IsModified to correctly detect changes
	LineStyle lStyle = *m_pDesign->GetOptions()->GetStyle( Style );
	WORD line = m_pDesign->GetOptions()->AddStyle(&lStyle);
	m_pDesign->GetOptions()->SetCurrentStyle( GetType(), line );
	Style = line;

	CalcBoundingRect();
}


// Rotate this object about a point
void CDrawPolygon::Rotate(CDPoint p,int dir)
{
  // Now rotate each point around the rotational point...
	arcpointCollection::iterator it = m_handles.begin();
	while (it != m_handles.end())
	{
		CArcPoint qp(*it + m_point_a);
		CArcPoint::arc_type at = (*it).arc;

		// Translate this point so the rotational point is the origin
		qp = CDPoint(qp.x-p.x,qp.y-p.y);

		// Perfrom the rotation
		switch (dir) {
		case 2: // Left
			qp = CDPoint(qp.y,-qp.x);
			if (it->arc == CArcPoint::Arc_in)
			{
				it->arc = CArcPoint::Arc_out;
			}
			else if (it->arc == CArcPoint::Arc_out)
			{
				it->arc = CArcPoint::Arc_in;
			}
			break;		
		case 3: // Right
			qp = CDPoint(-qp.y,qp.x);
			if (it->arc == CArcPoint::Arc_in)
			{
				it->arc = CArcPoint::Arc_out;
			}
			else if (it->arc == CArcPoint::Arc_out)
			{
				it->arc = CArcPoint::Arc_in;
			}
			break;
		case 4: // Mirror
			qp = CDPoint(-qp.x,qp.y);
			break;
		}

		if (dir != 4)
		{
			switch (at)
			{
			case CArcPoint::Arc_none:
				break;
			case CArcPoint::Arc_in:
				at = CArcPoint::Arc_out;
				break;
			case CArcPoint::Arc_out:
				at = CArcPoint::Arc_in;
				break;
			}
		}

		// Re-translate the points back to the original location
		*it = CArcPoint(qp.x+p.x,qp.y+p.y,at) - m_point_a;

		++ it;
	}

	// Now re-calc the bounding rectangle
	CalcBoundingRect();
}


// Re-calculate bounding rect
// Used when the polygon is inside a symbol.
//
void CDrawPolygon::CalcBoundingRect()
{
	// First flattern out the arcs
	FlatternPath();

	// Now Calc the bounding rect
	CDRect r;

	if (m_points.size() >0)
	{
		CDPoint q = m_points.front() + m_point_a;
		r = CDRect(static_cast<int>(q.x),static_cast<int>(q.y),static_cast<int>(q.x),static_cast<int>(q.y));
	}
	else
	{
		r = CDRect(m_point_a.x,m_point_a.y,m_point_a.x,m_point_a.y);
	}

	// Find the bounding rectangle
	pointCollection::iterator it = m_points.begin();
	while (it != m_points.end())
	{
		CDPoint qp = *it + m_point_a;

		r.left = static_cast<int>(min(r.left,qp.x));
		r.top = static_cast<int>(min(r.top,qp.y));
		r.right = static_cast<int>(max(r.right,qp.x));
		r.bottom = static_cast<int>(max(r.bottom,qp.y));

		++ it;
	}

	// Now move everything so that a and b
	// lie on the bounding rectangle

	CDPoint diff_a = CDPoint(r.left,r.top) - m_point_a;
	arcpointCollection::iterator itx = m_handles.begin();
	while (itx != m_handles.end())
	{
		*itx = *itx - diff_a;
		++ itx;
	}

	m_point_a = CDPoint(r.left,r.top);
	m_point_b = CDPoint(r.right,r.bottom);

	FlatternPath();
}


void CDrawPolygon::ToAngle()
{
  double FromX,FromY,FromC;
  
  CArcPoint last = m_point_a;
  if (m_handles.size() > 0)
  {
	last = m_handles.back() + m_point_a;
  }

  switch (g_EditToolBar.m_DrawPolyEdit.GetAngle()) 
  {
	case LINEBOX_90:
		if (last.x==m_point_b.x)
			g_EditToolBar.m_DrawPolyEdit.mode=2;
		if (last.y==m_point_b.y)
			g_EditToolBar.m_DrawPolyEdit.mode=1;
		if (g_EditToolBar.m_DrawPolyEdit.mode==0)
			g_EditToolBar.m_DrawPolyEdit.mode=1;
		break;
	case LINEBOX_45:
		g_EditToolBar.m_DrawPolyEdit.mode=0;
		// Which 45degree slope is it closest to?
		FromX=abs(last.x-m_point_b.x);
		FromY=abs(last.y-m_point_b.y);
		FromC=abs(FromX-FromY);
		// Is it closest to the X axis?
		if (FromX<FromY && FromX<FromC)
			m_point_b.x=static_cast<LONG>(last.x);
		// Is it closest to the Y axis?
		else if (FromY<FromC)
			m_point_b.y=static_cast<LONG>(last.y);
		// Must be closest to the 45 degree line
		else
			m_point_b.x=static_cast<LONG>(last.x+(m_point_b.x>last.x ? FromY : -FromY));
		break;
	default:
		g_EditToolBar.m_DrawPolyEdit.mode=0;
  }
}


BOOL CDrawPolygon::RButtonDown(CDPoint p, CDPoint s)
{
	// Get the current location of the mouse
	CPoint mp;
	GetCursorPos( &mp );

	// Now bring up the context menu..
	CMenu menu;
	menu.LoadMenu( IDR_POLYGON );
	menu.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN|TPM_RIGHTBUTTON,
		mp.x,mp.y, AfxGetMainWnd(), NULL );
	
	return TRUE;
}

void CDrawPolygon::FindNearestSegment( CDPoint p, int &line, int &handle )
{
	double distance = 1e10;

	int s = 0;
	int h = 0;
	CDPoint dla = m_points.back();
	CDPoint la( static_cast<int>(dla.x), static_cast<int>(dla.y) );
	pointCollection::iterator it = m_points.begin();
	arcpointCollection::iterator itz = m_handles.begin();
	while (it != m_points.end())
	{
		CDPoint a_np = *it + m_point_a;
		CDPoint np(a_np.x, a_np.y);
		CLineUtils l( la, np);
		CDPoint loc_i;
		double distance_i;

		distance_i = l.DistanceFromPoint( p, loc_i );
		if (distance_i <= distance)
		{
			distance = distance_i;
			line = h;
		}

		++ s;

		if (s == (*itz).flatterned_segment)
		{
			++ h;	
			++ itz;
		}

		la = np;
		++ it;
	}

	// Now find the nearest handle..
	distance = 1e10;
	s = 0;
	itz = m_handles.begin();
	while (itz != m_handles.end())
	{
		CArcPoint qp = *itz + m_point_a;
		double distance_i = 
			(qp.x-p.x)*(qp.x-p.x) + (qp.y-p.y)*(qp.y-p.y);
		if (distance_i < distance)
		{
			distance = distance_i;
			handle = s;
		}

		++ s;
		++ itz;
	}
}



// For the context menu
void CDrawPolygon::ContextMenu( CDPoint p, UINT id )
{
	CDPoint snap_p = m_pDesign->m_snap.Snap(p);

	if (!m_segment)
	{
		switch (id)
		{
		case ID_CONTEXT_ARCIN:
			g_EditToolBar.m_DrawPolyEdit.SetArcType( CArcPoint::Arc_in );
			break;
		case ID_CONTEXT_ARCOUT:
			g_EditToolBar.m_DrawPolyEdit.SetArcType( CArcPoint::Arc_out );
			break;
		case ID_CONTEXT_FREELINE:
			g_EditToolBar.m_DrawPolyEdit.SetArcType( CArcPoint::Arc_none );
			break;
		}
	}
	else
	{
		// Find the nearest line m_segment...
		// and locate the nearest handle to it...
		int line;
		int handle;

		Display();
		FindNearestSegment( p, line, handle );

		switch (id)
		{
		case ID_CONTEXT_ARCIN:
			if (line > 0)
			{
				m_handles[ line ].arc = CArcPoint::Arc_in;
			}
			break;
		case ID_CONTEXT_ARCOUT:
			if (line > 0)
			{
				m_handles[ line ].arc = CArcPoint::Arc_out;
			}
			break;
		case ID_CONTEXT_FREELINE:
			if (line > 0)
			{
				m_handles[ line ].arc = CArcPoint::Arc_none;
			}
			break;
		case ID_CONTEXT_DELETEHANDLE:
			if (m_handles.size() > 2)
			{
				m_handles.erase( m_handles.begin() + handle );
			}
			break;
		case ID_CONTEXT_ADDHANDLE:
			m_handles.insert( m_handles.begin() + line, CArcPoint( snap_p - m_point_a ) );
			break;
		}

		CalcBoundingRect();
		Display();
	}
}

int  CDrawPolygon::GetContextMenu()
{
	return IDR_POLYGON_EDIT;
}



// For the context menu
void CDrawPolygon::FinishDrawing( CDPoint no_snap_p )
{
	CDPoint p = m_pDesign->m_snap.Snap(no_snap_p);

	// Ok, we can place
	if (!m_segment)
	{
		// First line
		LButtonDown(p,no_snap_p);
	}
	// Second line..
	LButtonDown(p,no_snap_p);

	// Now Store...
	CDrawingObject *obj = Store();

	// .. and clear out...
	m_handles.erase( m_handles.begin(), m_handles.end() );
	m_points.erase( m_points.begin(), m_points.end() );

	// Now select the top object and switch back
	// to the Edit tool
	m_pDesign->UnSelect();
	m_pDesign->Select( obj );
	m_pDesign->SelectObject( new CDrawEditItem(m_pDesign) );
}



void CDrawPolygon::DblLButtonDown(CDPoint p,CDPoint)
{
	FinishDrawing( p );
}


void CDrawPolygon::LButtonUp(CDPoint p, CDPoint)
{
	// Can't drag out a polygon!
}


void CDrawPolygon::NewOptions()
{
  	Display();
	Style = m_pDesign->GetOptions()->GetCurrentStyle(GetType());
	Fill =  m_pDesign->GetOptions()->GetCurrentFillStyle(GetType());
	Display();
}


CString CDrawPolygon::GetName() const
{
	return "Polygon";
}

int CDrawPolygon::getMenuID() 
{ 
	return IDM_TOOLPOLYGON;
}


void CDrawPolygon::LButtonDown(CDPoint p, CDPoint)
{
  // New undo level for each placement...
  m_pDesign->BeginNewChangeSet();

  if (m_segment) 
  {
	m_point_a=p;
	m_point_b=p;
	m_segment=!m_segment;
	m_handles.push_back( CPoint(0,0) );
  } 
  else 
  {
	m_point_b=p;
	ToAngle();

	// Remove from screen
	Display();
	CArcPoint last = m_point_a;
	if (m_handles.size() > 0)
	{
		last = m_handles.back() + m_point_a;
	}

	CArcPoint c=m_point_b;

	switch (g_EditToolBar.m_DrawPolyEdit.mode) 
	{
		case 1: c=CPoint(static_cast<int>(m_point_b.x),static_cast<int>(last.y));
			break;
		case 2: c=CPoint(static_cast<int>(last.x),static_cast<int>(m_point_b.y));
			break;
	}

	c = c - m_point_a;

	CArcPoint p( c.x, c.y, g_EditToolBar.m_DrawPolyEdit.GetArcType());

	// Only push back if this point is different from
	// the last
	if (m_handles.size() > 0)
	{
		if (p != m_handles.back())
		{
			m_handles.push_back( p );
		}
	}
	else
	{
		m_handles.push_back( p );
	}

	FlatternPath();
	Display();	// Write to screen
	m_segment=0;

	ToAngle();
  }

  Display();
}


void CDrawPolygon::Move(CDPoint p, CDPoint no_snap_p)
{
  if (!m_segment) 
  {
	Display();

	m_point_b=p;
	FlatternPath();
	ToAngle();

	Display();
  }

}





ObjType CDrawPolygon::GetType()
{
  return xPolygon;
}

void CDrawPolygon::Display( BOOL erase )
{
	LineStyle *theStyle = m_pDesign->GetOptions()->GetStyle(Style);
	//int width = max(2,theStyle->Thickness) * 2;

  CDRect r(min(m_point_a.x,m_point_b.x),min(m_point_a.y,m_point_b.y),
	  max(m_point_a.x,m_point_b.x),max(m_point_a.y,m_point_b.y));


	pointCollection::iterator it = m_points.begin();
	while (it != m_points.end())
	{
		CDPoint qp = *it + m_point_a;
		  r.top = min(r.top, qp.y);
		  r.left = min(r.left, qp.x);
		  r.bottom = max(r.bottom, qp.y);
		  r.right = max(r.right, qp.x);

		++ it;
	}

	m_pDesign->InvalidateRect( r, erase, OUTLINE_SPACER * 8 );
}

void CDrawPolygon::PaintHandles( CContext&dc )
{
	// Put some handles around this object
	arcpointCollection::iterator it = m_handles.begin();
	dc.SelectBrush(cBLACK);
	dc.SelectPen(PS_SOLID,0,cBLACK);

	while (it != m_handles.end())
	{
		CArcPoint p = *it + m_point_a;
		CDRect r(p.x-2,p.y-2,p.x+2,p.y+2);
		dc.Rectangle(r);

		++ it;
	}

	// Now draw the tracker for resizing...
	CalcBoundingRect();
	CDRect r(m_point_a.x - OUTLINE_SPACER,m_point_a.y - OUTLINE_SPACER,
		m_point_b.x + OUTLINE_SPACER,m_point_b.y + OUTLINE_SPACER);

	dc.PaintTracker( r );
}


// Move fields of this object about
int CDrawPolygon::IsInsideField(CDPoint p)
{
	// Check the handles around this object
	int s = 0;
	arcpointCollection::iterator it = m_handles.begin();
	while (it != m_handles.end())
	{
		CArcPoint qp = *it + m_point_a;
		CDRect r(qp.x-2,qp.y-2,qp.x+2,qp.y+2);
		if (r.PtInRect(p))
		{
			return s + 1000;
		}

		++ s;
		++ it;
	}

	// Now is it inside our tracker?
	CRect r(static_cast<int>(m_point_a.x - OUTLINE_SPACER),
		static_cast<int>(m_point_a.y - OUTLINE_SPACER),
		static_cast<int>(m_point_b.x + OUTLINE_SPACER),
		static_cast<int>(m_point_b.y + OUTLINE_SPACER));

	CRectTracker tracker( r, CRectTracker::dottedLine | CRectTracker::resizeOutside  );
	s = tracker.HitTest( CPoint( static_cast<int>(p.x), static_cast<int>(p.y)) );

//	CPoint q(m_pDesign->GetTransform().Scale(p));
//	CRect rect(m_pDesign->GetTransform().Scale( CDRect(m_point_a.x,m_point_a.y,m_point_b.x,m_point_b.y) ));
//	rect.NormalizeRect();
//	CRectTracker	tracker( rect, CRectTracker::dottedLine | CRectTracker::resizeOutside  );
//	s = tracker.HitTest( q );


	if (s == 8)
	{
		s = 11;
	}

	return s;
}

void CDrawPolygon::MoveField(int w, CDPoint r)
{
	Display();

	if (w >= 1000)
	{
		// Individual handle movement...
		m_handles[w - 1000] = m_handles[w - 1000] + r;
	}
	else
	{
		CDRect rect( m_point_a.x, m_point_a.y, m_point_b.x, m_point_b.y );

		// Tracker movement...
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
		case CRectTracker::hitMiddle:
		case 11:
			rect += r;
			break;
		}

		// Determine scaling in the x and y
		CPoint tr;
		double scaling_x = 1.0;
		double scaling_y = 1.0;

		scaling_x = fabs(rect.Width()) / fabs( m_point_a.x - m_point_b.x );
		scaling_y = fabs(rect.Height()) / fabs( m_point_a.y - m_point_b.y );

		if (!_finite( scaling_x ))
		{
			scaling_x = 1.0;
		}
		if (!_finite( scaling_y ))
		{
			scaling_y = 1.0;
		}


		m_point_a.x = rect.left;
		m_point_a.y = rect.top;
		m_point_b.x = rect.right;
		m_point_b.y = rect.bottom;

		// Now apply the scaling...
		arcpointCollection::iterator it = m_handles.begin();
		while (it != m_handles.end())
		{
			CArcPoint p = *it;
			p.x *= scaling_x;
			p.y *= scaling_y;
			*it = p;
			++ it;
		}

	}

	FlatternPath();
	Display();
}

int CDrawPolygon::SetCursorEdit( CDPoint p )
{
	CDRect r(m_point_a.x,m_point_a.y,m_point_b.x,m_point_b.y);
	r.NormalizeRect();

	int s = -1;
	
	s = IsInsideField(p);
	if (s >= 1000)
	{
		// Always display the same cursor
		s = 8;
	}

	if (s == -1 && IsInsidePolygon(p))
	{
		s = 11;
	}


	return s;
}


// Try and find out if the point is inside the polygon.
//
// We use the odd/even rule.  We draw an imaginary line
// in any direction and count the number of lines we
// cross, if it is odd then we are inside the polygon,
// if even then it is outside.
//
// We can choose the line direction, so we pick a
// horizontal line.  We determine the line crossing
// by converting the lines to a parametric version:
// x = gy + h
// l1.x - g * l1.y = h
// Then determine if x is lies within the line at the crossing
// point, by comparing it with the line ends.
//
BOOL CDrawPolygon::IsInsidePolygon( CDPoint p )
{
	int s = 0;
	pointCollection::iterator it = m_points.begin();
	
	CDPoint l1,l2;
	int count = 0;

	// Pre-load with the last point, so
	// we check the full round polygon...
	l2 = m_points.back() + m_point_a;

	for (;it != m_points.end(); ++it, ++s)
	{
		// Get the line
		l1 = l2;
		l2 = *it + m_point_a;

		// We can ignore horizontal lines
		if (l1.y == l2.y)
		{
			continue;
		}

		// Is the line vertical?
		double x_intersect;
		if (l1.x == l2.x)
		{
			// Perform quick calculation
			if (   p.y < min(l1.y,l2.y) 
				|| p.y > max(l1.y,l2.y) )
			{
				continue;
			}
			x_intersect = l1.x;
		}
		else
		{
			// Now convert the line to a parametric
			// equation...
			double g = static_cast<double>(l2.x - l1.x) / static_cast<double>(l2.y - l1.y);
			double h = l2.x - g * l2.y;

			// Now find out where this intersects our
			// y co-ordinate
			// Note: we can have problems if the result lies
			// directly on a vertex.  To compensate we add 0.5 to the
			// test position.  As all vertex's y position are all ways
			// integer we can never lie on a vertex.
			x_intersect = g * (p.y + 0.5) + h;
		}

		// Is this inside the line?
		if (	x_intersect < min(l1.x,l2.x) 
			||	x_intersect > max(l1.x,l2.x))
		{
			// No, so continue
			continue;
		}

		// Is this in the positive direction?
		if (x_intersect > p.x)
		{
			// Yes, so count it!
			count ++;
		}
	}

	return (count & 1) != 0;
}

void CDrawPolygon::AddPolyBezier( pointCollection &cp, CArcPoint p1, CArcPoint p2 )
{
 	double x, y;
  	double t, t2, t3, a, b, c;

	CDPoint Np[4];
	Np[0] = CDPoint(p1.x,p1.y);
	Np[3] = CDPoint(p2.x,p2.y);
	double mid_x = (Np[0].x + Np[3].x)/2.0;
	double mid_y = (Np[0].y + Np[3].y)/2.0;

	switch (p2.arc)
	{
	case CArcPoint::Arc_out:
		Np[1].x = Np[0].x;
		Np[1].y = mid_y;
		Np[2].x = mid_x;
		Np[2].y = Np[3].y;
		break;
	default:
		Np[1].x = mid_x;
		Np[1].y = Np[0].y;
		Np[2].x = Np[3].x;
		Np[2].y = mid_y;
		break;
	}

	cp.push_back(Np[0]);

  	for (t=0.0; t<=1.0; t += 0.05) 
	{
   		t2 = t * t;
   		t3 = t2 * t;
   		a = 1 - 3*t + 3*t2 - t3;
   		b = 3*(t - 2*t2 + t3);
   		c = 3*(t2 - t3);
   		x = a * Np[0].x
       		+ b * Np[1].x
       		+ c * Np[2].x
       		+ t3 * Np[3].x;
   		y = a * Np[0].y
       		+ b * Np[1].y
       		+ c * Np[2].y
       		+ t3 * Np[3].y;

		cp.push_back(CDPoint(x, y));
  	}


	cp.push_back(Np[3]);
}



void CDrawPolygon::FlatternPath()
{
	m_points.erase( m_points.begin(), m_points.end() );

	// Now inspect the handles and create a flat line
	// version
	arcpointCollection::iterator it = m_handles.begin();
	CArcPoint last;
	while (it != m_handles.end())
	{
		if ((*it).arc != CArcPoint::Arc_none)
		{
			AddPolyBezier( m_points, last, *it );
		}
		else
		{
			
			m_points.push_back( CDPoint( (*it).x, (*it).y ) );
		}

		(*it).flatterned_segment = m_points.size();

		last = *it;
		++ it;
	}
}


// Display the line on the screen!
void CDrawPolygon::Paint(CContext&dc,paint_options options)
{
	dc.SelectPen(m_pDesign->GetOptions()->GetStyle(Style), options);
	dc.SetROP2(R2_COPYPEN);

	if (Fill == fsNONE || !m_segment || options == draw_selectable)
	{
		//Add last line segment to selectable outline.
		//It is missing if the polygon is not closed.
		if (Fill != fsNONE && options == draw_selectable && m_points.size() > 2) {
			pointCollection outline( m_points );
			outline.push_back( m_points.front() );
			dc.Polyline( outline, m_point_a, NULL );
		}
		else {
			dc.Polyline( m_points, m_point_a, NULL );
		}

		pointCollection cp;
		if (!m_segment) 
		{
			CArcPoint p1;
			if (m_points.size() > 0)
			{
				CDPoint bq = m_points.back();
				p1 = CArcPoint( bq.x, bq.y );
			}

			switch (g_EditToolBar.m_DrawPolyEdit.mode) 
			{
				case 1: 
					cp.push_back( CDPoint( p1.x, p1.y ) );
					p1 = CArcPoint(m_point_b.x - m_point_a.x,p1.y);
					break;
				case 2:	
					cp.push_back( CDPoint( p1.x, p1.y ) );
					p1 = CArcPoint(p1.x,m_point_b.y - m_point_a.y);
					break;
			}

			CArcPoint p2(m_point_b.x - m_point_a.x, m_point_b.y - m_point_a.y,g_EditToolBar.m_DrawPolyEdit.GetArcType());
			if (p2.arc != CArcPoint::Arc_none)
			{
				AddPolyBezier( cp, p1, p2 );
			}
			else
			{
				cp.push_back(CDPoint(p1.x,p1.y));
				cp.push_back(CDPoint(p2.x,p2.y));
			}
			dc.Polyline( cp, m_point_a, NULL );
		}

	}
	else
	{
		dc.Polyline( m_points, m_point_a, m_pDesign->GetOptions()->GetFillStyle(Fill) );
	}
}


// Store the line in the drawing
CDrawingObject* CDrawPolygon::Store()
{
  CDrawPolygon *NewObject;

  NewObject = new CDrawPolygon(m_pDesign);
  // Copy the details
  m_segment=1;
  *NewObject = *this;

  m_pDesign->Add(NewObject);

  return NewObject;
}


void CDrawPolygon::BeginEdit(BOOL re_edit)
{
  m_segment=1;

  m_pDesign->GetOptions()->SetCurrentStyle(GetType(), Style);
  m_pDesign->GetOptions()->SetCurrentFillStyle(GetType(), Fill);


  m_re_edit = re_edit;
  if (re_edit)
  {
	  g_EditToolBar.m_PolygonEdit.Open(m_pDesign,this);
  }
  else
  {
	  g_EditToolBar.m_DrawPolyEdit.Open(m_pDesign,this);
	  g_EditToolBar.m_DrawPolyEdit.mode=0;
  }
}

void CDrawPolygon::EndEdit()
{
  Display();
  if (m_re_edit)
  {
	g_EditToolBar.m_PolygonEdit.Close();
  }
  else
  {
	  g_EditToolBar.m_DrawPolyEdit.Close();
  }

  m_re_edit = FALSE;
}


int CDrawPolygon::IsInsideLine(double left,double right,double top,double bottom)
{
	int s = 0;
	CDPoint la;
	pointCollection::iterator it = m_points.begin();
	while (it != m_points.end())
	{
		CDPoint a_np = *it + m_point_a;
		CDPoint np(a_np.x, a_np.y);
		if (s != 0)
		{
			CLineUtils l( la, np);
			BOOL r = l.IsInside(left,right,top,bottom);
			if (r)
			{
			  return TRUE;
			}
		}

		la = np;
		++ s;
		++ it;
	}

	return FALSE;
}


double CDrawPolygon::DistanceFromPoint( CDPoint p )
{
	// Use fast cut-off to see if the bounding box is inside the intersection box
	// Use somewhat enlarged bounding box to allow DistanceFromPoint from just outside the bounding box
	if ( ((m_point_a.x<p.x-10 && m_point_b.x<p.x-10) || (m_point_a.x>p.x+10 && m_point_b.x>p.x+10)
	   || (m_point_a.y<p.y-10 && m_point_b.y<p.y-10) || (m_point_a.y>p.y+10 && m_point_b.y>p.y+10))) 
	{
		return 100.0;
	}

	// There is a fill, so just use the IsInsidePolygon routine...
	if (Fill != fsNONE)
	{
		if (IsInsidePolygon(p))
		{
			return 0.0;
		}
	}

	// Ok, so check for distance from one of our lines...
	double closest_distance = 100.0;
	int s = 0;
	CDPoint la;
	pointCollection::iterator it = m_points.begin();
	while (it != m_points.end())
	{
		CDPoint a_np = *it + m_point_a;
		CDPoint np(a_np.x, a_np.y);
		if (s != 0)
		{
			CLineUtils l( la, np);
			CDPoint d;
			closest_distance = min( closest_distance, l.DistanceFromPoint( p, d ) );
		}

		la = np;
		++ s;
		++ it;
	}

	//For filled polygons also check for distance to closing line segment
	if (Fill != fsNONE && m_points.size() > 2) {
		CDPoint np(m_points.front() + m_point_a);
		CLineUtils l( la, np);
		CDPoint d;
		closest_distance = min( closest_distance, l.DistanceFromPoint( p, d ) );
	}

	LineStyle *theStyle = m_pDesign->GetOptions()->GetStyle(Style);
	double width = min(0, theStyle->Thickness);// + (10 / (m_pDesign->GetTransform().GetZoomFactor()));
	return closest_distance - width;

	// On the polygon?
	//if (closest_distance <= width)
	//{
	//	return closest_distance - width;
	//}

	//return closest_distance;
}



BOOL CDrawPolygon::IsInside(double left,double right,double top,double bottom)
{
	// Use fast cut-off to see if the bounding box is inside the intersection box
	if ( (m_point_a.x<left && m_point_b.x<=left) || (m_point_a.x>right && m_point_b.x>=right)
      || (m_point_a.y<top && m_point_b.y<=top) || (m_point_a.y>bottom && m_point_b.y>=bottom) )
	{
		return FALSE;
	}

	// IsInside for point
	if (Fill == fsNONE && left==right && top==bottom)
	{
		// Inside a filled polygon?
		if (m_points.size() > 2 && m_points.front().Distance(m_points.back()) < 0.5 && 
			IsInsidePolygon(CDPoint(left,top)))
		{
			return TRUE;
		}

		LineStyle *theStyle = m_pDesign->GetOptions()->GetStyle(Style);
		double width = min(0, theStyle->Thickness) + (10 / (m_pDesign->GetTransform().GetZoomFactor()));
		if (DistanceFromPoint(CDPoint(left, top)) < width)
		{
			return TRUE;
		}
		return FALSE;
	}

	// We are definately inside if we intersect a line...
	if (IsInsideLine( left,right,top,bottom ))
	{
		return TRUE;
	}

	// If we are filled or editing then check to see 
	// if any of the four points are inside our polygon
	if (Fill != fsNONE || m_re_edit || (left==right && top==bottom))
	{
		return IsInsidePolygon( CDPoint(left,top) )
		|| IsInsidePolygon( CDPoint(right,top) )
		|| IsInsidePolygon( CDPoint(right,bottom) )
		|| IsInsidePolygon( CDPoint(left,bottom) );
	}

	return FALSE;
}



BOOL CDrawPolygon::IsEmpty()
{
	if (m_handles.size() > 1)
	{
		return FALSE;
	}

	return TRUE;
}
