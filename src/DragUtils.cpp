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

// DragUtils.cpp: implementation of the CDragUtils class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tinycad.h"
#include "TinyCadDoc.h"
#include "Object.h"
#include "DragUtils.h"
#include "LineUtils.h"
#include "JunctionUtils.h"
#include <algorithm> // for the find function


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDragUtils::CDragUtils(CTinyCadDoc* pDesign )
: m_j(pDesign)
{
	m_pDesign = pDesign;
	m_started = FALSE;

	// The stored begin data
	m_a = CDPoint(0,0);
	m_b = CDPoint(0,0);
}

CDragUtils::~CDragUtils()
{
	Clear();
}


void CDragUtils::Clear()
{
	// Now clear out our variables
	m_started = FALSE;
	m_discards.erase( m_discards.begin(), m_discards.end() );
	m_draggedWires.erase( m_draggedWires.begin(), m_draggedWires.end() );
}


// Find any wires that are horizontal or vertical and are joined to
// a wire in the same orientation.  Then use that to merge the wires
// together
//
void CDragUtils::Merge()
{
	// Look for any wires that are at a particular point...
	wireCollection::iterator it_wires = m_draggedWires.begin();
	while (it_wires != m_draggedWires.end())
	{
		CDrawingObject *pointer = (*it_wires).m_Object;
	
		// Is this a zero length wire?
		if (pointer->GetType() == xWire && m_discards.find( pointer ) == m_discards.end())
		{
			// Is this wire horizontal or vertical?
			bool vert = pointer->m_point_a.x == pointer->m_point_b.x;
			bool horiz = pointer->m_point_a.y == pointer->m_point_b.y;

			// if (vert || horiz)
			{
				CDrawingObject *merge_a = NULL;
				CDrawingObject *merge_b = NULL;

				bool is_junc_a = false;
				bool is_junc_b = false;

				// Look for any wire starting at this point
				// with the same orientation...
				// Or look for a junction at this point
				drawingIterator it = m_pDesign->GetDrawingBegin();
				while (it != m_pDesign->GetDrawingEnd()) 
				{
					CDrawingObject *test = *it;

					if (test->GetType() == xJunction)
					{
						if (test->m_point_a == pointer->m_point_a)
						{
							is_junc_a = true;
						}
						if (test->m_point_a == pointer->m_point_b)
						{
							is_junc_b = true;
						}
					}

					// Is this object not our comparision object  
					if (test != pointer && m_discards.find( test ) == m_discards.end())
					{
						// Is this a wire?
						if (!merge_a && test->GetType() == xWire && test->m_point_a != test->m_point_b)
						{
							// Is our pointer connected to this wire?
							CLineUtils l( test->m_point_a, test->m_point_b );
							CLineUtils p( pointer->m_point_a, pointer->m_point_b );

							double d;

							BOOL do_merge_a = l.IsPointOnLine( pointer->m_point_a, d);
							BOOL do_merge_b = l.IsPointOnLine( pointer->m_point_b, d);

							if (do_merge_a || do_merge_b)
							{
								bool test_vert = test->m_point_a.x == test->m_point_b.x;
								bool test_horiz = test->m_point_a.y == test->m_point_b.y;


								// Was is this a duplicate wire?  That is a wire that starts and ends
								// on the test wire
								if (do_merge_a && do_merge_b && l.GetLength() <= p.GetLength())
								{
									// Now make this wire discardable 
									// this will be picked up by the Clean() routine
									m_discards.insert( test );
								}
								else if (	(test_vert && test_vert == vert) 
									||	(test_horiz && test_horiz == horiz))
								{
									if (do_merge_a)
									{
										merge_a = test;
									}
									else
									{
										merge_b = test;
									}
								}
							}

						}
					}

					++ it;
				}

				if (merge_a && !is_junc_a)
				{
					CLineUtils l( pointer->m_point_a, pointer->m_point_b );
					double d;

					CDPoint new_b = pointer->m_point_b;
					CDPoint new_a = merge_a->m_point_a == pointer->m_point_a ? merge_a->m_point_b : merge_a->m_point_a;

					if (!l.IsPointOnLine(new_a,d))
					{
						pointer->m_point_a = new_a;
						pointer->m_point_b = new_b;
					}

					// Now make other wire discardable 
					// this will be picked up by the Clean() routine
					m_discards.insert( merge_a );
				}


				if (merge_b && !is_junc_b)
				{
					CLineUtils l( pointer->m_point_a, pointer->m_point_b );
					double d;

					CDPoint new_a = pointer->m_point_a;
					CDPoint new_b = merge_b->m_point_a == pointer->m_point_b ? merge_b->m_point_b : merge_b->m_point_a;

					if (!l.IsPointOnLine(new_b,d))
					{
						pointer->m_point_a = new_a;
						pointer->m_point_b = new_b;
					}
					// Now make other wire discardable 
					// this will be picked up by the Clean() routine
					m_discards.insert( merge_b );

				}
			}
		}

		++ it_wires;
	}
}

