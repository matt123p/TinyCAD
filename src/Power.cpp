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
#include <stdlib.h>
#include "TinyCadView.h"
#include "diag.h"
#include "colour.h"
#include "option.h"
#include "LineUtils.h"

// The Power Class //

#define POWER_SIZE	14



void CDrawPower::TagResources()
{
 	m_pDesign->GetOptions()->TagFont(fPIN);
}


int CDrawPower::DoRotate(int olddir,int newdir)
{
  //     New rotation=> 2  3  4  
  //				    Current dir ..\/..
  int table[] = {	2, 3, 0, 		// 0 (Up)
			3, 2, 1,		// 1 (Down)
			1, 0, 3,		// 2 (Left)
			0, 1, 2,		// 3 (Right)
		};

  return table[(newdir-2) + olddir*3];
}

const TCHAR* CDrawPower::GetXMLTag()
{
	return _T("POWER");
}

// Load and save to an XML file
void CDrawPower::SaveXML( CXMLWriter &xml )
{
	xml.addTag(GetXMLTag());

	xml.addAttribute( _T("pos"), CDPoint(m_point_a) );
	xml.addAttribute( _T("which"), which );
	xml.addAttribute( _T("direction"), dir );
	xml.addChildData( str );

	xml.closeTag();
}

void CDrawPower::LoadXML( CXMLReader &xml )
{
	xml.getAttribute( _T("pos"), m_point_a );
	xml.getAttribute( _T("which"), which );
	xml.getAttribute( _T("direction"), dir );
	xml.getChildData( str );

	m_point_b = m_point_a;
	TextPos = m_point_a;
}


void CDrawPower::Load(CStream &archive)
{
 	m_point_a = ReadPoint(archive);
  archive >> which >> dir >> str;
  m_point_b=m_point_a;
  m_segment=0;
}

double CDrawPower::DistanceFromPoint( CDPoint p )
{
	CDPoint pa;

	if (p.x > TextPos.x && p.x < TextEnd.x && p.y > TextEnd.y && p.y < TextPos.y)
	{
		return 0.0;
	}

  // Find out which way round the power object goes
  switch (dir) {
	case 0:		// Top
		pa=CDPoint(m_point_a.x,m_point_a.y-POWER_SIZE);
		break;
	case 1:		// Bottom
		pa=CDPoint(m_point_a.x,m_point_a.y+POWER_SIZE);
		break;
	case 2:		// Left
		pa=CDPoint(m_point_a.x-POWER_SIZE,m_point_a.y);
		break;
	case 3:		// Right
		pa=CDPoint(m_point_a.x+POWER_SIZE,m_point_a.y);
		break;
  }

  CLineUtils l(m_point_a,pa);
  CDPoint d;
  return l.DistanceFromPoint(p, d );
}



BOOL CDrawPower::IsInside(double left,double right,double top,double bottom)
{
	CDPoint pa;

  // IsInside for point
  if (left==right && top==bottom)
  {
	  double d = DistanceFromPoint(CDPoint(left, top));
	  double range = (10 / (m_pDesign->GetTransform().GetZoomFactor()));
	  return (d < range) ? TRUE : FALSE;
  }

  // Find out which way round the power object goes
  switch (dir) {
	case 0:		// Top
		pa=CDPoint(m_point_a.x,m_point_a.y-POWER_SIZE);
		break;
	case 1:		// Bottom
		pa=CDPoint(m_point_a.x,m_point_a.y+POWER_SIZE);
		break;
	case 2:		// Left
		pa=CDPoint(m_point_a.x-POWER_SIZE,m_point_a.y);
		break;
	case 3:		// Right
		pa=CDPoint(m_point_a.x+POWER_SIZE,m_point_a.y);
		break;
  }

  CLineUtils l(m_point_a,pa);

  return !( (TextPos.x<left && TextEnd.x<=left) || (TextPos.x>right  && TextEnd.x>=right)
         || (TextPos.y<top  && TextEnd.y<=top)  || (TextPos.y>bottom && TextEnd.y>=bottom) )
         || l.IsInside(left,right,top,bottom);
}

