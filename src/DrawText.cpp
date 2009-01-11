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



////// The Text Class //////


void CDrawText::TagResources()
{
 	m_pDesign->GetOptions()->TagFont(FontStyle);
}


BOOL CDrawText::CanEdit()
{
  return TRUE;
}

// Look for a seach string in the object
CString CDrawText::Find(const TCHAR *theSearchString)
{
  CString HoldString;

  HoldString = str;
  HoldString.MakeLower();

  // Now look for the search string in this string
  return (HoldString.Find(theSearchString) != -1) ? str : "";
}

void CDrawText::NewOptions()
{
  CalcLayout();
  Display();
  FontStyle = m_pDesign->GetOptions()->GetCurrentFont( GetType() );
  original_width = 0;
  CalcLayout();
  Display();
}

void CDrawText::CalcLayout()
{
  CDSize size = m_pDesign->GetTextExtent( str, FontStyle );
  if (dir>=2)
	m_point_b=CDPoint(m_point_a.x+size.cx,m_point_a.y-size.cy);
  else
	m_point_b=CDPoint(m_point_a.x-size.cy,m_point_a.y-size.cx);

}

int CDrawText::getMenuID()
{
	switch (xtype) {
		case xLabelEx2:
		case xLabelEx:
		case xLabel:
			return IDM_TOOLLABEL;
		case xTextEx2:
		case xTextEx:
		case xText:
			return IDM_TOOLTEXT;
		case xBusNameEx:
			return IDM_TOOLBUSNAME;
	}

	return -1;
}


const TCHAR* CDrawText::GetXMLTag(ObjType t)
{
	switch (t)
	{
	case xTextEx2:
	case xTextEx:
	case xText:
		return _T("TEXT");
		break;
	case xBusNameEx:
		return _T("BUSNAME");
		break;
	default:
		return _T("LABEL");
		break;
	}
}

// Load and save to an XML file
void CDrawText::SaveXML( CXMLWriter &xml )
{
	xml.addTag(GetXMLTag(xtype));

	xml.addAttribute( _T("pos"), CDPoint( m_point_a ) );
	xml.addAttribute( _T("direction"), dir );
	xml.addAttribute( _T("font"), FontStyle );
	xml.addAttribute( _T("color"), FontColour );
	xml.addChildData( str );
	xml.closeTag();
}

void CDrawText::LoadXML( CXMLReader &xml )
{
	xml.getAttribute( _T("pos"),  m_point_a );
	xml.getAttribute( _T("direction"), dir );
	xml.getAttribute( _T("font"), FontStyle );
	xml.getAttribute( _T("color"), FontColour );
	xml.getChildData( str );

	FontStyle = m_pDesign->GetOptions()->GetNewFontNumber(FontStyle);

	CDSize size=m_pDesign->GetTextExtent(str,FontStyle);
	if (dir>=2)
	{
		m_point_b=CDPoint(m_point_a.x+size.cx,m_point_a.y-size.cy);
	}
	else
	{
		m_point_b=CDPoint(m_point_a.x-size.cy,m_point_a.y-size.cx);
	}

	m_segment=0;
}



void CDrawText::Load(CStream &archive)
{
  m_point_a = ReadPoint(archive);
  archive >> dir >> str;
  switch (xtype) 
  {
  	case xLabelEx:
	case xTextEx:
	case xBusName:
		// Load the font number in
		archive >> FontStyle;

		// Convert the font number over
		FontStyle = m_pDesign->GetOptions()->GetNewFontNumber(FontStyle);
		break;
	case xTextEx2:
	case xLabelEx2:
	case xBusNameEx:
		// Load the font number in
		archive >> FontStyle;

	  	// Convert the font number over
		FontStyle = m_pDesign->GetOptions()->GetNewFontNumber(FontStyle);

		// Load in the font colour...
		archive >> FontColour;
		break;
	case xLabel:
		FontStyle = fTEXT;
		break;
	case xText:
		FontStyle = fTEXT;
		break;
	}


  switch (xtype)
  {
	case xLabel:
  	case xLabelEx:
		xtype = xLabelEx2;
		FontColour = m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::LABEL);
		break;
	case xBusName:
		xtype = xBusNameEx;
		FontColour = m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::LABEL);
		break;
	case xText:
	case xTextEx:
		xtype = xTextEx2;
		FontColour = cTEXT;
		break;
  }

  CDSize size=m_pDesign->GetTextExtent(str,FontStyle);

  if (dir>=2)
  {
	m_point_b=CDPoint(m_point_a.x+size.cx,m_point_a.y-size.cy);
  }
  else
  {
	m_point_b=CDPoint(m_point_a.x-size.cy,m_point_a.y-size.cx);
  }

  m_segment=0;
}


ObjType CDrawText::GetType()
{
  return xtype;
}

CString CDrawText::GetName() const
{
	switch (xtype)
	{
	case xLabelEx2:
		return "Label";
	case xBusNameEx:
		return "BusName";
	default:
		return "Text";
	}
}


