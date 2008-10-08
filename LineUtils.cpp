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

// LineUtils.cpp: implementation of the CLineUtils class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tinycad.h"
#include "LineUtils.h"
#include <math.h>
#include "Context.h"
#include "TinyCadDoc.h"

double	CLineUtils::pointOnLineDistance = 0.01;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLineUtils::CLineUtils(CDPoint ai, CDPoint bi )
{
	a = ai;
	b = bi;
}

CLineUtils::~CLineUtils()
{

}


BOOL CLineUtils::IsInside(double left,double right,double top,double bottom) const
{
	if (left == right && top == bottom)
	{
		// Determine if this point is on the line...
		CDPoint d;
		return DistanceFromPoint( CDPoint( right, top ), d ) < pointOnLineDistance;
	}
	else
	{
		return rIsInside(a,b,left,right,top,bottom);
	}
}


// How far is the line from a point?
double CLineUtils::DistanceFromPoint( CDPoint p, CDPoint &d) const
{
	double X,Y;

	// Is the line horizontal, vertical or other?
	if (a.y == b.y)
	{
		// Horizontal
		X = p.x;
		Y = a.y;
	}
	else if (a.x == b.x)
	{
		// Vertical
		X = a.x;
		Y = p.y;
	}
	else
	{
		// Other....

		// Determine the nearest point on the line
		// to this point.  
		//
		// The line is defined as:
		// x = g * y + h
		//
		//

		// Convert to a parametric equation
		double g = static_cast<double>(b.x - a.x) / static_cast<double>(b.y - a.y);
		double h = b.x - g * b.y;

		// Now find the nearest point
		Y = (p.y + p.x*g - g*h) / ( g*g + 1.0 );
		X = g * Y + h;
	}

	// We have calculated to an infinite line, if we
	// are past the end-points, bring on to the line...
	if (X < min(a.x,b.x))
	{
		if (a.x < b.x)
		{
			X = a.x;
			Y = a.y;
		}
		else
		{
			X = b.x;
			Y = b.y;
		}
	}

	if (X > max(a.x,b.x))
	{
		if (a.x > b.x)
		{
			X = a.x;
			Y = a.y;
		}
		else
		{
			X = b.x;
			Y = b.y;
		}
	}

	if (Y < min(a.y,b.y))
	{
		if (a.y < b.y)
		{
			X = a.x;
			Y = a.y;
		}
		else
		{
			X = b.x;
			Y = b.y;
		}
	}
	if (Y > max(a.y,b.y))
	{
		if (a.y > b.y)
		{
			X = a.x;
			Y = a.y;
		}
		else
		{
			X = b.x;
			Y = b.y;
		}
	}



	d = CDPoint( X, Y );
	double dx = p.x - X;
	double dy = p.y - Y;
	
	return sqrt(dx*dx + dy*dy);
}

BOOL CLineUtils::IsPointOnLine( CDPoint p, double &distance_along ) const
{
	CDPoint d;
	double distance = DistanceFromPoint( p, d );
	if (distance <= CLineUtils::pointOnLineDistance)
	{
		// On-line, so determine how far along it is...

		// Find the length of the line
		double length = sqrt(static_cast<double>((a.x - b.x)*(a.x - b.x)
				+ (a.y - b.y)*(a.y-b.y)));
		double mid = sqrt(static_cast<double>((a.x - p.x)*(a.x - p.x)
				+ (a.y - p.y)*(a.y-p.y)));
		distance_along = mid / length;

		return TRUE;
	}

	return FALSE;
}

// Given a ratio of how far along the line to move,
// determine the point
CDPoint CLineUtils::GetPointOnLine( double distance_along, TransformSnap *snap  ) const
{
	CDPoint r;

	if (distance_along <= 0.0)
	{
		return a;
	}
	if (distance_along >= 1.0)
	{
		return b;
	}

	r.x = (b.x - a.x) * distance_along + a.x;
	r.y = (b.y - a.y) * distance_along + a.y;


	return r;
}


// Work out whether a line is inside or passes through an area:-
//
//                    xxx1     xxxx     xx1x   
//            
// x1xx               1001  |  1000  |  1010
//                    ------+--------+------
// xxxx               0001  |  0000  |  0010
//                    ------+--------+------
// 1xxx               0101  |  0100  |  0110
//
int CLineUtils::WhichArea(CDPoint p,double left,double right,double top,double bottom) const
{
  int r=0;

  if (p.x<left)
	r |=1;
  else if (p.x>right)
	r |= 2;

  if (p.y>bottom)
	r |= 4;
  else if (p.y<top)
	r |= 8;

  return r;
}


BOOL CLineUtils::rIsInside(CDPoint p1,CDPoint p2,double left,double right,double top,double bottom) const
{
  {
	int aa = WhichArea(p1,left,right,top,bottom);
	int ab = WhichArea(p2,left,right,top,bottom);

	// Is it a trivial case?
	if (aa==0 || ab==0)
		return TRUE;
	if ((aa & ab)!=0)
		return FALSE;
  }

  // Cannot be accepted or rejected, so divide in two and try again!
  {
	CDPoint midpoint;
	midpoint.x=(p1.x+p2.x) / 2.0;
	midpoint.y=(p1.y+p2.y) / 2.0;
	return (midpoint != p2 && rIsInside(p1,midpoint,left,right,top,bottom)) || (midpoint != p1 && rIsInside(midpoint,p2,left,right,top,bottom));
  }
}


// Get the length of the line
double CLineUtils::GetLength()
{
	return sqrt( static_cast<double>((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y)) );
}



// Split this line up into rectangles for
// display, without redrawing too large an area...
void CLineUtils::SplitForDisplay( CTinyCadDoc *pDesign, BOOL erase, int grow )
{
	// Horizontal and vertical lines are easy...
	if (a.x == b.x || a.y == b.y)
	{
		CDRect r(a.x,a.y,b.x,b.y);
		pDesign->InvalidateRect( r, erase, grow );
	}

	int rect_size = 20;

	double split_x = fabs(a.x - b.x) / rect_size;
	double split_y = fabs(a.y - b.y) / rect_size;
	int split = static_cast<int>(max( split_y, split_x ));
	split = max(1,split);

	double x = 0;
	double y = 0;

	// Move along the x axis...
	CDPoint p1,p2;
	p1 = a;
	for (int i= 1; i <= split; i++)
	{
		if (i == split)
		{
			p2 = b;
		}
		else
		{
			p2 = GetPointOnLine( static_cast<double>(i) / split, NULL );
		}
		CDRect r(p1.x,p1.y,p2.x,p2.y);
		pDesign->InvalidateRect( r, erase, grow );
		p1 = p2;
	}
}

