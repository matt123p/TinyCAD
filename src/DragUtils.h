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

// DragUtils.h: interface for the CDragUtils class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRAGUTILS_H__E6C77B92_5C1E_4F7B_8367_6349A36C01D5__INCLUDED_)
#define AFX_DRAGUTILS_H__E6C77B92_5C1E_4F7B_8367_6349A36C01D5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "JunctionUtils.h"

class CDrawingObject;
class CTinyCadDoc;

class CDragUtils
{
protected:
	struct dragWire
	{
		BOOL m_MoveA;
		BOOL m_MoveB;
		CDrawingObject* m_Object;

		CDrawingObject* m_Attached_LineA;
		double m_Distance_AlongA;

		CDrawingObject* m_Attached_LineB;
		double m_Distance_AlongB;

		BOOL m_Done_A;
		BOOL m_Done_B;

		BOOL m_Lock_Horiz;
		BOOL m_Lock_Vert;

		dragWire(CDrawingObject *o, BOOL MoveA = FALSE, BOOL MoveB = FALSE)
		{
			m_Object = o;
			m_MoveA = MoveA;
			m_MoveB = MoveB;
			m_Attached_LineA = NULL;
			m_Distance_AlongA = 0.0;
			m_Attached_LineB = NULL;
			m_Distance_AlongB = 0.0;
			m_Done_A = FALSE;
			m_Done_B = FALSE;
			m_Lock_Horiz = o->m_point_a.y == o->m_point_b.y;
			m_Lock_Vert = o->m_point_a.x == o->m_point_b.x;
		}

		void Lock()
		{
			if (m_Lock_Horiz)
			{
				if (!m_Done_B)
				{
					m_Object->m_point_b.y = m_Object->m_point_a.y;
					m_Done_B = TRUE;
				}
				if (!m_Done_A)
				{
					m_Object->m_point_a.y = m_Object->m_point_b.y;
					m_Done_A = TRUE;
				}
			}

			if (m_Lock_Vert)
			{
				if (!m_Done_B)
				{
					m_Object->m_point_b.x = m_Object->m_point_a.x;
					m_Done_B = TRUE;
				}
				if (!m_Done_A)
				{
					m_Object->m_point_a.x = m_Object->m_point_b.x;
					m_Done_A = TRUE;
				}
			}
		}

		bool operator<(const dragWire &b) const
		{
			return m_Object < b.m_Object;
		}

		bool operator==(const dragWire &b) const
		{
			return m_Object == b.m_Object;
		}
	};

	typedef std::vector<dragWire> wireCollection;
	wireCollection m_draggedWires;

	typedef std::set<CDrawingObject *> discardCollection;
	discardCollection m_discards;

	void AddWiresAtPoint(CDPoint p);
	void AddWireToCollection(dragWire n);
	void AddAttachedObjects(CDrawingObject *wire, BOOL wire_move_a, BOOL wire_move_b);
	void MoveAttachedObjects(CDrawingObject *wire);
	void DisplayDraggedWires();

	// Our current design that this object manipulates
	CTinyCadDoc *m_pDesign;

	// Have we started?
	BOOL m_started;

	// The stored begin data
	CDPoint m_a;
	CDPoint m_b;

	void Begin();
	void Clear();
	void Merge();
	void Clean();

	// Our junction utils
	CJunctionUtils m_j;

public:
	CDragUtils& operator=(const CDragUtils &o)
	{
		// Don't do this!
		ASSERT( FALSE);
		return *this;
	}

	CDragUtils(CTinyCadDoc* pDesign);
	virtual ~CDragUtils();

	void Begin(CDPoint a, CDPoint b);
	void Drag(CDPoint r);
	void Move(CDPoint r);
	void End(bool no_clean);
	void MergeLinePoint(CDrawingObject *p);

	BOOL Started()
	{
		return m_started;
	}
};

#endif // !defined(AFX_DRAGUTILS_H__E6C77B92_5C1E_4F7B_8367_6349A36C01D5__INCLUDED_)
