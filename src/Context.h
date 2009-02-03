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



#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "option.h"
#include "ArcPoint.h"
#include "DSize.h"

#include <map>

#define STRLEN		256

#define MAX_FONTS	8
#define MAX_PENS	20
#define MAX_BRUSHES	4

// CUSTOM PEN NUMBERS MUST BE 9 OR HIGHER
#define PS_MARQUEE  9
#define PS_MARQUEE2 10


// The class that performs the snap to grid, when necessary
class TransformSnap
{
protected:
	double	grid;				// The spacing on the grid
	BOOL	GridSnap;			// Should we snap to grid?
public:
	// Set the current grid settings
	void SetAccurateGrid(double NewGrid) { grid = NewGrid / 10.0; }
	void SetGridSnap(BOOL NewSnap) { GridSnap = NewSnap; }

	// Get the current settings
	double GetAccurateGrid() { return grid * 10.0; }	
	double GetGrid() { return grid; }	
	BOOL GetGridSnap() { return GridSnap; }

	// Snap co-ords to the grid
	double Snap( double f );
	CDPoint Snap(CDPoint p) { return CDPoint(Snap(p.x),Snap(p.y)); }

	// The constructor
	TransformSnap();

	void SaveXML( CXMLWriter &xml )
	{
		xml.addTag( _T("GRID") );
		xml.addAttribute( _T("spacing"), grid * 10.0 );
		xml.addAttribute( _T("snap"), GridSnap );
		xml.closeTag();
	}

	void LoadXML( CXMLReader &xml )
	{
		xml.getAttribute( _T("spacing"), grid );
		xml.getAttribute( _T("snap"), GridSnap );
		grid /= 10.0;
	}


	void Load( CStream &theArchive, BOOL keep )
	{
		if (keep)
		{
			LONG igrid;
			theArchive >> igrid >> GridSnap;
			SetAccurateGrid((double)igrid);
		}
		else
		{
			LONG dummy1;
			BOOL dummy2;
			theArchive >> dummy1 >> dummy2;
		}
	}

};

// The Transform context, which combines transform and scalings
class Transform {

	double	m_zoom;				// The zoom of the drawing
	double	scaling_x;			// The scaling of a method part
	double	scaling_y;			// The scaling of a method part
	double	m_x,m_y;			// The coord of the top left hand corner (The origin)
	int		rotmir;				// The rotation and mirroring

	CPoint	pixel_offset;		// Pixel perfect offset for drawing to off-screen buffer


	BOOL	IsYUp;				// Should we invert the y-axis?
	int		BoundY;				// The Y bound - used when y is up

	// Internal scaling functions
	int ScaleX(double a) const;
	int ScaleY(double a) const;

	int Round(double d) const;

public:

	// Change the settings
	void SetZoomFactor( double NewZoom );
	void SetOriginX(double NewX);
	void SetOriginY(double NewY);
	void SetOrigin(CDPoint p);
	void SetYisup(int newBoundY);
	void SetScaling( int rot, double x, double y );
	void SetPixelOffset( CPoint &p );
	double GetZoomPixelScale() const;

	// Change the transform by rotating it 
	CDPoint SetTRM(CDPoint,CDPoint,int);
	void EndTRM(CDPoint);
	int RotateDir(int) const;
	CPoint GetPixelOffset() const;

	CDPoint GetOrigin() const;
	CDPoint GetScaledOrigin() const;
	double GetZoomFactor() const;
	void GetScaling( int rot, double &x, double &y ) const;
	BOOL GetIsYUp() const;

	int GetRotMir() const;
	

	// Convert from internal coord into window coords
	CPoint Scale(CDPoint) const;
	double doubleScale(double a) const;
	CRect Scale(CDRect r) const ;


	// Convert from window coords into internal coords
	CDPoint DeScale(TransformSnap &s,CPoint p) const;
	double doubleDeScale( LONG a ) const;
	double DeScale( double a ) const;
	double DeScaleX(TransformSnap &s, LONG a) const;
	double DeScaleY(TransformSnap &s, LONG a) const;

