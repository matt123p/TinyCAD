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

// JunctionUtils.h: interface for the CJunctionUtils class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JUNCTIONUTILS_H__C27DE585_CD2B_4C43_9CAB_513AC3D15E15__INCLUDED_)
#define AFX_JUNCTIONUTILS_H__C27DE585_CD2B_4C43_9CAB_513AC3D15E15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class CTinyCadDoc;

#include "Object.h"

class CJunctionUtils
{
public:

	// This is used during moving objects
	struct todo_point
	{
		CDPoint a;
		CDPoint b;

		todo_point(CDPoint ai, CDPoint bi)
		{
			a = ai;
			b = bi;
		}

		todo_point(CDPoint ai)
		{
			a = ai;
			b = ai;
		}

		static bool comparePoint(const CDPoint a, const CDPoint b)
		{
			if (a.x == b.x)
			{
				return a.y < b.y;
			}
			else
			{
				return a.x < b.x;
			}
		}

		bool operator<(const todo_point p) const
		{
			if (p.a == a)
			{
				return comparePoint(b, p.b);
			}
			else
			{
				return comparePoint(a, p.a);
			}
		}
	};
protected:
	typedef std::set<todo_point> todoList;
	todoList m_todo;

	CTinyCadDoc* m_pDesign;

	typedef std::set<CDrawingObject *> discardCollection;
	discardCollection m_discards;

public:

	// Check to see if a junction is required on a line
	void CheckJunctionRequirement(CDPoint a, CDPoint b, bool perform_split);

	// Check to see if a junction is required at the point specified...
	void CheckJunctionRequirement(CDPoint q, bool perform_split);

	// Add an object to the todo list
	void AddObjectToTodo(CDrawingObject *obj);
	void CheckTodoList(bool perform_split);

	// Paint the junctions in the todo list
	void PaintJunctions(CContext &dc, paint_options opt);

	CJunctionUtils(CTinyCadDoc* pDesign);
	virtual ~CJunctionUtils();

};

#endif // !defined(AFX_JUNCTIONUTILS_H__C27DE585_CD2B_4C43_9CAB_513AC3D15E15__INCLUDED_)
