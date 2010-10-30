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

#include "stdafx.h"
#include "revision.h"
#include "registry.h"
#include "option.h"
#include "context.h"
#include "registry.h"
#include "colour.h"
#include "math.h"
#include "registry.h"
#include "TinyCad.h"


////// The methods for the Context class //////

// The constructor
CContext::CContext(CWnd *NewWindow, Transform NewTransform)
{
  m_pWnd = NewWindow;

  m_pDC = new CClientDC(m_pWnd);
  m_SavePoint = m_pDC->SaveDC();

  deleteDC = TRUE;

  m_Transform = NewTransform;

  m_selected_font = m_fonts.end();
  m_datum_scaling = 5.0;

  allBlack = FALSE;
  allGrey  = FALSE;

}

CContext::CContext(CDC *NewDC, Transform NewTransform, CWnd *pWnd)
{
  m_pWnd = pWnd;

  m_pDC = NewDC;
  m_SavePoint = m_pDC->SaveDC();

  deleteDC = FALSE;

  m_Transform = NewTransform;

  m_selected_font = m_fonts.end();
  m_datum_scaling = 5.0;

  allBlack = FALSE;
  allGrey  = FALSE;
}


// The destructor
CContext::~CContext()
{
  // Unselect the gdi objects before they can be deleted
  m_pDC->RestoreDC(m_SavePoint);

  // Now delete the gdi objects
  font_map::iterator itf = m_fonts.begin();
  while (itf != m_fonts.end())
  {
	  delete (*itf).second;
	  ++ itf;
  }

  pen_map::iterator itp = m_pens.begin();
  while (itp != m_pens.end())
  {
	  delete (*itp).second;
	  ++ itp;
  }

  brush_map::iterator itb = m_brushes.begin();
  while (itb != m_brushes.end())
  {
	  delete (*itb).second;
	  ++ itb;
  }

  // Delete the DC (if necessary)
  if (deleteDC)
  {
	delete m_pDC;
  }
}

////// The transform operators //////

CDPoint CContext::SetTRM(CDPoint a,CDPoint b ,int c)
{
	CDPoint r = m_Transform.SetTRM( a, b, c );
	return r;
}

void CContext::EndTRM(CDPoint a)
{
	m_Transform.EndTRM( a );
}

void CContext::SetOrigin(CDPoint p)
{
	m_Transform.SetOrigin( p );
}

void CContext::SetZoomFactor(double NewZoom)
{
	m_Transform.SetZoomFactor( NewZoom );
}

void CContext::SetScaling( int rot, double x, double y )
{
	m_Transform.SetScaling( rot, x, y );
}

void CContext::SetPixelOffset( CPoint &p ) 
{ 
	m_Transform.SetPixelOffset( p );
}





////// The selection of drawing objects //////

BOOL CContext::SelectPen(LineStyle *pStyle, paint_options options)
{
	return SelectPen(pStyle->Style,pStyle->Thickness,pStyle->Colour, options);
}



