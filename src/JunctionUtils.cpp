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

// JunctionUtils.cpp: implementation of the CJunctionUtils class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tinycad.h"
#include "TinyCadDoc.h"
#include "JunctionUtils.h"
#include "LineUtils.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CJunctionUtils::CJunctionUtils(CTinyCadDoc*	pDesign)
{
	m_pDesign = pDesign;
}

CJunctionUtils::~CJunctionUtils()
{

}


void CJunctionUtils::CheckJunctionRequirement( CDPoint a, CDPoint b, bool perform_split )
{
  CLineUtils l(a,b);

  	// Is this enabled?
	if (!m_pDesign->GetOptions()->GetAutoJunc())
	{
		return;
	}

	typedef std::set<CDPoint> checkCollection;
	checkCollection check;


    // Search for methods, and look at their pins
	drawingIterator it = m_pDesign->GetDrawingBegin();
	// The GetDrawingEnd() function happens to be a relatively costly operation.
	// Especially noticable when dragging symbols with many connected pins.
	// Solution: store the iterator end as precalculated value.
	drawingIterator itEnd = m_pDesign->GetDrawingEnd();
	while (it != itEnd) 
	{
		CDrawingObject *ObjPtr = *it;

		CDPoint p;
	    CDPoint r;

		switch (ObjPtr->GetType()) 
		{
			case xJunction:
				p = ObjPtr->m_point_a;
				
				if (l.DistanceFromPoint(p, r) <= CLineUtils::pointOnLineDistance)
				{
					check.insert( ObjPtr->m_point_a );
				}
				break;
			case xWire:
				if (l.DistanceFromPoint(ObjPtr->m_point_a, r) <= CLineUtils::pointOnLineDistance)
				{
					check.insert( ObjPtr->m_point_a );
				}
				if (l.DistanceFromPoint(ObjPtr->m_point_b, r) <= CLineUtils::pointOnLineDistance)
				{
					check.insert( ObjPtr->m_point_b );
				}
				break;
			default:
				{
					CDRect s( ObjPtr->m_point_a.x, ObjPtr->m_point_a.y, ObjPtr->m_point_b.x, ObjPtr->m_point_b.y );
					s.NormalizeRect();
					if (l.IsInside(s.left- CLineUtils::pointOnLineDistance, s.right + CLineUtils::pointOnLineDistance, 
						s.top - CLineUtils::pointOnLineDistance, s.bottom + CLineUtils::pointOnLineDistance ))
					{
						CActiveNode a;
						
						// Search the symbol for pins
						ObjPtr->GetActiveListFirst( a );
						while (ObjPtr->GetActive( a )) 
						{
							// This is a valid pin...
							CDPoint d = a.m_a;
							if (l.DistanceFromPoint(d, r) <= CLineUtils::pointOnLineDistance)
							{
								check.insert( d );
							}
						}
					}
				}
			break;

		}

		++ it;
	}

	// Now check all of the locations...
	checkCollection::iterator itx = check.begin();
	while (itx != check.end())
	{
		CheckJunctionRequirement( *itx, perform_split );
		++ itx;
	}

}

