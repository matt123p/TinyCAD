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

#ifndef __ARCPOINT_H__
#define __ARCPOINT_H__

#include "DPoint.h"
#include "float.h"

class CArcPoint
{
public:
	double x;
	double y;

	int flatterned_segment;

	enum arc_type
	{
		Arc_none, 
		Arc_in, 
		Arc_out
	};
	arc_type arc;

	CArcPoint()
	{
		x = 0;
		y = 0;
		arc = Arc_none;
		flatterned_segment = 0;
	}
	CArcPoint(double xi, double yi, arc_type arci = Arc_none)
	{
		ASSERT(_finite(xi));
		ASSERT(_finite(yi));
		x = xi;
		y = yi;
		arc = arci;
		flatterned_segment = 0;
	}
	CArcPoint(const CPoint &p)
	{
		x = p.x;
		y = p.y;
		arc = Arc_none;
		flatterned_segment = 0;
	}

	CArcPoint(const CDPoint &p)
	{
		ASSERT(_finite(p.x));
		ASSERT(_finite(p.y));
		x = p.x;
		y = p.y;
		arc = Arc_none;
		flatterned_segment = 0;
	}

	void ReadPoint(CStream& archive)
	{
		int q, ix, iy;
		archive >> ix >> iy >> q;
		x = ix;
		y = iy;
		arc = static_cast<arc_type> (q);
	}

	CArcPoint operator+(const CArcPoint &b) const
	{
		ASSERT(_finite(b.x));
		ASSERT(_finite(b.y));

		CArcPoint r(*this);
		r.x += b.x;
		r.y += b.y;
		return r;
	}
	CArcPoint operator-(const CArcPoint &b) const
	{
		ASSERT(_finite(b.x));
		ASSERT(_finite(b.y));

		CArcPoint r(*this);
		r.x -= b.x;
		r.y -= b.y;
		return r;
	}

	CArcPoint& operator=(const CArcPoint &b)
	{
		ASSERT(_finite(b.x));
		ASSERT(_finite(b.y));
		x = b.x;
		y = b.y;
		return *this;
	}

	bool operator==(const CArcPoint &b) const
	{
		ASSERT(_finite(b.x));
		ASSERT(_finite(b.y));
		ASSERT(_finite(x));
		ASSERT(_finite(y));

		return x == b.x && y == b.y;
	}

	bool operator!=(const CArcPoint &b) const
	{
		ASSERT(_finite(b.x));
		ASSERT(_finite(b.y));
		ASSERT(_finite(x));
		ASSERT(_finite(y));
		return ! (*this == b);
	}
};

typedef std::vector<CArcPoint> arcpointCollection;

#endif