BOOL CContext::SelectPen(int Style,int Width,LONG Colour, paint_options options)
{

	switch (options)
	{
	case draw_selected:
		Colour = cSELECT;
		break;
	case draw_selectable:
		{
			COLORREF col = Colour;

			// Calculate a rough appearent luminosity of the color.
			// The human eye is not very sensitive to the color blue, 
			// that's why its contribution in the calculation is halved.
			if (GetRValue(col) + GetGValue(col) + (GetBValue(col)>>1) < (3 * 128))
			{
				// Dark colors will become light gray.
				Colour = RGB(192, 192, 192);
			}
			else
			{
				// Light colors will become black.
				Colour = RGB(0, 0, 0);
			}
		}
		break;
	}	


  //int Selected = -1;

  if (allBlack)
	Colour = RGB(0,0,0);

  if (allGrey)
  	Colour = RGB( GetRValue(Colour) >> 1, GetGValue(Colour) >> 1, GetBValue(Colour) >> 1);

  
  if (Width > 1 || m_pDC->IsPrinting())
  {
	Width = static_cast<int>(m_Transform.doubleScale(Width));
  }

  // Does this pen already exist?
  pen_map::iterator itp = m_pens.find( sPen( Width, Colour, Style ) );

  CPen *pPen = NULL;
  if (itp == m_pens.end())
  {
		// If it doesn't then create it
		pPen  = new CPen(); 

		DWORD pCustStyle[10];
		int new_style;
		int style_count = 0;

		if (Style != PS_SOLID && CTinyCadApp::IsWinNT())
		{
			const int dot = 4;
			const int dash = 8;

			switch (Style)
			{
			case PS_ALTERNATE:	// Pen is dotted.
				{
					pCustStyle[0] = dot;
					pCustStyle[1] = dot;
					style_count = 2;
				}
				break;
			case PS_DASH:		// Pen is dashed.
				{
					pCustStyle[0] = dash;
					pCustStyle[1] = dash;
					style_count = 2;
				}				
				break;
			case PS_DOT:		// Pen is dotted.
				{
					pCustStyle[0] = dot;
					pCustStyle[1] = dot;
					style_count = 2;
				}
				break;
			case PS_DASHDOT:	// Pen has alternating dashes and dots.
				{
					pCustStyle[0] = dash;
					pCustStyle[1] = dot;
					pCustStyle[2] = dot;
					pCustStyle[3] = dot;
					style_count = 4;
				}
				break;
			case PS_DASHDOTDOT:	// Pen has alternating dashes and double dots.
				{
					pCustStyle[0] = dash;
					pCustStyle[1] = dot;
					pCustStyle[2] = dot;
					pCustStyle[3] = dot;
					pCustStyle[4] = dot;
					pCustStyle[5] = dot;
					style_count = 6;
				}
				break;
			case PS_MARQUEE:	// Marquee pen.
				{
					pCustStyle[0] = dot;
					pCustStyle[1] = dot;
					style_count = 2;
				}
				break;
			case PS_MARQUEE2:	// marquee pen variant 2.
				{
					pCustStyle[0] = dash;
					pCustStyle[1] = dot;
					pCustStyle[2] = dot-1;
					pCustStyle[3] = dot;
					style_count = 4;
				}
				break;
			}
			new_style = PS_USERSTYLE | PS_GEOMETRIC;

			// Don't scale the marquee pen
			if (Style != PS_MARQUEE && Style != PS_MARQUEE2)
			{
				// Now scale with the transform
				for (int i = 0; i < style_count; i ++)
				{
					pCustStyle[i] = static_cast<int>(m_Transform.doubleScale( pCustStyle[i] ));
				}
			}
		}
		else
		{
			new_style = Style | PS_GEOMETRIC;
		}

		LOGBRUSH lb;
		lb.lbColor = Colour;
		lb.lbHatch = 0;
		lb.lbStyle = BS_SOLID;
		pPen->Attach( ::ExtCreatePen( new_style, Width, &lb, style_count, style_count > 0 ? pCustStyle : NULL ) );
		m_pens[ sPen( Width, Colour, Style ) ] = pPen;
	}
  else
  {
	  pPen = (*itp).second;
  }


  // Now select the pen
  m_pDC->SelectObject(pPen);

  return TRUE;
}




BOOL CContext::SelectBrush(COLORREF Colour, int Index)
{
  //int Selected = -1;

  if (allBlack)
	Colour = RGB(0,0,0);

  // Does this brush already exist?
  brush_map::iterator itb = m_brushes.find( sBrush( Colour, Index ) );

  CBrush *pBrush = NULL;

  // If it doesn't then create it
  if (itb == m_brushes.end()) 
  {

	if (Index == 0)
	{
		pBrush = new CBrush(Colour);
	}
	else
	{
		pBrush = new CBrush(Index-1,Colour);
	}
	m_brushes[ sBrush( Colour, Index ) ] = pBrush;
  }
  else
  {
	  pBrush = (*itb).second;
  }

  // Now select the brush
  m_pDC->SelectObject(pBrush);

  return TRUE;
}



BOOL CContext::SelectFont(LOGFONT &plf,
		int iRotation )
{
  //int Selected = -1;
  LOGFONT lf = plf;

  int iWidth = lf.lfWidth;
  int iHeight = lf.lfHeight;
  int Sign = iHeight>0 ? 1 : -1;

  int dHeight = max(1,static_cast<int>(lf.lfHeight * m_datum_scaling) * Sign) * Sign;
  int dWidth = static_cast<int>(lf.lfWidth * m_datum_scaling);

	iHeight = max(1,static_cast<int>(m_Transform.doubleScale(iHeight*Sign))) * Sign;
	if (iWidth != 0)
	{
		iWidth = static_cast<int>(m_Transform.doubleScale(iWidth));
	}
  iRotation = m_Transform.RotateDir(iRotation);

  if (m_Transform.GetIsYUp())
  	iRotation = (iRotation>=2) ? 0 : -900;
  else
  	iRotation = (iRotation>=2) ? 0 : 900;

  	lf.lfHeight = iHeight;
	lf.lfWidth = iWidth;
	lf.lfEscapement = iRotation;
	lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
	lf.lfQuality = ANTIALIASED_QUALITY;

	// Does this font already exist?
	m_selected_font = m_fonts.find( lf );

	// If it doesn't then create it
	if (m_selected_font == m_fonts.end()) 
	{
		sFont *f = new sFont;
		m_fonts[lf] = f;
		m_selected_font = m_fonts.find( lf );

		// Create the new font
		f->m_paint_font.CreateFontIndirect(&lf);

		// Now create the datum font
  		lf.lfHeight = dHeight;
		lf.lfWidth = dWidth;
		f->m_datum_font.CreateFontIndirect(&lf);
	}

	return TRUE;
}