void CJunctionUtils::CheckJunctionRequirement( CDPoint q, bool perform_split )
{
  CDPoint r(0,0);
  int items = 0;
  BOOL line_end = FALSE;

  CDrawingObject *line_a = NULL;
  CDrawingObject *line_b = NULL;

  	// Is this enabled?
	if (!m_pDesign->GetOptions()->GetAutoJunc())
	{
		return;
	}


    // Search for methods, and look at their pins
	CDrawingObject *split_line = NULL;
	drawingIterator junc;
	bool have_junc = false;

	drawingIterator it = m_pDesign->GetDrawingBegin();
	// The GetDrawingEnd() function happens to be a relatively costly operation.
	// Especially noticable when dragging symbols with many connected pins.
	// Solution: store the iterator end as precalculated value.
	drawingIterator itEnd = m_pDesign->GetDrawingEnd();
	while (it != itEnd) 
	{
		CDrawingObject *ObjPtr = *it;

		switch (ObjPtr->GetType()) 
		{
			case xWire:
				// If this wire is on the discard pile, or is of zero length
				// then treat it as if it doesn't exist...
				if ((m_discards.size()==0 || m_discards.find(ObjPtr) == m_discards.end())
					&& ObjPtr->m_point_a != ObjPtr->m_point_b)
				{

					CDrawLine *theLine = static_cast<CDrawLine*>(ObjPtr);
					CDPoint d;
					CLineUtils l( theLine->m_point_a, theLine->m_point_b );
					double distance = l.DistanceFromPoint( q, d );

					if (distance <= CLineUtils::pointOnLineDistance)
					{
						items ++;

						// Keep a copy for the merge...
						if (line_a == NULL)
						{
							line_a = ObjPtr;
						}
						else
						{
							line_b = ObjPtr;
						}


						// A point mid-way along a line is worth 2 lines
						// (because we may have to break them in the middle)
						if (q != theLine->m_point_a && q != theLine->m_point_b)
						{
							items ++;
							split_line = ObjPtr;
						}
						else
						{
							line_end = TRUE;
						}
					}
				}
			break;
			case xJunction:
				{
					// This is a valid pin...
					if (ObjPtr->m_point_a == q)
					{
						junc = it;
						have_junc = true;
					}
				}
				break;
			default:
				{				
					CDRect s( ObjPtr->m_point_a.x, ObjPtr->m_point_a.y, ObjPtr->m_point_b.x, ObjPtr->m_point_b.y );
					s.NormalizeRect();
					s.left -= CLineUtils::pointOnLineDistance;
					s.right += CLineUtils::pointOnLineDistance;
					s.bottom += CLineUtils::pointOnLineDistance;
					s.top -= CLineUtils::pointOnLineDistance;

					if (s.PtInRect(q))
					{
						CActiveNode a;
						ObjPtr->GetActiveListFirst( a );
						while (ObjPtr->GetActive(a)) 
						{
							// This is a valid pin...
							CDPoint d = a.m_a;

							if (d == q)
							{
								items ++;
								line_end = TRUE;
							}
						}
					}
				}
				break;
		}

		++ it;
	}


  if ((items <= 2 || !line_end) && have_junc)
  {
	  // No junction required, delete it!
	  CDrawingObject *pJunction = *junc;
	  m_pDesign->Delete( junc );

	  // Do we need to merge a wire?
	  if (perform_split && line_a && line_b)
	  {
			bool vert_a = line_a->m_point_a.x == line_a->m_point_b.x;
			bool horiz_a = line_a->m_point_a.y == line_a->m_point_b.y;

			bool vert_b = line_b->m_point_a.x == line_b->m_point_b.x;
			bool horiz_b = line_b->m_point_a.y == line_b->m_point_b.y;

			if (horiz_a && horiz_a == horiz_b)
			{
				// Join horizontally
				double min_x, max_x; 
				min_x = min( line_a->m_point_a.x, line_a->m_point_b.x ); 
				min_x = min( min_x, line_b->m_point_a.x ); 
				min_x = min( min_x, line_b->m_point_b.x ); 

				max_x = max( line_a->m_point_a.x, line_a->m_point_b.x ); 
				max_x = max( max_x, line_b->m_point_a.x ); 
				max_x = max( max_x, line_b->m_point_b.x ); 

				m_pDesign->MarkChangeForUndo( line_a );
				if (line_a->m_point_a.x < line_a->m_point_b.x) 
				{ 
					line_a->m_point_a.x = min_x; 
					line_a->m_point_b.x = max_x; 
				} 
				else 
				{ 
					line_a->m_point_b.x = min_x; 
					line_a->m_point_a.x = max_x; 
				} 
				
				m_discards.insert(line_b);
			}
			else if (vert_a && vert_a == vert_b)
			{
				// Join vertically
				double min_y, max_y; 
				min_y = min( line_a->m_point_a.y, line_a->m_point_b.y ); 
				min_y = min( min_y, line_b->m_point_a.y ); 
				min_y = min( min_y, line_b->m_point_b.y ); 

				max_y = max( line_a->m_point_a.y, line_a->m_point_b.y ); 
				max_y = max( max_y, line_b->m_point_a.y ); 
				max_y = max( max_y, line_b->m_point_b.y ); 

				if (line_a->m_point_a.y < line_a->m_point_b.y) 
				{ 
					line_a->m_point_a.y = min_y; 
					line_a->m_point_b.y = max_y; 
				} 
				else 
				{ 
					line_a->m_point_b.y = min_y; 
					line_a->m_point_a.y = max_y; 
				}

				m_discards.insert(line_b);
			}
	  }
  }
  else if (items > 2 && !have_junc && line_end)
  {
	  m_pDesign->AddNewJunction( q );
  }

  // Do we need to split a wire here?
  if (perform_split && items>2 && line_end && split_line)
  {
	  // Yep, so let us split the line, after we have
	  // registered the changes
	  
	  // Create a new line
	  CDrawLine *new_line = new CDrawLine( m_pDesign, xWire );
	  *new_line = *static_cast<CDrawLine*>(split_line);	// Copy over any options...
	  new_line->m_point_a = split_line->m_point_a;
	  new_line->m_point_b = q;
	  m_pDesign->Add( new_line );

	  // Shorten the old line
	  m_pDesign->MarkChangeForUndo( split_line );
	  split_line->Display();
	  split_line->m_point_a = q;

  }
}