	CDPoint DeScale(CPoint p) const;
	double DeScaleX(LONG a) const;
	double DeScaleY(LONG a) const;

	// The constructor
	Transform();
};

class CTinyCadDoc;

inline bool operator<(const LOGFONT&a,const LOGFONT&b)
{
	// Escapement
	if (a.lfEscapement < b.lfEscapement)
		return true;
	if (a.lfEscapement > b.lfEscapement)
		return false;

	// Orientation
	if (a.lfOrientation < b.lfOrientation)
		return true;
	if (a.lfOrientation > b.lfOrientation)
		return false;

	// Height
	if (a.lfHeight < b.lfHeight )
		return true;
	if (a.lfHeight > b.lfHeight )
		return false;

	// Width
	if (a.lfWidth < b.lfWidth )
		return true;
	if (a.lfWidth > b.lfWidth )
		return false;

	// Weight
	if (a.lfWeight < b.lfWeight )
		return true;
	if (a.lfWeight > b.lfWeight )
		return false;

	// Italic
	if (a.lfItalic < b.lfItalic )
		return true;
	if (a.lfItalic > b.lfItalic )
		return false;

	// Underline
	if (a.lfUnderline < b.lfUnderline )
		return true;
	if (a.lfUnderline > b.lfUnderline )
		return false;

	// Strikeout
	if (a.lfStrikeOut < b.lfStrikeOut )
		return true;
	if (a.lfStrikeOut > b.lfStrikeOut )
		return false;

	// Charset
	if (a.lfCharSet < b.lfCharSet )
		return true;
	if (a.lfCharSet > b.lfCharSet )
		return false;

	// Pitch And Family
	if (a.lfPitchAndFamily < b.lfPitchAndFamily)
		return true;
	if (a.lfPitchAndFamily > b.lfPitchAndFamily)
		return false;

	if (_tcsicmp(a.lfFaceName, b.lfFaceName) < 0)
		return true;
	if (_tcsicmp(a.lfFaceName, b.lfFaceName) > 0)
		return false;

	// They are identical as far as we are concerned
	return false;
}



// The Context class, a sort of extented DC
class CContext {

	BOOL		deleteDC;			// Delete the DC when destructor called
	BOOL		allBlack;			// Make all colours Black
	BOOL		allGrey;			// Make all colours lighter

	CSize		GetTextExtentI(CString);


	// The selectable objects into the DC

	// struct to hold font information
	class sFont
	{
	public:
		CFont	m_paint_font;
		CFont	m_datum_font;

		// The constructor
		sFont()
		{
		}

		// The destructor
		~sFont()
		{
		}
	};


	typedef std::map<LOGFONT,sFont*> font_map;
	font_map			m_fonts;
	font_map::iterator	m_selected_font;
	
	double				m_datum_scaling;

	class sPen
	{
	private:
		int			m_width;
		COLORREF	m_colour;
		int			m_style;
		
	public:
		sPen( int width, COLORREF colour, int style )
		{
			m_width = width;
			m_colour = colour;
			m_style = style;
		}

		bool operator<( const sPen &b ) const
		{
			if (m_width < b.m_width)
				return true;
			if (m_width > b.m_width)
				return false;

			if (m_colour < b.m_colour)
				return true;
			if (m_colour > b.m_colour)
				return false;

			if (m_style < b.m_style)
				return true;
			if (m_style > b.m_style)
				return false;

			// Identical...
			return false;
		}
	};
	typedef std::map<sPen,CPen*>	pen_map;
	pen_map		m_pens;

	class sBrush
	{
	private:
		COLORREF	m_colour;
		int			m_index;
	public:
		
		sBrush( COLORREF colour, int index )
		{
			m_colour = colour;
			m_index = index;
		}

		bool operator<( const sBrush &b ) const
		{
			if (m_colour < b.m_colour)
				return true;
			if (m_colour > b.m_colour)
				return false;

			if (m_index < b.m_index)
				return true;
			if (m_index > b.m_index)
				return false;

			// Identical...
			return false;
		}
	};

	typedef		std::map<sBrush, CBrush*> brush_map;
	brush_map	m_brushes;

	Transform	m_Transform;		// The transform parameter associated with this Context