void CContext::SelectFontNow( bool datum )
{
	if (m_selected_font != m_fonts.end())
	{
		CFont* r;
		if (datum)
		{
			r = m_pDC->SelectObject(&(m_selected_font->second)->m_datum_font);
		}
		else
		{
			r = m_pDC->SelectObject(&(m_selected_font->second)->m_paint_font);
		}
	}
}



////// The drawing primitives //////

void CContext::PolyBezier(CDPoint *pts,int Size)
{
  // Make a new array to hold the points
  POINT *Np = new POINT[Size];

  // Convert each of the points before calling PolyBezier
  for (int lp=0;lp!=Size;lp++)
	Np[lp] = m_Transform.Scale(pts[lp]);

  // The PolyBezier function must be emulated in Win32s or for Metafiles
  if (IsMetaFile()) {
	if (Size >= 4) {
  		float x, y;
  		float t, t2, t3, a, b, c;
  		m_pDC->MoveTo((int)Np[0].x,(int)Np[0].y);
  
  		for (t=0.0f; t<=1.0f; t += 0.02f) {
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
   			m_pDC->LineTo((int)x, (int)y);
  		}
 	} else {
  		m_pDC->Polyline(Np, Size);
 	}
  } else {
#ifndef _cWIN16_
  ::PolyBezier(m_pDC->m_hDC,Np,Size);
#endif
  }

  delete Np;
}




#if 0
void CContext::Arc( LPCRECT lpRect, POINT ptStart, POINT ptEnd )
{
 	CPoint Recta = m_Transform.Scale(CPoint(lpRect->left ,lpRect->top));
 	CPoint Rectb = m_Transform.Scale(CPoint(lpRect->right,lpRect->bottom));
 	CPoint Start = m_Transform.Scale(ptStart);
 	CPoint End   = m_Transform.Scale(ptEnd);

	m_pDC->Arc(CRect(Recta.x,Recta.y,Rectb.x,Rectb.y), Start, End);
}
#endif


void CContext::QuaterArc(CDPoint da, CDPoint db)
{
	double Width  = (da.x - db.x);
	double Height = (da.y - db.y);

	CDRect dRect = CDRect(db.x - Width, da.y - Height, db.x + Width, da.y + Height);
	CRect theRect = m_Transform.Scale(dRect);

	CPoint b = m_Transform.Scale(db);
	CPoint a = m_Transform.Scale(da);

	CPoint Mid = CPoint(theRect.left + (theRect.right  - theRect.left)/2,
					    theRect.top +  (theRect.bottom - theRect.top )/2 );

	double PI = 3.14159;

	double ra = atan2((double)(a.x - Mid.x), (double)(a.y- Mid.y));
	double rb = atan2((double)(b.x - Mid.x), (double)(b.y- Mid.y));
	double diff = ra - rb;

	if (diff < (-PI) || (diff > 0.0 && diff < PI )) {
		CPoint hold = a;
		a = b;
		b = hold;
	}

	if (m_Transform.GetIsYUp()) {
		CPoint hold = a;
		a = b;
		b = hold;
	}

	m_pDC->Arc(theRect, a, b);
}


void CContext::Polyline(pointCollection &points, CDPoint offset, FillStyle *pStyle)
{ 
	CPoint *lpPoints = new CPoint[ points.size() ];

	pointCollection::iterator it = points.begin();
	int n = 0;
	while (it != points.end())
	{
		lpPoints[n] = m_Transform.Scale( *it + offset );
		++ it;
		++ n;
	}

	if (!pStyle || n == 2)
	{
		m_pDC->Polyline( lpPoints, n );
	}
	else
	{
		SelectBrush( pStyle );
		m_pDC->SetBrushOrg( lpPoints[0] );
		m_pDC->SetPolyFillMode( WINDING );
		m_pDC->Polygon( lpPoints, n );
	}

	delete[] lpPoints;
}


