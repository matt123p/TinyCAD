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
#include <math.h>

#include "TinyCadView.h"
#include "diag.h"
#include "colour.h"
#include "option.h"
#include "LineUtils.h"
#include "JunctionUtils.h"


////// The item edit object //////


CDrawEditItem::CDrawEditItem( CTinyCadDoc *pDesign )
: CDrawingObject( pDesign )
, m_drag_utils( pDesign )
{
  m_segment=1;
  EditMethodText = -1;
  InMove = FALSE;
  InSelectByDrag = FALSE;
  LastPos = CDPoint(-1,-1);
}

void CDrawEditItem::BeginEdit( BOOL re_edit )
{
	// If there is a single item selected then
	// begin editing it...
	if (m_pDesign->IsSingleItemSelected())
	{
		m_pDesign->GetSingleSelectedItem()->BeginEdit( TRUE );
		m_segment = 0;
	}
}

void CDrawEditItem::Display( BOOL erase )
{
	if (m_point_a != m_point_b)
	{
		CDRect r( m_point_a.x,m_point_a.y,m_point_b.x,m_point_b.y);
		m_pDesign->InvalidateRect( r, erase, 2, TRUE );
	}
}


void CDrawEditItem::EndEdit()
{
  if (!m_segment)
  {
	(m_pDesign->GetSingleSelectedItem())->EndEdit();
  }

  m_drag_utils.End( false );

  m_pDesign->UnSelect();
//  m_pDesign->Invalidate();

  m_segment=1;
}


void CDrawEditItem::NewOptions()
{
	selectIterator it = m_pDesign->GetSelectBegin();
	while ( it != m_pDesign->GetSelectEnd() ) 
	{
		CDrawingObject *obj=*it;
		obj->NewOptions();
		++ it;
	}
}

void CDrawEditItem::Move(CDPoint p, CDPoint no_snap_p)
{
  CDPoint r = CDPoint(p.x - LastPos.x,p.y - LastPos.y);

  if (EditMethodText!=-1) 
  {
	  m_pDesign->SetSelectable( NULL );

	  // Is the control key down?
	  if (GetAsyncKeyState(VK_CONTROL) < 0 && m_pDesign->GetSingleSelectedItem()->CanControlAspect())
	  {
		  // Ctrl key pressed, so force the movement to be
		  // in a fixed aspect ratio way
		  switch (EditMethodText)
		  {
			// Tracker movement...
			case CRectTracker::hitBottomLeft:
			case CRectTracker::hitTopRight:
			  if (fabs(r.x) > fabs(r.y))
			  {
				r.y = -r.x;
			  }
			  else
			  {
				r.x = -r.y;
			  }
				break;
			case CRectTracker::hitTopLeft:
			case CRectTracker::hitBottomRight:
			  if (abs(r.x) > abs(r.y))
			  {
				r.y = r.x;
			  }
			  else
			  {
				r.x = r.y;
			  }
 			  break;	
			case CRectTracker::hitTop:
			case CRectTracker::hitRight:
			case CRectTracker::hitBottom:
			case CRectTracker::hitLeft:
				break;		
			}
	  }
	  m_pDesign->GetSingleSelectedItem()->MoveField(EditMethodText,r);
  } 
  else if (InSelectByDrag) 
  {
	  m_pDesign->SetSelectable( NULL );
	  if (m_point_b != p)
	  {
  		Display();
  		m_point_b = p;
  		Display(FALSE);

	  } 
  }
  else if (InMove) 
  {
	  m_pDesign->SetSelectable( NULL );

	  if (r != CDPoint(0,0))
	  {
		  // Is the control key down?
		  BOOL do_move = GetAsyncKeyState(VK_CONTROL) < 0;
		  if (!m_pDesign->GetOptions()->GetAutoDrag())
		  {
			  do_move = !do_move;
		  }

		  if (do_move)
		  {
	  		  m_drag_utils.Move(r);
		  }
		  else
		  {
			  m_drag_utils.Drag(r);
		  }
	  }
  }
  else
  {
	// Is there anything under the pointer?
	int check_field = -1;
	if (m_pDesign->IsSingleItemSelected())
	{
		check_field = m_pDesign->GetSingleSelectedItem()->IsInsideField(p);
	}

	if (check_field == -1)
	{
		m_pDesign->SetSelectable( GetClosestObject(no_snap_p) );
	}
	else
	{
		m_pDesign->SetSelectable( NULL );
	}
  }

  LastPos = p;	
}