void CDrawText::SetScalingWidths()
{
  if (dir >= 2)
  {
	original_box_width = m_point_b.x - m_point_a.x;
  }
  else
  {
	  original_box_width = m_point_b.y - m_point_a.y;
  }
  target_box_width = original_box_width;
  original_width = m_pDesign->GetOptions()->GetFontWidth( FontStyle );
}

void CDrawText::BeginEdit(BOOL re_edit)
{
  g_EditToolBar.m_TextEdit.Open(m_pDesign,this);
  m_pDesign->GetOptions()->SetCurrentFont(GetType(), FontStyle);
  SetScalingWidths();
}


void CDrawText::EndEdit()
{
  RButtonDown(CDPoint(0,0), CDPoint(0,0));
  g_EditToolBar.m_TextEdit.Close();
}




void CDrawText::Move(CDPoint p, CDPoint no_snap_p)
{
  Display();			

  if (xtype == xLabelEx2)
  {
    BOOL is_junction;
	p = m_pDesign->GetStickyPoint(p,FALSE,TRUE,is_stuck,is_junction );
  }
  m_point_a=p;
  m_segment=0;
  Display();
}


void CDrawText::MoveField(int w, CDPoint r)
{
	Display();

	CDRect rect;

	if (w == CRectTracker::hitMiddle || w == 11)
	{
		m_point_a += r;
	}
	else
	{	
		if (dir >= 2 )
		{
			rect = CDRect( m_point_a.x, m_point_b.y, m_point_a.x + target_box_width, m_point_a.y );
		}
		else
		{
			rect = CDRect( m_point_b.x, m_point_a.y+ target_box_width , m_point_a.x, m_point_a.y );
		}

		switch (w)
		{
		case CRectTracker::hitTopLeft:
			rect.left += r.x;
			rect.top += r.y;
			r.y = -r.y;
			break;
		case CRectTracker::hitTopRight:
			rect.right += r.x;
			rect.top += r.y;
			r.y = -r.y;
			r.x = -r.x;
			break;
		case CRectTracker::hitBottomRight:
			rect.right += r.x;
			rect.bottom += r.y;
			r.x = -r.x;
			break;
		case CRectTracker::hitBottomLeft:
			rect.left += r.x;
			rect.bottom += r.y;
			break;	
		case CRectTracker::hitTop:
			rect.top += r.y;
			r.y = -r.y;
			break;		
		case CRectTracker::hitRight:
			rect.right += r.x;
			r.x = -r.x;
			break;		
		case CRectTracker::hitBottom:
			rect.bottom += r.y;
			break;		
		case CRectTracker::hitLeft:
			rect.left += r.x;
			break;		
		}

		if (dir >= 2)
		{
			m_point_a.x = rect.left;
			m_point_a.y = rect.bottom;
			m_point_b.x = rect.right;
			m_point_b.y = rect.top;
		}
		else
		{
			m_point_b.x = rect.left;
			m_point_a.y = rect.bottom;
			m_point_a.x = rect.right;
			m_point_b.y = rect.top;
		}

		if (dir >= 2)
		{
			target_box_width = rect.Width();
		}
		else
		{
			target_box_width = -rect.Height();
		}

		if (original_box_width > 0)
		{
			FontStyle = m_pDesign->GetOptions()->ChangeFontSize(FontStyle, dir < 2 ? r.x : -r.y, 
				(original_width * target_box_width) / original_box_width );
		}

		Display();
	}
}

void CDrawText::LButtonDown(CDPoint p, CDPoint)
{
	// We can only store, if we have some text!
	if (str.IsEmpty())
	{
		g_EditToolBar.m_TextEdit.ReFocus();
		return;
	}

	// New undo level for each placement...
	m_pDesign->BeginNewChangeSet();

	Display();
	if (xtype == xLabelEx2)
	{
		BOOL is_junction;
		p = m_pDesign->GetStickyPoint(p,FALSE,TRUE,is_stuck,is_junction );
	}
	m_point_a=p;
	Store();
	Display();	// Write to screen

	// Now increment the number as necessary
	int hold=str.FindOneOf(_T("0123456789-"));
	// Only increment if old value was a number at end of string
	if (hold!=-1) {
		TCHAR NewNum[17];
		_itot_s(_tstoi(str.Mid(hold))+m_pDesign->GetNameDir(),NewNum,10);
		str = str.Left(hold)+NewNum;
	}
	g_EditToolBar.m_TextEdit.ReFocus();
	Display();
}


// The constructors

