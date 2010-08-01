/*
	TinyCAD program for schematic capture
	Copyright 1994/1995/2002,2003 Matt Pyne.

	This program is free software; you can redim_stribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This program is dim_stributed in the hope that it will be useful,
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

#include "DSize.h"

#include "TinyCadSymbolDoc.h"
#define TP_EDIT 1	//Feature contributed by Thomas Petersson.  This turns on auto-pin numbering.  The old code is left intact only in case there is an unforeseen problem.


////// The Pin Class //////


void CDrawPin::TagResources()
{
 	m_pDesign->GetOptions()->TagFont(fPIN);
}


void CDrawPin::OldLoad(CStream &archive)
{

  m_point_a = ReadPoint(archive);
  archive >> m_which >> m_elec >> m_dir >> m_part >> m_str >> m_number >> m_show;
  DetermineSize();
  m_segment=0;

  // Sneakily hide the smallpin parameter in the m_which parameter....
  BOOL smallpin = m_which & 128;
  m_which &= ~128;
  m_number_pos = 0;
  m_length = (smallpin ? 20 : 30);
  m_centre_name = IsPower();
}

void CDrawPin::Load(CStream &archive)
{
  m_point_a = ReadPoint(archive);
  archive >> m_which >> m_elec >> m_dir >> m_part >> m_str >> m_number >> m_show >> m_length;
  DetermineSize();
  m_segment=0;

    //Now that there is a "hotspot" graphical indicator, it  is possible to use zero length pins
  if ((m_length < 5) && (m_length != 0))
  {
      //Now that there is a "hotspot" graphical indicator, it may no longer be required to force a minimum pin length,
      //but since this was originally forced, I do not know whether this might be to handle some earlier symbol format conversion,
      //so I am leaving it in place, slightly less intact.
	  m_length = 20;
  }
  m_number_pos = 0;
  m_centre_name = IsPower();
}

const TCHAR* CDrawPin::GetXMLTag()
{
	return _T("PIN");
}

// Load and save to an XML file
void CDrawPin::SaveXML( CXMLWriter &xml )
{
	xml.addTag(GetXMLTag());
	xml.addAttribute( _T("pos"), CDPoint(m_point_a) );
    xml.addAttribute( _T("which"), m_which );
	xml.addAttribute( _T("elec"), m_elec );
	xml.addAttribute( _T("direction"), m_dir );
	xml.addAttribute( _T("part"), m_part );
	xml.addAttribute( _T("number"), m_number );
	xml.addAttribute( _T("show"), m_show );
	xml.addAttribute( _T("length"), m_length );
	xml.addAttribute( _T("number_pos"), m_number_pos );
	xml.addAttribute( _T("centre_name"), m_centre_name );

	xml.addChildData( m_str );

	xml.closeTag();
}

void CDrawPin::LoadXML( CXMLReader &xml )
{
    m_number_pos = 0;

	xml.getAttribute( _T("pos"), m_point_a );
    xml.getAttribute( _T("which"), m_which );
	xml.getAttribute( _T("elec"), m_elec );
	xml.getAttribute( _T("direction"), m_dir );
	xml.getAttribute( _T("part"), m_part );
	xml.getAttribute( _T("number"), m_number );
	xml.getAttribute( _T("show"), m_show );
	xml.getAttribute( _T("length"), m_length );
	xml.getAttribute( _T("number_pos"), m_number_pos );

	if (!xml.getAttribute( _T("centre_name"), m_centre_name ))
	{
		// Default power pins to centre, otherwise turn off
		m_centre_name = IsPower();
	}

	xml.getChildData( m_str );

	m_segment=0;

    //Now that there is a "hotspot" graphical indicator, it  is possible to use zero length pins
	if ((m_length < 5) && (m_length != 0))
	{
        //Now that there is a "hotspot" graphical indicator, it may no longer be required to force a minimum pin length,
        //but since this was originally forced, I do not know whether this might be to handle some earlier symbol format conversion,
        //so I am leaving it in place, slightly less intact.
		m_length = 20;
	}

	// Now determine it's size
	DetermineSize();
}


void CDrawPin::DetermineSize()
{
	// Is this pin hidden, or does it have a zero length?
	if (m_which == 4 || m_which == 5 || m_length == 0)
	{
		// Yes, so it has no size!
		m_point_b = m_point_a;
		return;
	}

	CDPoint pa,pb,pc,pd,pta,ptb;
	int dr;
	DetermineLayout( pa,pb,pc,pd,pta,ptb,dr, 0 );

	m_point_b = pta;

	if ((m_show&1)!=0)
	{
	  CDSize sz = m_pDesign->GetTextExtent( m_str, fPIN );
	  switch (m_dir)
	  {
	  case 0: // Up
		  m_point_b = CDPoint( pd.x, min( pta.y - sz.cx, pd.y ) );
		  break;
	  case 1: // Down
		  m_point_b = CDPoint(pd.x, max( pta.y, pd.y ) );
		  break;
	  case 2: // Left
		  m_point_b = CDPoint(min(pta.x,pd.x), pd.y);
		  break;
	  case 3: // Right
		  m_point_b = CDPoint( max(pta.x,pd.x), pd.y);
		  break;
	  }
	}

}

double CDrawPin::DistanceFromPoint( CDPoint p )
{
  // Only inside if it is in the current part
  if (IsInvisible())
  {
	  return 100.0;
  }

  // Now get the line part and determine if the user has clicked
  // on it...
  CDPoint pa,pb,pc,pd,pta,ptb;
  int dr;
  DetermineLayout( pa,pb,pc,pd,pta,ptb,dr, 0 );
  CLineUtils l(m_point_a, pd);
  CDPoint d;
  return l.DistanceFromPoint(p, d );
}

BOOL CDrawPin::IsInside(double left,double right,double top,double bottom)
{
  // Only inside if it is in the current part
  if (IsInvisible())
  {
	  return FALSE;
  }

  // IsInside for point
  if (left==right && top==bottom)
  {
	  double d = DistanceFromPoint(CDPoint(left, top));
	  double range = (10 / (m_pDesign->GetTransform().GetZoomFactor()));
	  return (d < range) ? TRUE : FALSE;
  }
  // Now get the line part and determine if the user has clicked
  // on it...
  CDPoint pa,pb,pc,pd,pta,ptb;
  int dr;
  DetermineLayout( pa,pb,pc,pd,pta,ptb,dr, 0 );
  CLineUtils l(m_point_a, pd);

  return l.IsInside(left,right,top,bottom );
}

BOOL CDrawPin::CanEdit()
{
  return TRUE;
}

ObjType CDrawPin::GetType()
{
  return xPinEx;
}


CString CDrawPin::GetName() const
{
  return _T("Pin");
}

// Look for a seach m_string in the object
CString CDrawPin::Find(const TCHAR *theSearchm_string)
{
  CString Holdm_string;

  Holdm_string = m_str;
  Holdm_string.MakeLower();

  // Now look for the search m_string in this m_string
  return (Holdm_string.Find(theSearchm_string) != -1) ? m_str : _T("");
}


// Construct a pin
CDrawPin::CDrawPin(CTinyCadDoc *pDesign)
: CDrawingObject( pDesign )
{
  m_segment=1;
  m_which=0;
  m_converted_power=0;
  m_dir=0;
  m_number="1";
  m_elec=0;
  m_str="";
  m_show=3;
  m_length = pDesign->GetOptions()->GetPinLength();
  m_part = m_pDesign->IsEditLibrary() ? static_cast<CTinyCadSymbolDoc*>(m_pDesign)->GetPart() : 0;
  m_number_pos = pDesign->GetOptions()->GetPinNumberPos();
  m_centre_name = FALSE;
}


CDPoint CDrawPin::GetActivePoint(CDrawMethod *parent)
{
  CDPoint r,p;
  CDPoint bound = parent->GetTr();
  CDPoint s = parent->m_point_a;
  CDPoint pos = m_point_a;


//  // We use point b if we are have a centred name
//  if (m_centre_name)
//  {
//	  pos = m_point_b;
//  }

  // Has this object been mirrored?
  if ((parent->GetRotate()&4)!=0)
	p = CDPoint(-pos.x + bound.x ,pos.y);
  else
	p = pos;

  parent->ScalePoint( p );

  switch ((parent->GetRotate())&3) {
	case 0:	// Up
		r.x = p.x + s.x;
		r.y = p.y + s.y;
		break;
	case 1: // Down
		r.x = p.x + s.x;
		r.y = (bound.y - p.y) + s.y;
		break;
	case 2: // Left
		r.x = p.y + s.x;
		r.y = p.x + s.y;
		break;
	case 3: // Right
		r.x = (bound.y - p.y) + s.x;
		r.y = p.x + s.y;
		break;
  }

  return r;
}


void CDrawPin::BeginEdit(BOOL re_edit)
{
  g_EditToolBar.m_PinEdit.Open(m_pDesign,this);
}

void CDrawPin::Move(CDPoint p, CDPoint no_snap_p)
{
  Display();
  m_point_a=p;
  DetermineSize();
  m_segment=0;
  Display();
}


void CDrawPin::ConvertPowerToNormal()
{
	// Convert this pin to a normal one
	// and set the 'pin is converted' indicater
	m_which = 0;
	m_converted_power = 1; 

	// Re-calculate our size
	DetermineSize();
}

// Determine whether or not this pin is visible
int CDrawPin::IsInvisible()
{
	if (m_pDesign->IsEditLibrary())
	{
		// When in a library symbol edit window, only show pins that are part of this
		// drawing
		return m_part != static_cast<CTinyCadSymbolDoc*>(m_pDesign)->GetPart();
	}
	else
	{
		// When on a normal drawing, don't show power pins
		return IsPower();
	}
}


#define SIZING		((m_length <= 20) ? 20 : 30)
#define DOT_SIZE	(SIZING/6)
#define LINE_SIZE	(m_length-DOT_SIZE*2)
#define TEXT_SPACE	(SIZING/8)

void CDrawPin::DetermineLayout( CDPoint &pa,CDPoint &pb,CDPoint &pc,CDPoint &pd,CDPoint &pta,CDPoint &ptb, int &dr, int rotmir )
{
  int spacing;
  CDPoint pos = m_point_a;
  CDSize size;

  dr=3;

  if ((m_which&2)==0)
	spacing=m_length+TEXT_SPACE;
  else
	spacing=m_length+(DOT_SIZE*2)+TEXT_SPACE;

  // Find out which way round the pin object goes
  switch (m_dir) {
	case 0:		// Up
		pa=CDPoint(pos.x,pos.y-LINE_SIZE);
		pb=CDPoint(pos.x-DOT_SIZE,pos.y-m_length);
		pc=CDPoint(pos.x+DOT_SIZE+1,pos.y-m_length);
		pd=CDPoint(pos.x,pos.y-m_length);

		size=m_pDesign->GetTextExtent(m_str, fPIN);
		if (m_centre_name)
		{
			pta=CDPoint(pos.x+size.cy,pos.y-(m_length - size.cx) /2);
		}
		else
		{
			pta=CDPoint(pos.x+size.cy/2,pos.y-spacing);
		}

		size=m_pDesign->GetTextExtent(m_str, fPIN);
		ptb=CDPoint(pos.x,pos.y-TEXT_SPACE - m_number_pos);
		dr=0;
		break;
	case 1:		// Down
		pa=CDPoint(pos.x,pos.y+LINE_SIZE);
		pb=CDPoint(pos.x-DOT_SIZE,pos.y+m_length);
		pc=CDPoint(pos.x+DOT_SIZE+1,pos.y+m_length);
		pd=CDPoint(pos.x,pos.y+m_length);

		size=m_pDesign->GetTextExtent(m_str, fPIN);
		if (m_centre_name)
		{
			pta=CDPoint(pos.x+size.cy,pos.y+(size.cx + m_length) /2);
		}
		else
		{
			pta=CDPoint(pos.x+size.cy/2,pos.y+spacing+size.cx);
		}
		size=m_pDesign->GetTextExtent(m_number, fPIN);
		ptb=CDPoint(pos.x,pos.y+size.cx+TEXT_SPACE+ m_number_pos);
		dr=0;
		break;
	case 2:		// Left
		pa=CDPoint(pos.x-LINE_SIZE,pos.y);
		pb=CDPoint(pos.x-m_length,pos.y-DOT_SIZE);
		pc=CDPoint(pos.x-m_length,pos.y+DOT_SIZE+1);
		pd=CDPoint(pos.x-m_length,pos.y);

		size=m_pDesign->GetTextExtent(m_str, fPIN);
		if (m_centre_name)
		{
			pta=CDPoint(pos.x - (size.cx + m_length) /2,pos.y+size.cy);
		}
		else
		{
			pta=CDPoint(pos.x-size.cx-spacing,pos.y+size.cy/2);
		}
		size=m_pDesign->GetTextExtent(m_number, fPIN);
		ptb=CDPoint(pos.x-size.cx-TEXT_SPACE - m_number_pos,pos.y);
		break;
	case 3:		// Right
		pa=CDPoint(pos.x+LINE_SIZE,pos.y);
		pb=CDPoint(pos.x+m_length,pos.y-DOT_SIZE);
		pc=CDPoint(pos.x+m_length,pos.y+DOT_SIZE+1);
		pd=CDPoint(pos.x+m_length,pos.y);

		size=m_pDesign->GetTextExtent(m_str, fPIN);
		if (m_centre_name)
		{
			pta=CDPoint(pos.x + (m_length - size.cx) /2,pos.y+size.cy);
		}
		else
		{
			pta=CDPoint(pos.x+spacing,pos.y+size.cy/2);
		}
		ptb=CDPoint(pos.x+TEXT_SPACE + m_number_pos,pos.y);
		break;
  }



  // Make corrections for bizarre rotations
  switch (m_dir)
  {
  case 2:
  case 3:
	    switch (rotmir)
		{
		case 1: // Down
		case 5: // Down & Mirror
		case 7: // Right
		case 3: // Right & Mirror
		  pta=CDPoint(pta.x, pta.y - size.cy );
		  ptb=CDPoint(ptb.x, ptb.y - size.cy );
		  break;
		}
  }

}


void CDrawPin::Display( BOOL erase )
{

  CDPoint pa,pb,pc,pd,pta,ptb;
  int dr;
  DetermineLayout( pa,pb,pc,pd,pta,ptb,dr, 0 );

	// Invalidate the symbol
	CDRect r( m_point_a.x -2,m_point_a.y, pd.x + 2,pd.y);
	m_pDesign->InvalidateRect( r, erase, DOT_SIZE + 6 );

	// Invalidate the no-connect bit...
	CDRect r2( m_point_a.x,m_point_a.y,m_point_b.x,m_point_b.y);
	m_pDesign->InvalidateRect( r2, erase, CONNECT_SIZE + 2 );


  if ((m_show&1)!=0)
  {
	  CDSize sz = m_pDesign->GetTextExtent( m_str, fPIN );
	  if (dr < 2)
	  {
		  r = CDRect( pta.x, pta.y, pta.x - sz.cy, pta.y - sz.cx);
	  }
	  else
	  {
		  r = CDRect( pta.x, pta.y - sz.cy, pta.x + sz.cx, pta.y + sz.cy );
	  }
	  m_pDesign->InvalidateRect( r, erase, 10 );
  }
  if ((m_show&2)!=0)
  {
  	  CDSize sz = m_pDesign->GetTextExtent( m_number, fPIN );
	  if (dr < 2)
	  {
		  r = CDRect( ptb.x - sz.cy, ptb.y - sz.cy, ptb.x + sz.cy, ptb.y + sz.cx);
	  }
	  else
	  {
		  r = CDRect( ptb.x, ptb.y - sz.cy, ptb.x + sz.cx, ptb.y + sz.cy );
	  }

	  m_pDesign->InvalidateRect( r, erase, 10 );
  }

}


// Display the pin item on the screen!
void CDrawPin::Paint(CContext &dc,paint_options options)
{
  // Only display the pin if it is in the current part
  if (IsInvisible() || (m_which == 5 && !(m_pDesign->IsEditLibrary())) )
	return;

  CDPoint pa,pb,pc,pd,pta,ptb;
  int dr;
  DetermineLayout( pa,pb,pc,pd,pta,ptb,dr, dc.GetTransform().GetRotMir() );

  dc.SetROP2(R2_COPYPEN);
  dc.SetTextColor(m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::POWER));

  dc.SelectBrush();
  dc.SelectFont(*m_pDesign->GetOptions()->GetFont(fPIN),dr);

  bool draw_dot = false;
  bool draw_line = false;
  bool draw_triangle = false;
  bool draw_cross = false;
  bool draw_no_connect = (m_elec == 6);
  bool hidden_colour = false;

  switch (m_which)
  {
  case 1: // Dot
	  draw_line = true;
	  draw_dot = true;
	  break;
  case 2: // Clock
	  draw_line = true;
	  draw_triangle = true;
	  break;
  case 3: // Dot Clock
	  draw_line = true;
	  draw_dot = true;
	  draw_triangle = true;
	  break;
  case 4: // Power
	  draw_line = true;
	  hidden_colour = true;
	  break;
  case 5: // Hidden
	  draw_line = true;
	  hidden_colour = true;
	  break;
  case 6: // Cross
	  draw_line = false;
	  draw_cross = true;
	  break;
  default: // Normal
      if (m_length != 0) {
    	  draw_line = true;
      }
      else {    //zero length pins don't draw a line, just the "hotspot"
          draw_line = false;
      }
	  break;
  }

  switch (options)
  {
  case draw_selected:
	    dc.SelectPen(PS_SOLID,1,cSELECT );
	  break;
  case draw_selectable:
	  dc.SelectPen(PS_SOLID,1,cPIN_CLK);
	  break;
  default:
	    if (hidden_colour)
		{
			dc.SelectPen(PS_SOLID,1,m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::HIDDEN_PIN));
		}
		else
		{
			dc.SelectPen(PS_SOLID,1,m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::PIN));
		}
  }


  if ( draw_dot )
  {
	// Draw the dot
	dc.Ellipse(CDRect(pb.x+(pa.x-pd.x),pb.y+(pa.y-pd.y),pc.x,pc.y));
  }
  else if (draw_line)
  {
	dc.MoveTo(pa);
	dc.LineTo(pd);
  }

  // Draw the clock (triangle)
  if (draw_triangle)
  {
	dc.MoveTo(pb);
  	dc.LineTo(pc);
	dc.MoveTo(pb);
	dc.LineTo(CDPoint(pd.x-(pa.x-pd.x),pd.y-(pa.y-pd.y)));
	dc.LineTo(pc);
  }

  // Draw the bits that all pins have in common
  if (draw_line)
  {
	  dc.MoveTo(m_point_a);
	  dc.LineTo(pa);
  }

  if ((m_show&1)!=0)
  {
	dc.TextOut(m_str,pta,options,dr);
  }

  if ((m_show&2)!=0 && !IsHierarchicalPin()) {
	dc.TextOut(m_number,ptb,options,dr);
  }

  // If this pin is a cross then draw it!
  if (draw_cross)
  {
	  CDPoint LineTo1=CDPoint(m_point_a.x+CONNECT_SIZE/2,m_point_a.y+CONNECT_SIZE/2);
	  CDPoint LineTo2=CDPoint(m_point_a.x+CONNECT_SIZE/2,m_point_a.y-CONNECT_SIZE/2);

	  dc.MoveTo(CDPoint(m_point_a.x-CONNECT_SIZE/2,m_point_a.y-CONNECT_SIZE/2));
	  dc.LineTo(CDPoint(LineTo1.x,LineTo1.y));
	  dc.MoveTo(CDPoint(m_point_a.x-CONNECT_SIZE/2,m_point_a.y+CONNECT_SIZE/2));
	  dc.LineTo(CDPoint(LineTo2.x,LineTo2.y));
  }

  // If this pin is a "not connected" then we must draw the NC cross on
  // the end of it....
  if (draw_no_connect)
  {
	  CDPoint LineTo1=CDPoint(m_point_a.x+CONNECT_SIZE,m_point_a.y+CONNECT_SIZE);
	  CDPoint LineTo2=CDPoint(m_point_a.x+CONNECT_SIZE,m_point_a.y-CONNECT_SIZE);

	  dc.MoveTo(CDPoint(m_point_a.x-CONNECT_SIZE,m_point_a.y-CONNECT_SIZE));
	  dc.LineTo(CDPoint(LineTo1.x,LineTo1.y));
	  dc.MoveTo(CDPoint(m_point_a.x-CONNECT_SIZE,m_point_a.y+CONNECT_SIZE));
	  dc.LineTo(CDPoint(LineTo2.x+1,LineTo2.y));
  }

  // Draw the "Hot Spot" on the screen, but not on a printout 
  if (!dc.GetDC()->IsPrinting()) 
  {
	  // The "Hot Spot" is a graphical aid to help users get the pin rotation correct and 
	  // not build symbols with backwards pins.  When printing a schematic (as opposed to
	  // viewing or editing a schematic on screen), the Hot Spot is omitted.
	  // The "Hot Spot" (HS) is a small rectangle on the connection end of the pin.
	  dc.SelectPen(PS_SOLID,1,cHOT_SPOT);
	  dc.Rectangle1(CDRect(m_point_a.x-CONNECT_SIZE/4,m_point_a.y-CONNECT_SIZE/4, m_point_a.x+CONNECT_SIZE/4,m_point_a.y+CONNECT_SIZE/4));
  }
}

BOOL CDrawPin::IsHierarchicalPin()
{
	return m_pDesign->IsHierarchicalSymbol();
}

void CDrawPin::EndEdit()
{
  RButtonDown(CDPoint(0,0), CDPoint(0,0));
  g_EditToolBar.m_PinEdit.Close();
}


#ifdef TP_EDIT
void IncrementNumber(CString &number, int increment) {
  int intpos = number.FindOneOf(_T("0123456789"));
  if (intpos >= 0) {
	CString nums = number.Mid(intpos).SpanIncluding(_T("0123456789"));
	int numi = _tstoi(nums) + increment;
	if (numi > 999) numi = 999;
	if (numi < 1) numi = 1;
	TCHAR newNum[4];
	#ifdef USE_VS2003
		_itot(numi, newNum, 10);
	#else	/* use the VS2008 "safe" version */
		_itot_s(numi, newNum, 3, 10);
	#endif
	number = number.Left(intpos) + newNum + number.Mid(intpos + nums.GetLength());
  } else {
	int lastpos = number.GetLength() - 1;
	if (lastpos >= 0) {
		TCHAR ch = number.GetAt(lastpos);
		ch += increment;
		if (ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z') {
			number.SetAt(lastpos, ch);
		}
	}
  }
}
#endif

