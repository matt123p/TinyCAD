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

// The constructor
TransformSnap::TransformSnap()
{
	SetAccurateGrid(NormalGrid);
	SetGridSnap(TRUE);
}

// Snap an int to the grid
double TransformSnap::Snap(double d)
{
	if (!GridSnap || grid == 0) return d;

	double a = floor(d / grid) * grid;
	if ( (d - a) * 2 > grid) a += grid;

	return a;
}

////// The Scaling operation for translation, rotation and mirroring //////

// The Constructor
Transform::Transform()
{
	SetOriginX(-1);
	SetOriginY(-1);
	SetZoomFactor(1.0);
	rotmir = 0;

	IsYUp = FALSE;
	BoundY = 0;

	scaling_x = 1.0;
	scaling_y = 1.0;
	pixel_offset = CPoint(0, 0);

}

// Efficient round double to nearest integer.
// static_cast<int> rounds to the nearest integer towards zero.
inline int Transform::Round(double d) const
{
	if (d > -0.5)
	{
		return static_cast<int> (d + 0.5);
	}
	else
	{
		return static_cast<int> (d - 0.5);
	}
}

inline int Transform::ScaleX(double a) const
{
	return Round(doubleScale(a * scaling_x - m_x)) + pixel_offset.x;
}

inline int Transform::ScaleY(double a) const
{
	return (IsYUp ? 
			Round(-doubleScale(a * scaling_y - m_y)) - BoundY : 
			Round(doubleScale(a * scaling_y - m_y)))
		+ pixel_offset.y; 
}

inline int Transform::Scale(double dist) const
{
	return ScaleX(dist);
}

inline double Transform::doubleScale(double a) const
{
	return a * m_zoom;
}

inline double Transform::GetZoomPixelScale() const
{
	return 1;
}

CDPoint Transform::SetTRM(CDPoint os, CDPoint bound, int rotate)
{
	CDPoint old = CDPoint(m_x, m_y);

	m_x -= os.x;
	m_y -= os.y;
	rotmir = rotate;

	// Calculate the offset given the rotation and bounding box
	switch (rotmir & 3)
	{
		case 1: // Down
			m_y -= bound.y;
			break;
		case 3: // Right
			m_x -= bound.y;
			break;
	}
	if ( (rotmir & 4) != 0)
	{
		switch (rotmir & 3)
		{
			case 0: // Up
			case 1: // Down
				m_x -= bound.x;
				break;
			case 2: // Left
			case 3: // Right
				m_y -= bound.x;
				break;
		}
	}

	return old;
}

void Transform::SetScaling(int rot, double x, double y)
{

	switch (rot & 3)
	{
		case 0: // Up
		case 1: // Down
			scaling_x = x;
			scaling_y = y;
			break;
		case 2: // Left
		case 3: // Right
			scaling_x = y;
			scaling_y = x;
			break;
	}
}

void Transform::GetScaling(int rot, double &x, double &y) const
{
	switch (rot & 3)
	{
		case 0: // Up
		case 1: // Down
			x = scaling_x;
			y = scaling_y;
			break;
		case 2: // Left
		case 3: // Right
			y = scaling_x;
			x = scaling_y;
			break;
	}
}

void Transform::EndTRM(CDPoint os)
{
	rotmir = 0;
	m_x = os.x;
	m_y = os.y;
}

// Scale with rotation and mirroring
CPoint Transform::Scale(CDPoint p) const
{

	// Do we mirror?
	if ( (rotmir & 4) != 0)
	{
		p = CDPoint(-p.x, p.y);
	}

	switch (rotmir & 3)
	{
		case 1: // Down
			p = CDPoint(p.x, -p.y);
			break;
		case 2: // Left
			p = CDPoint(p.y, p.x);
			break;
		case 3: // Right
			p = CDPoint(-p.y, p.x);
			break;
	}

	return CPoint(ScaleX(p.x), ScaleY(p.y));
}

double Transform::doubleDeScale(LONG a) const
{
	return a / m_zoom;
}

double Transform::DeScale(double a) const
{
	return a / m_zoom;
}

// Rotate a direction (for power, pins etc)
int Transform::RotateDir(int in) const
{

	// Current rotation=> 0  1  2  3
	// Current dir ..						  \/..
	int table[] = {0, 1, 2, 3, // 0
	               1, 0, 3, 2, // 1
	               2, 2, 0, 0, // 2
	               3, 3, 1, 1, // 3
	        };

	// Mirror if necessary
	if ( (rotmir & 4) != 0)
	{
		if (in == 3) in = 2;
		else if (in == 2) in = 3;
	}

	// Now rotate
	in = table[ (rotmir & 3) + (in & 3) * 4];

	return in;
}

void Transform::SetOriginX(double NewX)
{
	m_x = NewX;
}

void Transform::SetOriginY(double NewY)
{
	m_y = NewY;
}

void Transform::SetOrigin(CDPoint p)
{
	m_x = p.x;
	m_y = p.y;
}

void Transform::SetYisup(int newBoundY)
{
	IsYUp = TRUE;
	BoundY = newBoundY;
}

void Transform::SetPixelOffset(CPoint &p)
{
	pixel_offset = p;
}

CPoint Transform::GetPixelOffset() const
{
	return pixel_offset;
}

CDPoint Transform::GetOrigin() const
{
	return CDPoint(m_x, m_y);
}

CDPoint Transform::GetScaledOrigin() const
{
	return CDPoint(doubleScale(m_x), doubleScale(m_y));
}

void Transform::SetZoomFactor(double zoom)
{
	m_zoom = zoom;
}

double Transform::GetZoomFactor() const
{
	return m_zoom;
}

BOOL Transform::GetIsYUp() const
{
	return IsYUp;
}

int Transform::GetRotMir() const
{
	return rotmir;
}

CRect Transform::Scale(CDRect r) const
{
	CPoint a = Scale(CDPoint(r.left, r.top));
	CPoint b = Scale(CDPoint(r.right, r.bottom));
	return CRect(a.x, a.y, b.x, b.y);
}

// Convert from window coords into internal coords
CDPoint Transform::DeScale(TransformSnap &s, CPoint p) const
{
	return CDPoint(DeScaleX(s, p.x), DeScaleY(s, p.y));
}

double Transform::DeScaleX(TransformSnap &s, LONG a) const
{
	return s.Snap(DeScaleX(a));
}

double Transform::DeScaleY(TransformSnap &s, LONG a) const
{
	return s.Snap(DeScaleY(a));
}

CDPoint Transform::DeScale(CPoint p) const
{
	return CDPoint(DeScaleX(p.x), DeScaleY(p.y));
}

double Transform::DeScaleX(LONG a) const
{
	return (static_cast<double> (a) / m_zoom) + m_x;
}

double Transform::DeScaleY(LONG a) const
{
	return (static_cast<double> (a) / m_zoom) + m_y;
}

