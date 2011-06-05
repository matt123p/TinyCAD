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

#pragma once

#include "DPoint.h"
#include "DSize.h"

class CDRect
{
public:
	double left;
	double top;
	double right;
	double bottom;

	// uninitialized rectangle
	CDRect();
	// from left, top, right, and bottom
	CDRect(double l, double t, double r, double b);

	// Attributes (in addition to RECT members)

	// retrieves the width
	double Width() const ;
	// returns the height
	double Height() const ;

	// returns TRUE if point is within rectangle
	BOOL PtInRect(CDPoint point) const ;

	// Inflate rectangle's width and height by
	// size.cx units to the left and right ends of the rectangle
	// and size.cy units to the top and bottom.
	// For deflation, use negative size value.
	void InflateRect(CDSize size);
	void InflateRect(double l, double t, double r, double b);

	void NormalizeRect();

	// set this rectangle to intersection of two others
	// BOOL IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2) ;

	// Operators returning CRect values
	CDRect operator+(CDPoint point) const ;
	CDRect operator-(CDPoint point) const ;
	CDRect operator+=(CDPoint point);
};