void CDrawPin::LButtonDown(CDPoint p, CDPoint)
{
  // New undo level for each placement...
  m_pDesign->BeginNewChangeSet();

  Display();
  m_point_a=p;
  DetermineSize();
  Store();
  Display();	// Write to screen

#ifdef TP_EDIT
  IncrementNumber(m_number, m_pDesign->GetPinDir());
  IncrementNumber(m_str, m_pDesign->GetNameDir());
#else
  // Add 1 to the pin m_number (if possible)
  int hold=_tstoi(m_number);
  // Only increment if old value was a m_number
  if (hold!=0) {
	hold += m_pDesign->GetPinDir();
	if (hold>999 || hold<1) {
		hold=1;
	}
	#ifdef USE_VS2003
		_itot(hold++,m_number.GetBuffer(17),10);
	#else	/* use the VS2008 "safe" version */
		_itot_s(hold++,m_number.GetBuffer(17),17,10);
	#endif
	m_number.ReleaseBuffer();
  }

  hold=m_str.FindOneOf(_T("0123456789-"));
  // Only increment if old value was a m_number
  if (hold!=-1) {
	TCHAR NewNum[17];
	_itot_s(_tstoi(m_str.Mid(hold))+m_pDesign->GetNameDir(),NewNum,10);
	m_str = m_str.Left(hold)+NewNum;
  }
#endif


  g_EditToolBar.m_PinEdit.ReFocus();

  Display();
}


