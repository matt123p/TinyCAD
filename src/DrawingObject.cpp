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
#include "StreamMemory.h"
#include "net.h"

// The editing dialogs
extern CDlgERCListBox theERCListBox;

////////// The default actions for each object ///////////


CDrawingObject::CDrawingObject(CTinyCadDoc *pDesign)
{
	m_pDesign = pDesign;
	m_point_a = CDPoint(0, 0);
	m_point_b = CDPoint(0, 0);
	m_segment = 0;
}

BOOL CDrawingObject::IsConstruction()
{
	return FALSE;
}

BOOL CDrawingObject::IsEmpty()
{
	return FALSE;
}

// If the user holds down ctrl, can should we fix the aspect ratio?
BOOL CDrawingObject::CanControlAspect()
{
	return TRUE;
}

void CDrawingObject::LButtonUp(CDPoint, CDPoint)
{

}

// Is a point pointing at one of our fields?
int CDrawingObject::IsInsideField(CDPoint)
{
	return -1;
}

void CDrawingObject::MoveField(int w, CDPoint r)
{
	// Do nothing
}

// Set the new layer when the options change
void CDrawingObject::NewOptions()
{
}

void CDrawingObject::TagResources()
{

}

// Read the CPoints from a file (for Win16 compatibility)
CDPoint CDrawingObject::ReadPoint(CStream &theFile)
{
	CPoint r;

	theFile >> r;

	return CDPoint(r.x, r.y);
}

// The user has double clicked with the left hand button
void CDrawingObject::DblLButtonDown(CDPoint, CDPoint)
{
}

// Place the object into the design
void CDrawingObject::LButtonDown(CDPoint p, CDPoint)
{
	// New undo level for each placement...
	m_pDesign->BeginNewChangeSet();

	if (m_segment)
	{
		m_point_a = p;
		m_point_b = p;
		m_segment = !m_segment;
		Display();
	}
	else
	{
		m_point_b = p;
		Display();
		Store();
		m_point_a = m_point_b;
	}
}

// Find a string in this object
CString CDrawingObject::Find(const TCHAR *)
{
	return "";
}

// Move the object about the design
void CDrawingObject::Move(CDPoint p, CDPoint no_snap_p)
{
	if (!m_segment)
	{
		Display();
		m_point_b = p;
		Display();
	}
}

// Cancel editing this part of the object in the design
BOOL CDrawingObject::RButtonDown(CDPoint p, CDPoint s)
{
	// If necessary remove the line from the screen
	Display();

	// Signal this is the end of this object
	return FALSE;
}

void CDrawingObject::RButtonUp(CDPoint, CDPoint)
{

}

void CDrawingObject::BeginEdit(BOOL re_edit)
{
}

void CDrawingObject::EndEdit()
{
	Display();
}

void CDrawingObject::NotifyEdit(int action)
{
}

BOOL CDrawingObject::CanEdit()
{
	return FALSE;
}

ObjType CDrawingObject::GetType()
{
	return xAnnotation;
}

CString CDrawingObject::GetName() const
{
	return "??";
}

void CDrawingObject::Display(BOOL erase)
{
	if (m_point_a != m_point_b)
	{
		CDRect r(m_point_a.x, m_point_a.y, m_point_b.x, m_point_b.y);
		m_pDesign->InvalidateRect(r, erase, 0);
	}
}

void CDrawingObject::Paint(CContext &dc, paint_options options)
{
}

void CDrawingObject::PaintHandles(CContext &)
{
}

CDrawingObject* CDrawingObject::Store()
{
	return NULL;
}

double CDrawingObject::DistanceFromPoint(CDPoint p)
{
	return IsInside(p.x, p.x, p.y, p.y) ? 0 : 100.0;
}

BOOL CDrawingObject::IsInside(double left, double right, double top, double bottom)
{
  return !( (m_point_a.x<left && m_point_b.x<=left) || (m_point_a.x>right && m_point_b.x>=right)
        || (m_point_a.y<top && m_point_b.y<=top) || (m_point_a.y>bottom && m_point_b.y>=bottom));
}

BOOL CDrawingObject::IsCompletelyInside(double left, double right, double top, double bottom)
{
  return (left <= min(m_point_a.x, m_point_b.x) && right >= max(m_point_b.x, m_point_a.x)
		&& top <= min(m_point_a.y, m_point_b.y) && bottom >= max(m_point_b.y, m_point_a.y));
}

// Load the object to a file
void CDrawingObject::Load(CStream& archive)
{
	m_point_a = ReadPoint(archive);
	m_point_b = ReadPoint(archive);
}

// Load and save to an XML file
void CDrawingObject::SaveXML(CXMLWriter &xml)
{
}

void CDrawingObject::LoadXML(CXMLReader &xml)
{
}

// Rotate this object about a point
void CDrawingObject::Rotate(CDPoint p, int dir)
{
	// Translate this point so the rotational point is the origin
	m_point_a = CDPoint(m_point_a.x - p.x, m_point_a.y - p.y);
	m_point_b = CDPoint(m_point_b.x - p.x, m_point_b.y - p.y);

	// Perfrom the rotation
	switch (dir)
	{
		case 2: // Left
			m_point_a = CDPoint(m_point_a.y, -m_point_a.x);
			m_point_b = CDPoint(m_point_b.y, -m_point_b.x);
			break;
		case 3: // Right
			m_point_a = CDPoint(-m_point_a.y, m_point_a.x);
			m_point_b = CDPoint(-m_point_b.y, m_point_b.x);
			break;
		case 4: // Mirror
			m_point_a = CDPoint(-m_point_a.x, m_point_a.y);
			m_point_b = CDPoint(-m_point_b.x, m_point_b.y);
			break;
	}

	// Re-translate the points back to the original location
	m_point_a = CDPoint(m_point_a.x + p.x, m_point_a.y + p.y);
	m_point_b = CDPoint(m_point_b.x + p.x, m_point_b.y + p.y);

}

int CDrawingObject::SetCursor(CDPoint p)
{
	return -1;
}

int CDrawingObject::SetCursorEdit(CDPoint p)
{
	BOOL r = IsInside(p.x, p.x, p.y, p.y);
	return r ? 11 : -1;
}

void CDrawingObject::ContextMenu(CDPoint p, UINT id)
{
}

// For the context menu
void CDrawingObject::FinishDrawing(CDPoint p)
{
}

int CDrawingObject::GetContextMenu()
{
	return IDR_EDITTOOL;
}

// Extract the netlist/active points from this object
void CDrawingObject::GetActiveListFirst(CActiveNode &a)
{
}

bool CDrawingObject::GetActive(CActiveNode &a)
{
	return false;
}

// Move the object by the relative displacement r
void CDrawingObject::Shift(CDPoint r)
{
	m_point_a += r;
	m_point_b += r;
}

// Compare two objects for equality
bool CDrawingObject::operator==(const CDrawingObject &obj) const
{
	// Cheap test for unequality
	if (m_point_a != obj.m_point_a || m_point_b != obj.m_point_b)
	{
		return false;
	}

	// Compare object contents
	CStreamMemory stream1;
	CStreamMemory stream2;
	CXMLWriter xml1(&stream1);
	CXMLWriter xml2(&stream2);
	// ('->' will call the virtual SaveXML function, '.' would always call the CDrawingObject::SaveXML)
	((CDrawingObject*) this)->SaveXML(xml1);
	((CDrawingObject &) obj).SaveXML(xml2);

	// Are stored contents are equal?
	return (stream1 == stream2);
}

bool CDrawingObject::operator!=(const CDrawingObject &obj) const
{
	return ! (*this == obj);
}