////// The text drawing functions //////
//
// On entry the dc should contain the text you wish to display
// Over-bars will be added automatically
// This function will either display the text, or a greyed box

// Calculate the extent of the text on the DC
CSize CContext::GetTextExtentI(CString text)
{
  int len=text.GetLength(),lp,lpi=0;
  TCHAR TextString[STRLEN];

  m_pDC->SetTextAlign(TA_LEFT | TA_BOTTOM | TA_NOUPDATECP);

  // seperate the strings into the overbars and text
  for (lp=0;lp!=len;lp++)
  {
	if (text[lp]!='`')
		TextString[lpi++]=text[lp];
  }

  TextString[lpi]=0;

  SIZE r = { 0, 0 };
  ::GetTextExtentPoint32( m_pDC->m_hDC, TextString, lpi, &r );
  //int e = GetLastError();
  return r;
}

CDSize CContext::GetTextExtent(CString s) 
{ 
	// Force the selection of the font
	SelectFontNow( true );

	CSize r = GetTextExtentI(s);
	//double scale = m_Transform.doubleDeScale( static_cast<int>(m_datum_scaling) );
	return CDSize(r.cx / m_datum_scaling, r.cy / m_datum_scaling ); 
}


// Display text on the screen without kerning
void CContext::TextOut(double x,double y,const TCHAR *t)
{ 
	SelectFontNow( false );
	CPoint r = m_Transform.Scale(CDPoint(x,y)); 
	m_pDC->TextOut(r.x,r.y,t); 
}


// Display the text on the screen with kerning
void CContext::TextOut(CString text,CDPoint da,paint_options options,int dir)
{
	TCHAR BarString[STRLEN];
	TCHAR TextString[STRLEN];

	// seperate the strings into the overbars and text
	int len = text.GetLength();
	int lpi = 0;
	int lp;
	for (lp=0;lp!=len;lp++) 
	{
		if (text[lp]=='`')
		{
			BarString[lpi]='_';
		}
		else 
		{
			BarString[lp]=' ';
			TextString[lpi]=text[lp];
			lpi++;
		}
	}
	BarString[lpi]=0;
	TextString[lpi]=0;
	len = lpi;

	int *widths = new int[ len ];

	CPoint a = m_Transform.Scale(da);
	dir = m_Transform.RotateDir(dir);


	// First we measure the text in both the datum
	// and the target device
	double *datum_width = new double[len];
	int *actual_width = new int[len];
	double scale = m_Transform.doubleDeScale( static_cast<int>(m_datum_scaling) );

    SelectFontNow( true );
	int i;
	for (i =0; i < len; ++i)
	{
		// Determine the size of this character
		ABC datum_abc;

		m_pDC->GetCharABCWidths( TextString[i], TextString[i], &datum_abc );
		datum_width[i] = (datum_abc.abcA + datum_abc.abcB + datum_abc.abcC) / scale; 
	}

	SelectFontNow( false );
	for ( i =0; i < len; ++i)
	{
		// Determine the size of this character
		ABC abc;

		m_pDC->GetCharABCWidths( TextString[i], TextString[i], &abc );
		actual_width[i] = (abc.abcA + abc.abcB + abc.abcC); 
	}


	// Now we apply kerning, we use a multi-pass process to produce the
	// best results:
	// First we only kern spaces, then we kern spaces and character and then
	// finally we kern increasing our limits until we have the correct text width

	double max_space_adjust = 0.2;
	double max_char_adjust = 0.0;
	double fpos;
	int ipos;
	int max_iterations = 5;

	do 
	{
		fpos = 0;
		ipos = 0;

		for (i =0; i < len; ++i)
		{
			double max_adjust = max_char_adjust;
			int iwidth;

			if (TextString[i] == ' ')
			{
				max_adjust = max_space_adjust;
			}


			// Use the datum width, but only adjust by up to 20%
			iwidth = static_cast<int>((fpos - ipos) + datum_width[ i ]);

			// Enforce max adjust rule
			if (actual_width[i] > 0)
			{
				if (iwidth / actual_width[i] < 1.0 - max_adjust)
				{
					iwidth = static_cast<int>(actual_width[i] * 1.0 - max_adjust);
				}

				if (iwidth / actual_width[i] > 1.0 + max_adjust)
				{
					iwidth = static_cast<int>(actual_width[i] * 1.0 + max_adjust);
				}
			}
			fpos += datum_width[ i ];
			ipos += iwidth;
			widths[i] = iwidth;
		}

		// Change our adjustments
		if (max_char_adjust < 0.1)
		{
			max_char_adjust = 0.1;
		}
		else
		{
			max_space_adjust *= 1.2;
			max_char_adjust *= 1.2;
		}
		-- max_iterations;

	// Keep going until we have less than 5 pixels error
	} while (fabs(fpos - ipos) > 5 && max_iterations > 0);

	delete[] datum_width;
	delete[] actual_width;

	int oldalign = m_pDC->SetTextAlign(TA_LEFT | TA_BOTTOM | TA_NOUPDATECP);

	// Write the text out
	m_pDC->SetROP2(R2_COPYPEN);
	m_pDC->SetBkMode(TRANSPARENT);

	// move the positioning to follow the rotations
	if (dir==1)
	{
		a=CPoint(a.x,a.y+ipos);
	}
	if (dir==2)
	{
		a=CPoint(a.x-ipos,a.y);
	}


	m_pDC->ExtTextOut( a.x,a.y,0,NULL,TextString,widths);

	// Now write the overbars
	// Now add bars to the text
	TCHAR Last=' ';
	CPen *oldPen = NULL;

	int iHeight = static_cast<int>(m_Transform.doubleScale(GetTextExtent( "A" ).cy));
	
	SelectPen(PS_SOLID,1,cSELECT);

	int x_pos = 0;
	for (lp=0;lp!=lpi;lp++) 
	{
		if (BarString[lp] == '_') 
		{
			if (dir>=2)
				m_pDC->MoveTo(a.x+x_pos,a.y-iHeight);
			else
				m_pDC->MoveTo(a.x-iHeight,a.y-x_pos);

			x_pos += widths[lp];

			if (dir>=2)
				m_pDC->LineTo(a.x+x_pos,a.y-iHeight);
			else
				m_pDC->LineTo(a.x-iHeight,a.y-x_pos);
		}
		else
		{
			x_pos += widths[lp];
		}

		Last=BarString[lp];
	}
	
	if (oldPen)
	{
		m_pDC->SelectObject( oldPen );
	}


	m_pDC->SetTextAlign( oldalign );

	delete[] widths;
}