void CDragUtils::Clean()
{
	// We look at the lines we have played with to remove
	// any zero length wires we max have created by
	// this drag...
	// Find any wires attached to this pin and add them to the list
	wireCollection::iterator it_wires = m_draggedWires.begin();
	while (it_wires != m_draggedWires.end())
	{
		CDrawingObject *pointer = (*it_wires).m_Object;
	
		// Is this a zero length wire?
		if (pointer->GetType() == xWire && pointer->m_point_a == pointer->m_point_b)
		{
			// Yes, so delete it!
			m_discards.insert( pointer );
		}
		else
		{
			m_j.AddObjectToTodo( pointer );
		}

		++ it_wires;
	}

	// Now scan the discard list...
	discardCollection::iterator it_disc = m_discards.begin();
	while (it_disc != m_discards.end())
	{
		m_j.AddObjectToTodo( *it_disc );
		m_pDesign->Delete( *it_disc );
		++ it_disc;
	}
}


void CDragUtils::End( bool no_clean )
{
	// Clear out the discards list
	m_discards.erase( m_discards.begin(), m_discards.end() );

	if (!no_clean)
	{
		// Now determine which of these objects are still in the
		// drawing...
		wireCollection::iterator it_wires = m_draggedWires.begin();
		while (it_wires != m_draggedWires.end())
		{
			CDrawingObject *pointer = (*it_wires).m_Object;
			wireCollection::iterator prev_it_wires = it_wires ++;
			if (!m_pDesign->IsInDrawing( pointer ))
			{
				it_wires = m_draggedWires.erase( prev_it_wires );
			}
		}

		// Merge any wires that are now straight!
		Merge();

		// Now discard any zero length wires
		Clean();

		// If we have made some changes then we had better re-check the
		// junctions...
		m_j.CheckTodoList( true );
	}

	Clear();
}


void CDragUtils::AddWireToCollection( dragWire n )
{
	// Now update this entry in the system...
	wireCollection::iterator it = find(m_draggedWires.begin(), m_draggedWires.end(), n);

	if (it == m_draggedWires.end())
	{
		// New
		m_draggedWires.push_back( n );
	
/*		TRACE("Adding %X Move_A(%d) = %X, Move_B(%d) = %X\n",
			n.m_Object,
			n.m_MoveA, n.m_Attached_LineA,
			n.m_MoveB, n.m_Attached_LineB );
*/
		// Now add any attached wires to this object...
		if (n.m_Object->GetType() == xWire || n.m_Object->GetType() == xBus)
		{
			AddAttachedObjects( n.m_Object, n.m_MoveA, n.m_MoveB );
		}

	}
	else
	{
		// Update
		if (n.m_MoveA && !(*it).m_MoveA)
		{
			(*it).m_MoveA |= n.m_MoveA;
			(*it).m_Attached_LineA = n.m_Attached_LineA;
			(*it).m_Distance_AlongA = n.m_Distance_AlongA;
		}
		if (n.m_MoveB && !(*it).m_MoveB)
		{
			(*it).m_MoveB |= n.m_MoveB;
			(*it).m_Attached_LineB = n.m_Attached_LineB;
			(*it).m_Distance_AlongB = n.m_Distance_AlongB;
		}

/*		TRACE("Updating %X Move_A(%d) = %X, Move_B(%d) = %X\n",
			(*it).m_Object,
			(*it).m_MoveA, (*it).m_Attached_LineA,
			(*it).m_MoveB, (*it).m_Attached_LineB );
*/
	}
}