	CWnd		*m_pWnd;			// The window associated with this device

	CDC			*m_pDC;				// The device context associated with this Context
	int			m_SavePoint;		// The point to RestoreDC() at

	// Select the font right away...
	void SelectFontNow( bool datum );

public:
	void PaintTracker( CDRect &r );
	void PaintConnectPoint( CDPoint p );


	CContext(CWnd *,Transform);		// Construction from window
	CContext(CDC *,Transform, CWnd *pWnd = NULL);		// Construction from CDC

	virtual ~CContext();							// The destructor

	CDC* GetDC() { return m_pDC; }

	// The transform operators...
	const Transform& GetTransform() const { return m_Transform; }
	CDPoint SetTRM(CDPoint,CDPoint,int);
	void EndTRM(CDPoint);
	void SetOrigin(CDPoint p);
	void SetZoomFactor(double NewZoom);
	void SetScaling( int rot, double x, double y );
	void SetPixelOffset( CPoint &p );


	// Select objects to paint with
	BOOL SelectPen(int Style,int Width,LONG Colour, paint_options options = draw_normal);
	BOOL SelectPen(LineStyle *pStyle, paint_options options );
	BOOL SelectBrush(COLORREF Colour, int index = 0);
	BOOL SelectBrush(FillStyle *pStyle) { return SelectBrush( pStyle->Colour, pStyle->Index); }
	BOOL SelectBrush() { m_pDC->SelectStockObject(HOLLOW_BRUSH); return TRUE; }
	BOOL SelectFont(LOGFONT &lf, int Rotation );
	

	// Change the parameters
	void SetBlack(BOOL NewBlack) { allBlack = NewBlack; }
	void SetROP2(int r) { m_pDC->SetROP2(r); }
	void SetTextColor(LONG Colour) { m_pDC->SetTextColor(allBlack ? RGB(0,0,0) : Colour); }
	void SetTextAlign(int a) { m_pDC->SetTextAlign(a); }
	void SetBkMode(int a) { m_pDC->SetBkMode(a); }
	void SetGreyMode(BOOL a) { allGrey = a; }

	// Get the parameters
	BOOL GetBlack() { return allBlack; }
	CDRect GetUpdateRegion();
	BOOL IsMetaFile() { return m_Transform.GetIsYUp(); }
	

	// Misc operations
	void Invalidate() 
	{ 
		if (m_pWnd!=NULL) m_pWnd->Invalidate(); 
	}

	void InvalidateRect( CDRect r, BOOL erase ,int grow )
	{
		if (m_pWnd != NULL)
		{
			if (r.top > r.bottom)
			{
				double q = r.top;
				r.top = r.bottom;
				r.bottom = q;
			}
			if (r.left > r.right)
			{
				double q = r.right;
				r.right = r.left;
				r.left = q;
			}

			r.left -= grow;
			r.top -= grow;
			r.bottom += grow;
			r.right += grow;

			m_pWnd->InvalidateRect( m_Transform.Scale(r), erase );
		}
	}

  
	// The drawing primitives
	void Ellipse(CDRect r) { m_pDC->Ellipse(m_Transform.Scale(r)); }
	void Rectangle(CDRect r) { m_pDC->Rectangle(m_Transform.Scale(r)); }
	void Polyline(pointCollection &points, CDPoint offset, FillStyle *pStyle);
	void LineTo(CDPoint p) { m_pDC->LineTo(m_Transform.Scale(p)); }
	void MoveTo(CDPoint p) { m_pDC->MoveTo(m_Transform.Scale(p)); }
	void SetPixel(CDPoint p,int c) { CPoint q = m_Transform.Scale(p); m_pDC->SetPixel(q.x,q.y,c); }
	void QuaterArc(CDPoint, CDPoint);
	void PolyBezier(CDPoint *pts,int);
	void TextOut(CString,CDPoint,paint_options,int=3);
	void TextOut(double x,double y,const TCHAR *t);
	void TextOut(double x,double y, int width, const TCHAR *t);
	CDSize GetTextExtent(CString s);
	void SelectDefault();
};


#endif