int CDrawEditItem::SetCursor( CDPoint p )
{
	if (m_pDesign->IsSelected()) 
	{

	   	// Is this object currently being edited a method object?
	  	if (m_pDesign->IsSingleItemSelected()) 
		{
			// Has the user clicked on one of the method fields?
			return m_pDesign->GetSingleSelectedItem()->SetCursorEdit( p );
  		}
		else
		{
			selectIterator it = m_pDesign->GetSelectBegin();
			while ( it != m_pDesign->GetSelectEnd() ) 
			{
				CDrawingObject *obj=*it;
				if (obj->DistanceFromPoint( p ) <= 15)
					return 11;
				++ it;
			}
		}
	}

	return -1;
}


void CDrawEditItem::ClickSelection( CDPoint p, CDPoint no_snap_p )
{
	LastPos = p;

	// Has the user clicked in a field to be moved/edited?
  	if (m_pDesign->IsSingleItemSelected()) 
	{
		// Has the user clicked on one of the method fields?
		EditMethodText = m_pDesign->GetSingleSelectedItem()->IsInsideField(no_snap_p);
		if (EditMethodText != -1)
		{
			// Begin Editing of a method field
			m_pDesign->BeginNewChangeSet();
			m_pDesign->MarkChangeForUndo( m_pDesign->GetSingleSelectedItem() );
			InMove = TRUE;
			return;
		}
	}


	// Has the user clicked on a new item to be selected?

	// Unselect the curently selected items
 	BOOL ctrl_pressed = GetAsyncKeyState(VK_CONTROL) < 0;

	CDrawingObject *closest_object = GetClosestObject( no_snap_p );

	if (!ctrl_pressed)
	{
		if (!m_pDesign->IsSelected( closest_object))
		{
			// This can cause the selection to change, so we
			// search again...
   			EndSelection();

			closest_object = GetClosestObject( no_snap_p );
		}
	}


	if (closest_object != NULL) 
	{
		// If necessary remove the line from the screen
		if (!m_segment && m_pDesign->IsSingleItemSelected()) 
		{
			(m_pDesign->GetSingleSelectedItem())->EndEdit();
			closest_object = GetClosestObject( no_snap_p );
			m_segment = 1;
		}

		if (ctrl_pressed)
		{
			// If ctrl is pressed then toggle the state...
			if (m_pDesign->IsSelected( closest_object ))
			{

				m_pDesign->UnSelect( closest_object );
			}
			else
			{
				m_pDesign->Select( closest_object );
			}
		}
		else
		{
			// Move selected item to the head
			m_pDesign->BeginNewChangeSet();
			m_pDesign->Select(closest_object);
			m_pDesign->MarkChangeForUndo( closest_object );
		}

  		closest_object->Display();

		// Is there a single item selected?
		if (m_pDesign->IsSingleItemSelected())
		{

  			// Select this item
			m_drag_utils.Begin(p,p);

			if ((m_pDesign->GetSingleSelectedItem())->CanEdit()) 
			{
				// Begin Editing this Item
				m_pDesign->BeginNewChangeSet();
				(m_pDesign->GetSingleSelectedItem())->BeginEdit(TRUE);
			  	m_segment=0;
			}
			else
			{
				m_segment=1;
			}
		}
		else
		{
			m_segment = 1;
		}
	}

}

void CDrawEditItem::LButtonDown(CDPoint p, CDPoint no_snap_p)
{
	ClickSelection( p, no_snap_p );

	// Was a selection found?
	if (!m_pDesign->IsSelected())
	{
		// No, so start the select box...
		m_point_a = p;
		m_point_b = p;

		InMove = FALSE;
 		InSelectByDrag = TRUE;

	  	Display();
	}
	else
	{
	   	// Is this object currently being edited a method object?
	  	if (m_pDesign->IsSingleItemSelected()) 
		{
			// Has the user clicked on one of the method fields?
			EditMethodText = m_pDesign->GetSingleSelectedItem()->IsInsideField(no_snap_p);
  		}

		InMove = TRUE;
	}

	m_pDesign->ForceSetCursor();
}


void CDrawEditItem::Paint(CContext &dc,paint_options options)
{

  dc.SelectBrush();
  dc.SelectPen(PS_DOT,1,cBLOCK);
  dc.SetROP2(R2_COPYPEN);

  dc.Rectangle(CDRect(m_point_a.x,m_point_a.y,m_point_b.x,m_point_b.y));
}


CDrawingObject* CDrawEditItem::GetClosestObject( CDPoint p )
{
  	// Search each object in turn
	CDrawingObject* closest_object = NULL;
	double closest_distance = 15.0;

	drawingIterator it = m_pDesign->GetDrawingBegin();
	while (it != m_pDesign->GetDrawingEnd()) 
	{
		CDrawingObject *pointer = *it;

		double distance = pointer->DistanceFromPoint( p );
		if (distance <= closest_distance)
		{
			closest_object = pointer;
			closest_distance = distance;
		}

		// Move pointer on
		++ it;
  	}

	return closest_object;
}