// Add any wires that end at the point given to
// the drag list
void CDragUtils::AddWiresAtPoint( CDPoint hs )
{
	// Find any wires attached to this pin and add them to the list
	drawingIterator it = m_pDesign->GetDrawingBegin();
	while (it != m_pDesign->GetDrawingEnd()) 
	{
		CDrawingObject *pointer = *it;

		// Only search unselected objects
		if (!m_pDesign->IsSelected( pointer ))
		{
			if ((pointer->GetType()==xWire || pointer->GetType()==xBus) 
				&& (pointer->m_point_a == hs || pointer->m_point_b==hs)) 
			{
				// Build the undo list
				m_pDesign->MarkChangeForUndo( pointer );
				AddWireToCollection( dragWire(pointer, pointer->m_point_a == hs, pointer->m_point_b == hs) );
			}
		}

		++ it;
	}
}

// Given a wire, add any objects that are attached to it...
void CDragUtils::AddAttachedObjects( CDrawingObject *wire, BOOL wire_move_a, BOOL wire_move_b )
{
	CLineUtils l( wire->m_point_a, wire->m_point_b );


	// Find any wires attached to this wire and add them in...
	drawingIterator it = m_pDesign->GetDrawingBegin();
	while (it != m_pDesign->GetDrawingEnd()) 
	{
		CDrawingObject *pointer = *it;

		// Is this the incoming wire?
		if (pointer == wire)
		{
			++ it;
			continue;
		}

		switch (pointer->GetType())
		{
		case xBus:
		case xWire:
			// Is this wire attached at either end to this
			// wire?
			double distance_along_a = 0;
			double distance_along_b = 0;
			BOOL move_a = FALSE;
			BOOL move_b = FALSE;
			
			move_a = l.IsPointOnLine( pointer->m_point_a, distance_along_a );
			move_b = l.IsPointOnLine( pointer->m_point_b, distance_along_b );

			// Does this object require moving?
			if (move_a || move_b)
			{
				// Build our drag wire
				bool use = false;
				dragWire n( pointer, move_a, move_b );

				if (move_a)
				{
					if (	(wire_move_a && distance_along_a < 1.0)
						||  (wire_move_b && distance_along_a > 0))
					{
						use |= true;
					}

					n.m_Distance_AlongA = distance_along_a;
					n.m_Attached_LineA = wire;
				}
				if (move_b)
				{
					if (	(wire_move_a && distance_along_b < 1.0)
						||  (wire_move_b && distance_along_b > 0))
					{
						use |= true;
					}

					n.m_Distance_AlongB = distance_along_b;
					n.m_Attached_LineB = wire;
				}

				// Build the undo list
				if (use)
				{
					m_pDesign->MarkChangeForUndo( n.m_Object );
					AddWireToCollection( n );
				}
			}
			break;
		}
		
		++ it;
	}
}