int CDrawPin::DoRotate(int olddir,int newdir)
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



// Rotate this object about a point
void CDrawPin::Rotate(CDPoint p,int ndir)
{
  // Translate this point so the rotational point is the origin
  m_point_a = CDPoint(m_point_a.x-p.x,m_point_a.y-p.y);
  m_point_b = CDPoint(m_point_b.x-p.x,m_point_b.y-p.y);

  // Perform the rotation
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

  m_dir = DoRotate(m_dir,ndir);
}

const TCHAR* CDrawPin::GetElectricalTypeName(int i)
{
	static TCHAR* ElectricalTypes[] = {
	  _T("Input"),
	  _T("Output"),
	  _T("Tristate"),
	  _T("Open Collector"),
	  _T("Passive"),
	  _T("Input/Output"), 
	  _T("Not Connected")
	  };

	if (i < 0 || i >= sizeof(ElectricalTypes)/sizeof(TCHAR*))
	{
		return NULL;
	}
	
	return ElectricalTypes[i];
}

// Store the line in the drawing
CDrawingObject* CDrawPin::Store()
{
  CDrawPin *NewObject;

  NewObject = new CDrawPin(m_pDesign);
  *NewObject = *this;

  if (IsHierarchicalPin())
  {
	  // Never display pin names for hierarchical symbols
	  NewObject->m_show &= 1;
  }

  m_pDesign->Add(NewObject);

  return NewObject;
}

