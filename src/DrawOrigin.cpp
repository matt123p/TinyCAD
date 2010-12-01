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



////// The Origin Class //////


CDrawOrigin::CDrawOrigin(CTinyCadDoc *pDesign)
: CDrawingObject( pDesign )
{
	m_point_a=m_point_b=CDPoint();
	m_segment=1;
	is_stuck = FALSE;
}

BOOL CDrawOrigin::IsConstruction()
{
	return TRUE;
}

const TCHAR* CDrawOrigin::GetXMLTag()
{
	return _T("ORIGIN");
}

// Load and save to an XML file
void CDrawOrigin::SaveXML( CXMLWriter &xml )
{
	if (m_point_a.hasValue())
	{
		xml.addTag(GetXMLTag());
		xml.addAttribute( _T("pos"), CDPoint(m_point_a) );
		xml.closeTag();
	}
}

void CDrawOrigin::LoadXML( CXMLReader &xml )
{
	xml.getAttribute( _T("pos"), m_point_a );
	m_point_b = m_point_a;
}



void CDrawOrigin::Load(CStream &archive)
{
	m_point_a = ReadPoint(archive);
	m_point_b=m_point_a;
	m_segment=0;
}


double CDrawOrigin::DistanceFromPoint( CDPoint p )
{
	if (m_point_a.hasValue())
	{
		double distance = p.Distance(m_point_a);
		if (distance < CONNECT_SIZE)
		{
			return 0.0;
		}
		return distance;
	}

	return 1E5;
}


BOOL CDrawOrigin::IsInside(double left,double right,double top,double bottom)
{
	return (m_point_a.x>=left && m_point_a.x<=right && m_point_a.y>=top && m_point_a.y<=bottom);
}

int CDrawOrigin::SetCursorEdit( CDPoint p )
{
	return DistanceFromPoint( p )<10.0 ? 11 : -1;
}

ObjType CDrawOrigin::GetType()
{
	return xOrigin;
}

CString CDrawOrigin::GetName() const
{
	return "Origin";
}

void CDrawOrigin::Move(CDPoint p, CDPoint no_snap_p)
{
	Display();
	BOOL is_junction;
	p = m_pDesign->GetStickyPoint(no_snap_p,TRUE,FALSE,is_stuck,is_junction);
	m_point_a=p;
	m_point_b=p;
	m_segment=0;
	Display();
}

void CDrawOrigin::Shift( CDPoint r )
{
	CDrawingObject::Shift( r );
	if (m_point_a.hasValue())
	{
		m_pDesign->GetOptions()->SetOrigin(m_point_a);
	}
}


void CDrawOrigin::LButtonDown(CDPoint p, CDPoint no_snap_p)
{
	// New undo level for each placement...
	m_pDesign->BeginNewChangeSet();

	Display();
	BOOL is_junction;
	p = m_pDesign->GetStickyPoint(no_snap_p,TRUE,FALSE,is_stuck,is_junction);
	m_point_a=p;
	m_point_b=p;

	// look for existing origin object.
	// When not found then store a new one
	// When found then overwrite the old one
	CDrawingObject *origin = NULL;
	for( drawingIterator it = m_pDesign->GetDrawingBegin(); it != m_pDesign->GetDrawingEnd(); it++ ) 
	{
		CDrawingObject *obj = *it;
		if (obj->GetType() == xOrigin && obj->m_point_a.hasValue())
		{
			origin = obj;
		}		
	}
	if (origin == NULL)
	{
		// Store new (singleton) origin object
		Store();
	}
	else
	{
		// Overwrite exiting origin object
		m_pDesign->MarkChangeForUndo(origin);
		*origin = *this;
	}

	if (m_point_a.hasValue())
	{
		// Set new design origin position
		m_pDesign->GetOptions()->SetOrigin(m_point_a);
	}

	Display();	// Write to screen
}

void CDrawOrigin::Display( BOOL erase )
{
	// Invalidate the rulers
	m_pDesign->InvalidateRulers();
	// Invalidate the symbol
	m_pDesign->Invalidate();
}