CDrawText::CDrawText(CTinyCadDoc *pDesign,ObjType NewType)
: CDrawRectOutline( pDesign )
{
	is_stuck = FALSE;
	xtype=NewType;
	dir=3;
	str="";
	m_segment=1;
	m_point_a=m_point_b=CDPoint(0,0);
	original_width = 0;
	original_box_width = 0;
	target_box_width = 0;
    FontStyle = m_pDesign->GetOptions()->GetCurrentFont(GetType());

	switch (xtype)
	{
	case xLabelEx2:
		FontColour = m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::LABEL);	
		break;
	case xBusNameEx:
		FontColour = m_pDesign->GetOptions()->GetUserColor().Get( CUserColor::LABEL);	
		break;
	default:
		FontColour = cTEXT;	
		break;
	}
}


void CDrawText::Display( BOOL erase )
{
	CDRect r( m_point_a.x,m_point_a.y,m_point_b.x,m_point_b.y);
	m_pDesign->InvalidateRect( r, erase, 8 );

	if (xtype == xBusNameEx)
	{
		if (dir>=2) 
		{
			r = CDRect(m_point_b.x+10,m_point_a.y-10,m_point_b.x-10,m_point_a.y+10);
		} 
		else 
		{
			r = CDRect(m_point_a.x-10,m_point_b.y-10,m_point_a.x+10,m_point_b.y+10);
		}
		m_pDesign->InvalidateRect( r, erase, 2 );
	}
}




// Display the text on the screen!
void CDrawText::Paint(CContext &dc,paint_options options)
{
  dc.SelectFont(*m_pDesign->GetOptions()->GetFont(FontStyle),dir);

  CDSize size=dc.GetTextExtent(str);
  if (dir>=2)
	m_point_b=CDPoint(m_point_a.x+size.cx,m_point_a.y-size.cy);
  else
	m_point_b=CDPoint(m_point_a.x-size.cy,m_point_a.y-size.cx);

  if (original_width == 0)
  {
    SetScalingWidths();
  }

  dc.SetTextColor(FontColour);
  dc.TextOut(str,m_point_a,options,dir);

  // Draw a little blob, so the user knows where it
  // is stuck to
  if (xtype == xLabelEx2)
  {
	  dc.SelectBrush();
	  dc.SelectPen( PS_SOLID,1,cBOLD );
	  dc.Rectangle(CDRect(m_point_a.x-2,m_point_a.y-2,m_point_a.x+2,m_point_a.y+2));
  }

  // Is this a bus name?
  if (xtype == xBusNameEx)
  {
		switch (options)
		{
		case draw_selected:
			dc.SelectPen(PS_SOLID, 1 ,cSELECT);
			break;
		case draw_selectable:
			dc.SelectPen(PS_SOLID, 1 , cBLACK_CLK);
			break;
		default:
			dc.SelectPen(PS_SOLID, 1 , cBLACK);
		}

		dc.SetROP2(R2_COPYPEN);

		if (dir>=2) 
		{
			dc.MoveTo(CDPoint(m_point_b.x+10,m_point_a.y-10));
			dc.LineTo(CDPoint(m_point_b.x-10,m_point_a.y+10));
		} 
		else 
		{
			dc.MoveTo(CDPoint(m_point_a.x-10,m_point_b.y-10));
			dc.LineTo(CDPoint(m_point_a.x+10,m_point_b.y+10));
		}  
  }

  if (is_stuck)
  {
	  // Draw a small circle to show the stickness...
	  dc.SelectBrush();
	  dc.SelectPen(PS_SOLID,1,cBOLD);
	  dc.SetROP2(R2_COPYPEN);
	  dc.Ellipse(CDRect(m_point_a.x+HIGHLIGHT_SIZE/2,m_point_a.y+HIGHLIGHT_SIZE/2,m_point_a.x-HIGHLIGHT_SIZE/2,m_point_a.y-HIGHLIGHT_SIZE/2));
  }

}


// Store the text in the drawing
CDrawingObject* CDrawText::Store()
{
  CDrawText *NewObject;

  NewObject = new CDrawText(m_pDesign, xtype);
  *NewObject = *this;
  NewObject->is_stuck = FALSE;

  m_pDesign->Add(NewObject);

  return NewObject;
}


// Rotate this object about a point
void CDrawText::Rotate(CDPoint p,int ndir)
{
  // Translate this point so the rotational point is the origin
  m_point_a = CDPoint(m_point_a.x-p.x,m_point_a.y-p.y);
  m_point_b = CDPoint(m_point_b.x-p.x,m_point_b.y-p.y);

  if (ndir!=4) {
	if (dir>=2)
		dir = 0;
	else
		dir = 3;
  }

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

  CDPoint la = m_point_a;

  // Adjust for the rotation
  if (dir>=2) 
  {
	m_point_a = CDPoint(min(la.x,m_point_b.x),max(la.y,m_point_b.y));
	m_point_b = CDPoint(max(la.x,m_point_b.x),min(la.y,m_point_b.y));
  } 
  else 
  {
	m_point_a = CDPoint(max(la.x,m_point_b.x),max(la.y,m_point_b.y));
	m_point_b = CDPoint(min(la.x,m_point_b.x),min(la.y,m_point_b.y));
  }
}