void CDrawEditItem::LButtonUp(CDPoint p)
{
    // Keep a copy of this position
	LastPos = p;

  	// Were we moving a method field?
	if (EditMethodText != -1) 
	{
		EditMethodText = -1;
	}


	// Are we in a move?
	if (InMove) 
	{
		InMove = FALSE;
		return;
	}

	// Turn off the drag box (if there is one...)
	if (InSelectByDrag) 
	{
		Display();
		InSelectByDrag = FALSE;
	}
	else
	{
		m_point_a = m_point_b = CDPoint(0,0);
	}



  // If this was a dragged box then select all the items inside the box!
  if (m_point_a != m_point_b) 
  {
  	//*** Must select a block of items
	//*** Select so they can be moved or cut/copied

  	// Unselect the edited items
	if (GetAsyncKeyState(VK_CONTROL) == 0)
	{
   		EndSelection();
	}

	// Select the items in the box (if any)
	m_pDesign->Select(m_point_a,m_point_b);
	m_drag_utils.Begin(m_point_a,m_point_b);
	m_point_a = m_point_b = CDPoint(0,0);

	// If only one item selected then start editing it!
	if (m_pDesign->IsSingleItemSelected() 
	&& (m_pDesign->GetSingleSelectedItem())->CanEdit()) 
	{
		// Begin Editing this Item
		(m_pDesign->GetSingleSelectedItem())->BeginEdit(TRUE);
		m_segment=0;
	}
	else
	{
		m_segment=1;
	}
  }
}

void CDrawEditItem::ReleaseSelection()
{
  // If in the middle of a move then cancel it!
  InMove = FALSE;

  // If necessary remove the edit item dialog
  if (!m_segment && m_pDesign->IsSingleItemSelected() ) 
  {
	(m_pDesign->GetSingleSelectedItem())->EndEdit();
	if (m_pDesign->GetSingleSelectedItem())
	{
		(m_pDesign->GetSingleSelectedItem())->Display();
	}
  }

  m_segment=1;

}


void CDrawEditItem::RButtonUp(CDPoint p, CDPoint)
{
}

void CDrawEditItem::ContextMenu( CDPoint p, UINT id )
{

	switch (id)
	{
	case IDM_EDITDELITEM:
		{
			if (m_pDesign->IsSelected())
			{
				CJunctionUtils j( m_pDesign );

				selectIterator it = m_pDesign->GetSelectBegin();
				while ( it != m_pDesign->GetSelectEnd() ) 
				{
					CDrawingObject *obj=*it;
					j.AddObjectToTodo( obj );

					++ it;
				}
				ReleaseSelection();
				m_pDesign->BeginNewChangeSet();
				m_pDesign->SelectDelete();
				j.CheckTodoList( true );
				m_pDesign->Invalidate();
			}
		}
		break;
	default:
		if (!m_segment && m_pDesign->IsSingleItemSelected()) 
		{
			(m_pDesign->GetSingleSelectedItem())->ContextMenu(p, id );
		}
	}
}


BOOL CDrawEditItem::RButtonDown(CDPoint p, CDPoint s)
{
	// Same selection rules as LButtonDown(), but no moving!
	ClickSelection( p, s );
	EditMethodText = -1;
	InMove = FALSE;
	InSelectByDrag = FALSE;


	// Get the current location of the mouse
	CPoint mp;
	GetCursorPos( &mp );

	// Now bring up the context menu..
	CMenu menu;
	if (!m_segment && m_pDesign->IsSingleItemSelected()) 
	{
		menu.LoadMenu( (m_pDesign->GetSingleSelectedItem())->GetContextMenu() );
	}
	else
	{
		menu.LoadMenu( IDR_EDITTOOL );
	}
	menu.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN|TPM_RIGHTBUTTON,
		mp.x,mp.y, AfxGetMainWnd(), NULL );
	
	return TRUE;
}

void CDrawEditItem::EndSelection()
{

  // If in the middle of a move then end it!
  if (InMove)
  {
    m_drag_utils.Drag(CDPoint(m_point_a.x - LastPos.x, m_point_a.y - LastPos.y));
	InMove = FALSE;
  }
  m_drag_utils.End( false );


  // If necessary end the selected item
  if (!m_segment) 
  {
	  if (m_pDesign->IsSingleItemSelected())
	  {
		(m_pDesign->GetSingleSelectedItem())->EndEdit();
	  }

	  if (m_pDesign->IsSingleItemSelected())
	  {
		(m_pDesign->GetSingleSelectedItem())->Display();
	  }
  }
	


  // Unselect the last object
  m_pDesign->UnSelect();

  m_segment=1;

}