// Text out with fixed width
void CContext::TextOut(double x,double y, int width, const TCHAR *t)
{
	// Force the selection of the font
	SelectFontNow( false );

	CPoint r = m_Transform.Scale(CDPoint(x,y)); 
	int s_width = static_cast<int>(m_Transform.doubleScale( width ));
	int s_height = m_pDC->GetTextExtent( t ).cy;

	CRect rect(r.x ,r.y - s_height,r.x+s_width,r.y );

	int old = m_pDC->SetTextAlign( DT_TOP | DT_LEFT );
	m_pDC->DrawText(t,rect,DT_SINGLELINE | DT_RIGHT | DT_TOP | DT_NOPREFIX );
	m_pDC->SetTextAlign( old );
}



CDRect CContext::GetUpdateRegion()
{
  TransformSnap snap;

  snap.SetGridSnap(FALSE);

  CRect rect;
  rect = (((CPaintDC *)m_pDC)->m_ps).rcPaint;

  CDPoint Start = m_Transform.DeScale(snap,CPoint(rect.left,rect.top));
  CDPoint End = m_Transform.DeScale(snap,CPoint(rect.right,rect.bottom));

  return CDRect(Start.x,Start.y,End.x,End.y);
}
 
void CContext::PaintConnectPoint( CDPoint dp )
{
	// Draw a nice circle to show the stickness...
	SelectBrush();

	// Descale before drawing
	int size = static_cast<int>(min( 4000, m_Transform.doubleScale( HIGHLIGHT_SIZE ) ));
	CPoint p = m_Transform.Scale( dp );
	SelectPen(PS_SOLID,0,cBOLD);
	SetROP2(R2_COPYPEN);
	m_pDC->Ellipse(CRect(p.x+size,p.y+size,p.x-size,p.y-size));
}

void CContext::PaintTracker(CDRect &r)
{
	// Convert to normal
	Transform t( m_Transform );

	CRect rect = t.Scale(r);
	int q = 1;
	rect.InflateRect( q,q,q,q );

	// Put some handles around this object
	CRectTracker	tracker( rect, CRectTracker::dottedLine | CRectTracker::resizeOutside  );
	tracker.Draw( GetDC() );

}
