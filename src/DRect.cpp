/*
 TinyCAD program for schematic capture
 Copyright 1994/1995/2002-2005 Matt Pyne.

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
#include "math.h"

#include ".\drect.h"

CDRect::CDRect(void)
{
}

// from left, top, right, and bottom
CDRect::CDRect(double l, double t, double r, double b)
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}

// Attributes (in addition to RECT members)

// retrieves the width
double CDRect::Width() const
{
	return right - left;
}

// returns the height
double CDRect::Height() const
{
	return bottom - top;
}

// returns TRUE if point is within rectangle
BOOL CDRect::PtInRect(CDPoint point) const
{
	CDRect r(*this);
	r.NormalizeRect();
	return ! (point.x < r.left || point.x > r.right || point.y < r.top || point.y > r.bottom);
}

// Inflate rectangle's width and height by
// size.cx units to the left and right ends of the rectangle
// and size.cy units to the top and bottom.
void CDRect::InflateRect(CDSize size)
{
	left -= size.cx;
	right += size.cx;
	top -= size.cy;
	bottom += size.cy;
}

void CDRect::InflateRect(double l, double t, double r, double b)
{	//Note:  This operation can result in a denormalized rectangle when inflating a 
	//rectangle by a negative size that exceeds the width or height of the original rectangle.
	left -= l;
	right += r;
	top -= t;
	bottom += b;
}

void CDRect::NormalizeRect()
{
	if (right < left)
	{
		double s = right;
		right = left;
		left = s;
	}
	if (bottom < top)
	{
		double s = bottom;
		bottom = top;
		top = s;
	}
}

//Detect if a rectangle is in unnormalized form - useful when reducing 
//the size of very small rectangles when the reduction results in the
//creation of a denormalized rectangle
BOOL CDRect::IsNormalized()
{
	if (right < left)
	{
		return FALSE;
	}
	if (bottom < top)
	{
		return FALSE;
	}
	return TRUE;
}

// Operators returning CRect values
CDRect CDRect::operator+(CDPoint point) const
{
	CDRect r;
	r.top += point.y;
	r.bottom += point.y;
	r.left += point.x;
	r.right += point.x;

	return r;
}

CDRect CDRect::operator-(CDPoint point) const
{
	CDRect r;
	r.top -= point.y;
	r.bottom -= point.y;
	r.left -= point.x;
	r.right -= point.x;

	return r;
}

CDRect CDRect::operator+=(CDPoint point)
{
	top += point.y;
	bottom += point.y;
	left += point.x;
	right += point.x;

	return *this;
}