// Move any of the objects that are attached to this object...
//
void CDragUtils::MoveAttachedObjects( CDrawingObject *wire )
{
	CLineUtils l( wire->m_point_a, wire->m_point_b );

	wireCollection::iterator it_wires = m_draggedWires.begin();
	while (it_wires != m_draggedWires.end())
	{
		BOOL Updated = FALSE;
		CDPoint move_by;

		if (!(*it_wires).m_Done_A && (*it_wires).m_MoveA && (*it_wires).m_Attached_LineA == wire) 
		{
			Updated = TRUE;
			CDPoint r = l.GetPointOnLine( (*it_wires).m_Distance_AlongA, &m_pDesign->m_snap );
//			move_by = (*it_wires).m_Object->m_point_a - r;
//			TRACE("A move_by = %lg,%lg\n", move_by.x,move_by.y);
			(*it_wires).m_Object->m_point_a = r;
			(*it_wires).m_Done_A = TRUE; 

//			TRACE("Moving %X on A at %lg to %lg,%lg from %X\n", (*it_wires).m_Object,(*it_wires).m_Distance_AlongA,r.x,r.y, wire );
		}

		if (!(*it_wires).m_Done_B && (*it_wires).m_MoveB && (*it_wires).m_Attached_LineB == wire) 
		{
			Updated = TRUE;
			CDPoint r = l.GetPointOnLine( (*it_wires).m_Distance_AlongB, &m_pDesign->m_snap );
//			move_by = (*it_wires).m_Object->m_point_b - r;
//			TRACE("B move_by = %lg,%lg\n", move_by.x,move_by.y);
			(*it_wires).m_Object->m_point_b = r;
			(*it_wires).m_Done_B = TRUE; 

//			TRACE("Moving %X on B at %lg to %lg,%lg from %X\n", (*it_wires).m_Object,(*it_wires).m_Distance_AlongB,r.x,r.y, wire );
		}


		if (Updated)
		{
			// (*it_wires).Lock();
			(*it_wires).m_Object->Display();

			// Now move any connected wires...
			MoveAttachedObjects( (*it_wires).m_Object );
		}

		++ it_wires;
	}
}

void CDragUtils::Begin( CDPoint ai, CDPoint bi )
{
	Clear();
	m_a = ai;
	m_b = bi;
}

void CDragUtils::Begin()
{
	m_started = TRUE;

	// Search for Methods which might be attached to wires via pins
	m_draggedWires.erase( m_draggedWires.begin(), m_draggedWires.end() );

	selectIterator it = m_pDesign->GetSelectBegin();
	while ( it != m_pDesign->GetSelectEnd() ) 
	{
		CDrawingObject *obj=*it;
		m_pDesign->MarkChangeForUndo( obj );

		CActiveNode a;
		obj->GetActiveListFirst( a );
		while (obj->GetActive( a ))
		{
			// Are there any wires connected to this point?
			AddWiresAtPoint( a.m_a );
		}

		++ it;
	}

	// Search for wires which might need one point fixed in place
	BOOL MoveA,MoveB;
	it = m_pDesign->GetSelectBegin();
	while ( it != m_pDesign->GetSelectEnd() ) 
	{
		CDrawingObject *obj=*it;
		MoveA=FALSE;
		MoveB=FALSE;

		// Is this a wire which needs to be draged?
		if (obj->GetType()==xWire || obj->GetType()==xBus) 
		{
			if (   obj->m_point_a.x>=min(m_a.x,m_b.x) && obj->m_point_a.x<=max(m_a.x,m_b.x)
			    && obj->m_point_a.y>=min(m_a.y,m_b.y) && obj->m_point_a.y<=max(m_a.y,m_b.y) )
				MoveA = TRUE;

			if (   obj->m_point_b.x>=min(m_a.x,m_b.x) && obj->m_point_b.x<=max(m_a.x,m_b.x)
			    && obj->m_point_b.y>=min(m_a.y,m_b.y) && obj->m_point_b.y<=max(m_a.y,m_b.y) )
				MoveB = TRUE;

			// If neither point is inside this, then
			// choose to move both...
			if (!MoveA && !MoveB)
			{
				MoveA = TRUE;
				MoveB = TRUE;
				AddWiresAtPoint( obj->m_point_a );
				AddWiresAtPoint( obj->m_point_b );
			}

			AddWireToCollection( dragWire( obj, MoveA, MoveB ) );
		}

		++ it;
	}
}