void CDrawOrigin::Paint(CContext &dc,paint_options options)
{
	switch (options)
	{
		case draw_selected:
			dc.SelectPen(PS_SOLID,1,cRULER_CLK);
			break;
		case draw_selectable:
			dc.SelectPen(PS_SOLID,1,cPIN_CLK);
			break;
		default:
			dc.SelectPen(PS_SOLID,1,cHOT_SPOT);
	}
	  
	const int size = 6;
	const int circleSize = 4;

	dc.SelectBrush();

	dc.SetROP2(R2_COPYPEN);
	if (options==draw_selected)
	{
		CDPoint b = CDPoint( m_pDesign->GetDetails().GetPageBoundsAsPoint().x + 150, m_point_a.y );
		CDPoint a = CDPoint(dc.GetTransform().DeScale( CPoint(0,0) ).x, m_point_a.y );
		dc.MoveTo( a );
		dc.LineTo( b );	

		a = CDPoint( m_point_a.x, dc.GetTransform().DeScale( CPoint(0,RULER_WIDTH) ).y );
		b = CDPoint( m_point_a.x, m_pDesign->GetDetails().GetPageBoundsAsPoint().y + 150 );
		dc.MoveTo( a );
		dc.LineTo( b );	

		dc.SelectPen(PS_SOLID,1,cSELECT);
		dc.Ellipse1(CDRect(m_point_a.x-circleSize, m_point_a.y-circleSize, m_point_a.x+circleSize, m_point_a.y+circleSize));
	}
	else
	{
		dc.MoveTo(CDPoint(m_point_a.x-size, m_point_a.y));
		dc.LineTo(CDPoint(m_point_a.x+size, m_point_a.y));
		dc.MoveTo(CDPoint(m_point_a.x, m_point_a.y-size));
		dc.LineTo(CDPoint(m_point_a.x, m_point_a.y+size));
		//dc.Ellipse1(CDRect(m_point_a.x-circleSize, m_point_a.y-circleSize, m_point_a.x+circleSize, m_point_a.y+circleSize));
	}
}


// Store the origin in the drawing
CDrawingObject* CDrawOrigin::Store()
{
  CDrawOrigin *NewObject;

  is_stuck = FALSE;

  NewObject = new CDrawOrigin(m_pDesign);

  *NewObject = *this;

  m_pDesign->Add(NewObject);

  return NewObject;
}

void CDrawOrigin::NotifyEdit(int action)
{
	switch(static_cast<CDocUndoSet::action>(action))
	{
	case CDocUndoSet::Addition:
		if (m_point_a.hasValue())
		{
			// Origin is only valid in a library symbol
			if (!m_pDesign->IsEditLibrary())
			{
				// Set this origin point to 'nothing'			
				m_point_a = CDPoint();
			}
			else
			{
				// The origin must behave like a singleton
				// So invalidate all other occurences of the origin object
				// by setting their origin point (m_point_a) to 'nothing'
				CDrawingObject *origin = NULL;
				for( drawingIterator it = m_pDesign->GetDrawingBegin(); it != m_pDesign->GetDrawingEnd(); it++ ) 
				{
					CDrawingObject *obj = *it;
					if (obj != this && obj->GetType() == xOrigin && obj->m_point_a.hasValue())
					{
						// Set this origin point to 'nothing'			
						m_point_a = CDPoint();
						break;
					}
				}

				if (m_point_a.hasValue())
				{
					// Set new origin position
					m_pDesign->GetOptions()->SetOrigin(m_point_a);
					m_pDesign->InvalidateRulers();
				}
			}
		}
		break;
	case CDocUndoSet::Deletion:
		{
			// Restore the previous origin position
			// by looking for a valid origin object
			CDPoint origin;
			for( drawingIterator i = m_pDesign->GetDrawingBegin(); i != m_pDesign->GetDrawingEnd(); i++ ) 
			{
				CDrawingObject *obj = *i;
				if (obj != this && obj->GetType() == xOrigin && obj->m_point_a.hasValue())
				{
					origin = obj->m_point_a;
					break;
				}		
			}

			m_pDesign->GetOptions()->SetOrigin(origin);
			m_pDesign->InvalidateRulers();
		}
		break;
	case CDocUndoSet::Change:
		{
			// Search for the single valid origin object
			CDPoint origin;
			for( drawingIterator i = m_pDesign->GetDrawingBegin(); i != m_pDesign->GetDrawingEnd(); i++ ) 
			{
				CDrawingObject *obj = *i;
				if (obj->GetType() == xOrigin && obj->m_point_a.hasValue())
				{
					origin = obj->m_point_a;
					break;
				}		
			}

			m_pDesign->GetOptions()->SetOrigin(origin);
			m_pDesign->InvalidateRulers();
		}
		break;
	}
}