// Add an object to the todo list
void CJunctionUtils::AddObjectToTodo( CDrawingObject *obj )
{
	// Is this enabled?
	if (!m_pDesign->GetOptions()->GetAutoJunc())
	{
		return;
	}

	// Extract the todo list...
	switch (obj->GetType()) 
	{
	case xWire:
		m_todo.insert( todo_point( obj->m_point_a, obj->m_point_b ) );
		break;
	case xJunction:
		m_todo.insert( todo_point( obj->m_point_a ) );
		break;
	default:
		{
			CActiveNode a;
			
			// Search the symbol for pins
			obj->GetActiveListFirst( a );
			while (obj->GetActive( a )) 
			{
				// This is a valid pin...
				CDPoint d = a.m_a;
				m_todo.insert( todo_point( d ) );
			}
		}
		break;
	}
}

void CJunctionUtils::CheckTodoList(bool perform_split)
{
	// Is this enabled?
	if (!m_pDesign->GetOptions()->GetAutoJunc())
	{
		return;
	}

	// Now check-up on the junctions
	todoList::iterator it = m_todo.begin();
	while (it != m_todo.end())
	{
	  if ((*it).a == (*it).b)
	  {
		  CheckJunctionRequirement( (*it).a, perform_split );
	  }
	  else
	  {
		  CheckJunctionRequirement( (*it).a, (*it).b, perform_split );
	  }
	  ++ it;
	}

	m_todo.erase( m_todo.begin(), m_todo.end() );

	// Now use the clean
	if (perform_split)
	{
		// Scan the discard list...
		discardCollection::iterator it_disc = m_discards.begin();
		while (it_disc != m_discards.end())
		{
			m_pDesign->Delete( *it_disc );
			++ it_disc;
		}

		m_discards.erase( m_discards.begin(), m_discards.end() );
	}
}

// Paint the junctions in the todo list
void CJunctionUtils::PaintJunctions( CContext &dc, paint_options opt )
{
	// Is this enabled?
	if (!m_pDesign->GetOptions()->GetAutoJunc())
	{
		return;
	}


	// Search for junctions and check if they are connected to
	// our todo list...
	drawingIterator it = m_pDesign->GetDrawingBegin();
	while (it != m_pDesign->GetDrawingEnd()) 
	{
		CDrawingObject *ObjPtr = *it;

		CDPoint p;
		CDPoint r;

		if (ObjPtr->GetType() == xJunction)
		{
			p = ObjPtr->m_point_a;
			bool paint = false;
			
			todoList::iterator i = m_todo.begin();
			while (i != m_todo.end())
			{
			  if ((*i).a == (*i).b)
			  {
				  if ((*i).a == p)
				  {
					  paint = true;
					  break;
				  }
			  }
			  else
			  {
				  CLineUtils l((*i).a,(*i).b);
				  if (l.DistanceFromPoint(p, r) <= CLineUtils::pointOnLineDistance)
				  {
					  paint = true;
					  break;
				  }
			  }
			  ++ i;
			}

			if (paint)
			{
				ObjPtr->Paint( dc, opt );
			}
		}

		++ it;
	}

}