// Look for a seach string in the object
CString CDrawPower::Find(const TCHAR *theSearchString)
{
  CString HoldString;

  HoldString = str;
  HoldString.MakeLower();

  // Now look for the search string in this string
  return (HoldString.Find(theSearchString) != -1) ? str : "";
}


// Rotate this object about a point
void CDrawPower::Rotate(CDPoint p,int ndir)
{			 
	// Rotate bounding box only if we have a centre point
	if (p != CDPoint(0, 0))
	{
		// Translate this point so the rotational point is the origin
		m_point_a = CDPoint(m_point_a.x-p.x,m_point_a.y-p.y);
		m_point_b = CDPoint(m_point_b.x-p.x,m_point_b.y-p.y);

		// Perfrom the rotation
		switch (ndir) {
		case 2: // Left
			m_point_a = CDPoint(m_point_a.y,-m_point_a.x);
			m_point_b = CDPoint(m_point_b.y,-m_point_b.x);
			break;		
		case 3: // Right
			m_point_a = CDPoint(-m_point_a.y,m_point_a.x);
			m_point_b = CDPoint(-m_point_b.y,m_point_b.x);
			break;
		case 4: // Mirror
			m_point_a = CDPoint(-m_point_a.x,m_point_a.y);
			m_point_b = CDPoint(-m_point_b.x,m_point_b.y);
			break;
		}

		// Re-translate the points back to the original location
		m_point_a = CDPoint(m_point_a.x+p.x,m_point_a.y+p.y);
		m_point_b = CDPoint(m_point_b.x+p.x,m_point_b.y+p.y);
	}
	
	dir = DoRotate(dir,ndir);

	CalcLayout();
}


BOOL CDrawPower::CanEdit()
{
  return TRUE;
}

ObjType CDrawPower::GetType()
{
  return xPower;
}

CString CDrawPower::GetName() const
{
  return "Power";
}


CDrawPower::CDrawPower(CTinyCadDoc *pDesign)
: CDrawingObject( pDesign )
{
  m_segment=1;
  m_point_a=CDPoint(0,0);
  m_point_b=m_point_a;
  str="";
  dir=1;
  which=0;
  is_stuck = FALSE;
  is_junction = FALSE;
}

void CDrawPower::BeginEdit(BOOL re_edit)
{
  g_EditToolBar.m_PowerEdit.Open(m_pDesign,this);
}

void CDrawPower::Move(CDPoint p, CDPoint no_snap_p)
{
  Display();
  p = m_pDesign->GetStickyPoint(no_snap_p, TRUE, TRUE, is_stuck, is_junction);
  m_point_a=p;
  m_point_b=m_point_a;
  m_segment=0;
  Display();
}

void CDrawPower::LButtonDown(CDPoint p, CDPoint no_snap_p)
{
	// New undo level for each placement...
	m_pDesign->BeginNewChangeSet();

	Display();
	p = m_pDesign->GetStickyPoint(no_snap_p, TRUE, TRUE, is_stuck, is_junction);
	m_point_a=p;
	Store();
	g_EditToolBar.m_PowerEdit.ReFocus();
	Display();	// Write to screen
}


void CDrawPower::Display( BOOL erase )
{
	// Invalidate the symbol
	CDRect r( m_point_a.x,m_point_a.y,m_point_b.x,m_point_b.y);
	m_pDesign->InvalidateRect( r, erase, POWER_SIZE * 2 + 2 );

	r.left = TextPos.x;
	r.top = TextPos.y;
	r.right = TextEnd.x;
	r.bottom = TextEnd.y;
	m_pDesign->InvalidateRect( r, erase, 5 );

 if (is_stuck)
 {
	 r.left = m_point_b.x - HIGHLIGHT_SIZE;
	 r.right = m_point_b.x + HIGHLIGHT_SIZE;
	 r.top = m_point_b.y - HIGHLIGHT_SIZE;
	 r.bottom = m_point_b.y + HIGHLIGHT_SIZE;
	 m_pDesign->InvalidateRect( r, erase, 3 );
 }

}

