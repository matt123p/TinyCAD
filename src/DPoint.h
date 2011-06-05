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

#ifndef __DPOINT_H__
#define __DPOINT_H__

#include "revision.h"
#include <limits>
// Use the C header for _isnan() function
// because Microsoft doesn't supply the C++ std::isnan() function 
#include <float.h>

class CDPoint
{
public:
	double x;
	double y;

	static double makeXMLUnits(int a)
	{
		return makeXMLUnits(static_cast<double> (a));
	}

	static double makeXMLUnits(double a)
	{
		return a / static_cast<double> (PIXELSPERMM);
	}

	static double unmakeXMLUnits(double a)
	{
		return a * static_cast<double> (PIXELSPERMM);
	}

	bool hasValue() const
	{
		return ! (_isnan(x) || _isnan(y));
	}

	void ForceLargerSize() {
		if (x > y) y = x;
		else x = y;
	}

	void ForceMinSize(double size) {
		if (x < size) x = size;
		if (y < size) y = size;
	}

	CDPoint()
	{
		// Initialize with not-a-number
		x = std::numeric_limits<long double>::quiet_NaN();
		y = std::numeric_limits<long double>::quiet_NaN();
	}

	CDPoint(double ix, double iy)
	{
		x = ix;
		y = iy;
	}

	CDPoint operator+(CDPoint a)
	{
		CDPoint r(*this);
		r.x += a.x;
		r.y += a.y;
		return r;
	}

	CDPoint& operator+=(CDPoint a)
	{
		x += a.x;
		y += a.y;
		return *this;
	}

	CDPoint& operator-=(CDPoint a)
	{
		x -= a.x;
		y -= a.y;
		return *this;
	}

	CDPoint operator+(CPoint a)
	{
		CDPoint r(*this);
		r.x += a.x;
		r.y += a.y;
		return r;
	}
	CDPoint operator-(CPoint a)
	{
		CDPoint r(*this);
		r.x -= a.x;
		r.y -= a.y;
		return r;
	}
	CDPoint operator-(CDPoint a)
	{
		CDPoint r(*this);
		r.x -= a.x;
		r.y -= a.y;
		return r;
	}

	double Distance(CDPoint a) const
	{
		return sqrt( (a.x - x) * (a.x - x) + (a.y - y) * (a.y - y));
	}

	bool is_close_to(const CDPoint &a) const
	{
		const double close_distance = 0.001;
		return fabs(a.x - x) < close_distance && fabs(a.y - y) < close_distance;
	}

	bool operator==(const CDPoint &a) const
	{
		return is_close_to(a); // a.x == x && a.y == y;
	}

	bool operator!=(const CDPoint &a) const
	{
		return ! (*this == a);
	}

	bool operator<(const CDPoint &a) const
	{
		const double close_distance = 0.001;
		if (fabs(a.x - x) < close_distance)
		{
			return y < a.y - close_distance;
		}
		else
		{
			return x < a.x - close_distance;
		}
	}
};

typedef std::vector<CDPoint> pointCollection;

#endif
