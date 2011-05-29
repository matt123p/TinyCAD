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
#include "TinyCadView.h"
#include "diag.h"
#include "colour.h"

// This defines the block edit functions

// The rectangle area slightly re-written
#define theArea CDRect(m_point_a.x,m_point_a.y,m_point_b.x,m_point_b.y)

////// The rotate move object //////

void CDrawBlockRotate::Paint(CContext &dc, paint_options options)
{
	// Draw marquee
	dc.SelectBrush();
	if (m_point_a.x < m_point_b.x)
	{
		dc.SelectPen(PS_MARQUEE, 1, cBLOCK);
	}
	else
	{
		// Other marquee pen for right to left selection
		dc.SelectPen(PS_MARQUEE2, 1, cBLOCK);
	}

	dc.SetROP2(R2_COPYPEN);
	dc.Rectangle(theArea);
}

CDrawBlockRotate::CDrawBlockRotate(CTinyCadDoc *pDesign) :
	CDrawingObject(pDesign)
{
	m_segment = 1;
	placed = 0;
	m_point_a = m_point_b = CDPoint(0, 0);
}

CString CDrawBlockRotate::GetName() const
{
	return "Block Rotate";
}

void CDrawBlockRotate::BeginEdit(BOOL re_edit)
{
}

int CDrawBlockRotate::SetCursor(CDPoint p)
{
	return placed ? -1 : 12;
}

void CDrawBlockRotate::EndEdit()
{
	if (placed) g_EditToolBar.m_RotateBox.Close();

	if (!m_segment | placed)
	{
		m_pDesign->UnSelect();
		Display();
		m_segment = 1;
		placed = 0;
	}
}

void CDrawBlockRotate::LButtonDown(CDPoint p, CDPoint)
{
	// New undo level for each placement...
	m_pDesign->BeginNewChangeSet();

	if (placed)
	{
		Display();
		m_pDesign->UnSelect();
		g_EditToolBar.m_RotateBox.Close();
		m_pDesign->SetModifiedFlag(TRUE);
		m_segment = 1;
		placed = 0;
		m_point_a = m_point_b;
	}
	else if (m_segment)
	{
		m_point_a = p;
		m_point_b = p;
		m_segment = 0;
		placed = 0;
		Display();
	}
	else
	{
		Display();
		m_point_b = p;
		placed = 1;
		m_segment = 0;
		// Now select all the objects in this rectangle
		m_pDesign->Select(m_point_a, m_point_b);
		g_EditToolBar.m_RotateBox.Open(m_pDesign, this);

		// Build the Undo list
		m_pDesign->SelectUndoMove(CDPoint(0, 0));

		Display();
	}
}

void CDrawBlockRotate::Move(CDPoint p, CDPoint no_snap_p)
{
	if (!placed && !m_segment)
	{
		Display();
		m_point_b = p;
		Display();
	}
}

void CDrawBlockRotate::ChangeDir(int NewDir)
{
	if (!m_pDesign->IsSelected()) return;

	// Find the centre of rotation
	CDPoint centre = m_pDesign->m_snap.Snap(CDPoint( (m_point_b.x - m_point_a.x) / 2 + m_point_a.x, (m_point_b.y - m_point_a.y) / 2 + m_point_a.y));

	Display();
	CJunctionUtils j(m_pDesign);

	selectIterator it = m_pDesign->GetSelectBegin();
	while (it != m_pDesign->GetSelectEnd())
	{
		CDrawingObject *obj = *it;

		j.AddObjectToTodo(obj);
		obj->Rotate(centre, NewDir);
		j.AddObjectToTodo(obj);

		++it;
	}

	j.CheckTodoList(false);

	Rotate(centre, NewDir);

	Display();

	m_pDesign->Invalidate();
}

BOOL CDrawBlockRotate::RButtonDown(CDPoint p, CDPoint s)
{
	BOOL r = m_pDesign->IsSelected();

	// Cancel the rotate operation
	if (placed)
	{
		g_EditToolBar.m_RotateBox.Close();
		m_point_a = m_point_b;
	}

	// If necessary remove the line from the screen
	if (!m_segment | placed)
	{
		Display();
		m_pDesign->UnSelect();
		m_segment = 1;
		placed = 0;
	}

	return r;
}

