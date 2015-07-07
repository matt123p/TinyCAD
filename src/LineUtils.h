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

// LineUtils.h: interface for the CLineUtils class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LINEUTILS_H__7B352AE0_AB93_41F9_B5CB_BB14CDCAB472__INCLUDED_)
#define AFX_LINEUTILS_H__7B352AE0_AB93_41F9_B5CB_BB14CDCAB472__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class TransformSnap;
class CTinyCadDoc;

class CLineUtils
{
protected:
	CDPoint a;
	CDPoint b;

	int WhichArea(CDPoint p, double left, double right, double top, double bottom) const;
	BOOL rIsInside(CDPoint p1, CDPoint p2, double left, double right, double top, double bottom) const;

public:
	CLineUtils(CDPoint ai, CDPoint bi);
	virtual ~CLineUtils();

	// Is the line inside or crosses the rectangle given?
	BOOL IsInside(double left, double right, double top, double bottom) const;

	// Determine how far a point lies from this line
	double DistanceFromPoint(CDPoint p, CDPoint &d) const;

	// Determine how far a point lies from this line along the X axis
	double DistanceFromPointX(CDPoint p, CDPoint &d) const;

	// Determine how far a point lies from this line along the Y axis
	double DistanceFromPointY(CDPoint p, CDPoint &d) const;

	// Determine if a point lies on the line
	BOOL IsPointOnLine(CDPoint p, double &distance_along) const;

	// Given a ratio of how far along the line to move,
	// determine the point
	CDPoint GetPointOnLine(double distance_along, TransformSnap *snap) const;

	// Split this line up into rectangles for
	// display, without redrawing too large an area...
	void SplitForDisplay(CTinyCadDoc *pDesign, BOOL erase, int grow);

	// Determine if the line is diagonal
	BOOL IsDiagonal() const;

	// Get the length of the line
	double GetLength();

	static double pointOnLineDistance;
};

#endif // !defined(AFX_LINEUTILS_H__7B352AE0_AB93_41F9_B5CB_BB14CDCAB472__INCLUDED_)