void CDrawPower::CalcLayout()
{
	int spacing;
	CDSize size;
	CDPoint pos=m_point_a;

	size = m_pDesign->GetTextExtent(str, fPIN);
	if (which!=0)
	{
		spacing = POWER_SIZE * 2 + POWER_SIZE / 4;
	}
	else
	{
		spacing = POWER_SIZE + POWER_SIZE / 4;
	}

	// Find out which way round the power object goes
	switch (dir) {
	case 0:		// Top
		TextPos = CDPoint(pos.x-size.cx/2,pos.y-spacing);
		break;
	case 1:		// Bottom
		TextPos = CDPoint(pos.x-size.cx/2,pos.y+spacing+size.cy);
		break;
	case 2:		// Left
		TextPos=CDPoint(pos.x-size.cx-spacing,pos.y+size.cy/2);
		break;
	case 3:		// Right
		TextPos=CDPoint(pos.x+spacing,pos.y+size.cy/2);
		break;
	}

	TextEnd = CDPoint(TextPos.x + size.cx,TextPos.y - size.cy);

}




// Display the power item on the screen!
void CDrawPower::Paint(CContext &dc,paint_options options)
{
  int spacing;
  CDPoint pa,pb,pc,pd,pos=m_point_a;

  CalcLayout();

  dc.SelectFont(*m_pDesign->GetOptions()->GetFont(fPIN),2);
  dc.SetROP2(R2_COPYPEN);
  dc.SetTextColor(m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::POWER));

  if (which!=0)
	spacing=POWER_SIZE*2+POWER_SIZE/4;
  else
	spacing=POWER_SIZE+POWER_SIZE/4;

  // Find out which way round the power object goes
  switch (dir) {
	case 0:		// Top
		pa=CDPoint(pos.x,pos.y-POWER_SIZE);
		pb=CDPoint(pos.x-POWER_SIZE/2,pos.y-POWER_SIZE);
		pc=CDPoint(pos.x+POWER_SIZE/2+1,pos.y-POWER_SIZE);
		pd=CDPoint(pos.x,pos.y-POWER_SIZE*2);
		break;
	case 1:		// Bottom
		pa=CDPoint(pos.x,pos.y+POWER_SIZE);
		pb=CDPoint(pos.x-POWER_SIZE/2,pos.y+POWER_SIZE);
		pc=CDPoint(pos.x+POWER_SIZE/2+1,pos.y+POWER_SIZE);
		pd=CDPoint(pos.x,pos.y+POWER_SIZE*2);
		break;
	case 2:		// Left
		pa=CDPoint(pos.x-POWER_SIZE,pos.y);
		pb=CDPoint(pos.x-POWER_SIZE,pos.y-POWER_SIZE/2);
		pc=CDPoint(pos.x-POWER_SIZE,pos.y+POWER_SIZE/2+1);
		pd=CDPoint(pos.x-POWER_SIZE*2,pos.y);
		break;
	case 3:		// Right
		pa=CDPoint(pos.x+POWER_SIZE,pos.y);
		pb=CDPoint(pos.x+POWER_SIZE,pos.y-POWER_SIZE/2);
		pc=CDPoint(pos.x+POWER_SIZE,pos.y+POWER_SIZE/2+1);
		pd=CDPoint(pos.x+POWER_SIZE*2,pos.y);
		break;
  }

  dc.TextOut(str,TextPos,options);

  switch (options)
  {
  case draw_selected:
  case draw_selectable:
	  dc.SelectPen(PS_SOLID,1,cSELECT);
	  break;
  default:
	  dc.SelectPen(PS_SOLID,1,cLINE);
  }

  
  dc.SelectBrush();


  // Draw the main line of the power item
  switch (which) {
	case 0: // Draw the Bar
  		dc.MoveTo(pb);
  		dc.LineTo(pc);
		break;
	case 1: // Draw the Circle
		dc.Ellipse(CDRect(pb.x-(pa.x-pd.x),pb.y-(pa.y-pd.y),pc.x,pc.y));
		break;
	case 2: // Draw the Wave
		// The PolyBezier function is not implemented in MFC!
		// So this hack has to be used....
		{
		  CDPoint pts[4];
		  pts[0]=CDPoint(pb.x+(pd.x-pa.x)/2,pb.y+(pd.y-pa.y)/2);
		  pts[1]=CDPoint(pd.x+(pb.x-pa.x)/2,pd.y+(pb.y-pa.y)/2);
		  pts[2]=CDPoint(pa.x+(pc.x-pa.x)/2,pa.y+(pc.y-pa.y)/2);
		  pts[3]=CDPoint(pc.x-(pa.x-pd.x)/2,pc.y-(pa.y-pd.y)/2);
		  dc.PolyBezier(pts,4);
		}
		break;
	case 3: // Draw the Arrow
		dc.MoveTo(pb);
  		dc.LineTo(pc);
		dc.MoveTo(pb);
		dc.LineTo(pd);
		dc.LineTo(pc);
		break;
	case 4: // Draw the Earth
  		dc.MoveTo(pb);
  		dc.LineTo(pc);
		dc.MoveTo(CDPoint(pb.x-(pb.x-pd.x)/2,pb.y-(pb.y-pd.y)/2));
		dc.LineTo(CDPoint(pc.x-(pc.x-pd.x)/2,pc.y-(pc.y-pd.y)/2));
		dc.MoveTo(CDPoint(pd.x-(pb.x-pa.x)/4,pd.y-(pb.y-pa.y)/4));
		dc.LineTo(CDPoint(pd.x+(pb.x-pa.x)/4,pd.y+(pb.y-pa.y)/4));
		break;
  }



  // Draw the bits that all power items have in comman
  dc.MoveTo(pos);
  dc.LineTo(pa);


  if (is_stuck)
  {
	  // Draw a nice circle to show the stickness...
  	  dc.PaintConnectPoint( m_point_b );

	  // Do we need a junction
	  if (is_junction)
	  {
	    int js=JUNCTION_SIZE;
		CDPoint br,tl;
		br=CDPoint(m_point_b.x+js,m_point_b.y+js);
		tl=CDPoint(m_point_b.x-js,m_point_b.y-js);

		dc.SetROP2(R2_COPYPEN);

		dc.SelectPen(PS_SOLID,1,m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::JUNCTION));
		dc.SelectBrush(m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::JUNCTION));
  	    dc.Ellipse1(CDRect(tl.x,tl.y,br.x,br.y));
	  }
  }

}


void CDrawPower::EndEdit()
{
	RButtonDown(CDPoint(0,0), CDPoint(0,0));
	g_EditToolBar.m_PowerEdit.Close();
}

// Store the line in the drawing
CDrawingObject* CDrawPower::Store()
{
  CDrawPower *NewObject;

  NewObject = new CDrawPower(m_pDesign);
  *NewObject = *this;
  NewObject->is_junction = FALSE;
  NewObject->is_stuck = FALSE;

  m_pDesign->Add(NewObject);
  if (is_junction)
  {
	  m_pDesign->AddNewJunction( m_point_a );
  }

  return NewObject;
}


// Extract the netlist/active points from this object
void CDrawPower::GetActiveListFirst( CActiveNode &a )
{
	// Do nothing...
}

bool CDrawPower::GetActive( CActiveNode &a )
{
	if (a.m_sent > 0)
	{
		return false;
	}

	a.m_a = m_point_a;
	++ a.m_sent;
	return true;
}