// Call "display" on all of the wires in the dragged list
void CDragUtils::DisplayDraggedWires()
{
	wireCollection::iterator it_wires = m_draggedWires.begin();
	while (it_wires != m_draggedWires.end())
	{
		m_j.AddObjectToTodo( (*it_wires).m_Object );
		(*it_wires).m_Object->Display();
		(*it_wires).m_Done_A = FALSE;
		(*it_wires).m_Done_B = FALSE;
		++ it_wires;
	}
}

void CDragUtils::MergeLinePoint( CDrawingObject *p )
{
	Clear();
	m_draggedWires.push_back( dragWire(p, true, true) );
	End( false );
}


void CDragUtils::Drag( CDPoint r )
{
//	TRACE("Drag by %lg,%lg\n", r.x,r.y);

	// Have we already started?
	if (!m_started)
	{
		Begin();
	}

	// Is there anything to do?
	if (r == CDPoint(0,0))
	{
		return;
	}


	// With drag only move the points of a wire inside the update region
	selectIterator it = m_pDesign->GetSelectBegin();
	while ( it != m_pDesign->GetSelectEnd() ) 
	{
		CDrawingObject *obj=*it;

		bool defer = find(m_draggedWires.begin(), m_draggedWires.end(), dragWire(obj)) != m_draggedWires.end();
		if (!defer)
		{
			obj->Display();

			m_j.AddObjectToTodo(obj);

			// Move the object
			obj->Shift( r );

			m_j.AddObjectToTodo(obj);

			obj->Display();
		}
		
		++ it;
	}

	DisplayDraggedWires();
//	TRACE("m_draggedWires.size = %i\n",m_draggedWires.size());

	wireCollection::iterator it_wires = m_draggedWires.begin();
	while (it_wires != m_draggedWires.end())
	{
		BOOL moved = FALSE;

		if ((*it_wires).m_MoveA && (*it_wires).m_Attached_LineA == NULL) 
		{
			(*it_wires).m_Done_A = TRUE;
			(*it_wires).m_Object->m_point_a += r;

			moved |= TRUE;
//			TRACE("Moving %X on A by %lg,%lg\n", (*it_wires).m_Object,r.x,r.y);
		}

		if ((*it_wires).m_MoveB && (*it_wires).m_Attached_LineB == NULL) 
		{
			(*it_wires).m_Done_B = TRUE;
			(*it_wires).m_Object->m_point_b += r;

			moved |= TRUE;
//			TRACE("Moving %X on B by %lg,%lg\n", (*it_wires).m_Object,r.x,r.y);
		}

		// (*it_wires).Lock();
	
		// Now move any attached items to this..
		if (moved)
		{
			MoveAttachedObjects( (*it_wires).m_Object );
		}

		++ it_wires;
	}

	DisplayDraggedWires();

	m_a += r;
	m_b += r;

	// Now check for junctions
	m_j.CheckTodoList( false );
}



void CDragUtils::Move( CDPoint r )
{
	// Have we already started?
	if (!m_started)
	{
		Begin();
	}

	// Is there anything to do?
	if (r == CDPoint(0,0))
	{
		return;
	}

	// Move all of the objects without dragging
	selectIterator it = m_pDesign->GetSelectBegin();
	while ( it != m_pDesign->GetSelectEnd() ) 
	{
		CDrawingObject *obj=*it;
		obj->Display();

		m_j.AddObjectToTodo(obj);

		// Move the object
		obj->Shift( r );

		m_j.AddObjectToTodo(obj);

		obj->Display();

		++ it;
	}

	m_a += r;
	m_b += r;

	// Now check for junctions
	m_j.CheckTodoList( false );
}
