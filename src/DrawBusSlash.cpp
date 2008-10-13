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



// This handles the bus objects

#include "stdafx.h"
#include "TinyCadView.h"
#include "MainFrm.h"
#include "diag.h"
#include "colour.h"
#include "option.h"
#include "LineUtils.h"


////// The BusSlash Class //////

#define SLASH_SIZE 20

void CDrawBusSlash::Load(CStream &archive)
{
  m_point_a = ReadPoint(archive);
  archive >> theDir;
  m_point_b=m_point_a;
  m_segment=0;
}

const TCHAR* CDrawBusSlash::GetXMLTag()
{
	return _T("BUSSLASH");
}

// Load and save to an XML file
void CDrawBusSlash::SaveXML( CXMLWriter &xml )
{
	xml.addTag(GetXMLTag());
	xml.addAttribute( _T("pos"), CDPoint(m_point_a) );
	xml.addAttribute( _T("direction"), theDir );
	xml.closeTag();
}

void CDrawBusSlash::LoadXML( CXMLReader &xml )
{
	xml.getAttribute( _T("pos"), m_point_a );
	xml.getAttribute( _T("direction"), theDir );

	m_point_b = m_point_a;
}



CDrawBusSlash::CDrawBusSlash(CTinyCadDoc *pDesign, int NewDir)
: CDrawingObject( pDesign )
{
	theDir = NewDir;
	m_point_a=m_point_b=CDPoint(0,0);
	m_segment=1;
}

BOOL CDrawBusSlash::IsInside(double left,double right,double top,double bottom)
{
	CLineUtils l(m_point_a,m_point_b);
	return l.IsInside(left,right,top,bottom);
}

double CDrawBusSlash::DistanceFromPoint( CDPoint p )
{
	CLineUtils l(m_point_a,m_point_b);
	CDPoint d;
	return l.DistanceFromPoint(p, d );
}


ObjType CDrawBusSlash::GetType()
{
  return xBusSlash;
}

int CDrawBusSlash::getMenuID() 
{ 
	switch (theDir)
	{
	case 0:
		return IDM_BUSSLASH; 
	default:
		return IDM_BUSBACK;
	}
	
}


CString CDrawBusSlash::GetName() const
{
  return "Bus Entry";
}

void CDrawBusSlash::Move(CDPoint p, CDPoint no_snap_p)
{
  Display();
  m_point_a=p;
  m_segment=0;
  Display();
}

void CDrawBusSlash::LButtonDown(CDPoint p, CDPoint)
{
  // New undo level for each placement...
  m_pDesign->BeginNewChangeSet();

  Display();
  m_point_a=p;
  m_point_b=p;
  Store();
  Display();	// Write to screen
}


void CDrawBusSlash::Display( BOOL erase )
{
	// Invalidate the symbol
	CDRect r( m_point_a.x,m_point_a.y,m_point_b.x,m_point_b.y);
	m_pDesign->InvalidateRect( r, erase, 2 );
}


void CDrawBusSlash::Paint(CContext &dc,paint_options options)
{
  CPoint bl;
  switch (options)
  {
  case draw_selected:
	  dc.SelectPen(PS_SOLID,1,cSELECT);
	  break;
  case draw_selectable:
	  dc.SelectPen(PS_SOLID,1,cPIN_CLK);
	  break;
  default:
	  dc.SelectPen(PS_SOLID,1,m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::PIN ) );
  }

  

	dc.SetROP2(R2_COPYPEN);

  if (theDir == 0)
	m_point_b=CDPoint(m_point_a.x-SLASH_SIZE,m_point_a.y+SLASH_SIZE);
  else
	m_point_b=CDPoint(m_point_a.x-SLASH_SIZE,m_point_a.y-SLASH_SIZE);

  dc.MoveTo(m_point_b);
  dc.LineTo(m_point_a);

}


// Store the slash in the drawing
CDrawingObject* CDrawBusSlash::Store()
{
  CDrawBusSlash *NewObject;

  NewObject = new CDrawBusSlash(m_pDesign);
  *NewObject = *this;

  m_pDesign->Add(NewObject);

  return NewObject;
}



// Extract the netlist/active points from this object
void CDrawBusSlash::GetActiveListFirst( CActiveNode &a )
{
}

bool CDrawBusSlash::GetActive( CActiveNode &a )
{
	switch (a.m_sent)
	{
	case 0:
		a.m_a = m_point_a;
		break;
	case 1:
		a.m_a = m_point_b;
		break;
	default:
		return false;
	}


	++ a.m_sent;
	return true;
}

