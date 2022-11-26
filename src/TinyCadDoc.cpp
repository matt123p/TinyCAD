/*
 * Project:		TinyCAD program for schematic capture
 *				https://www.tinycad.net
 * Copyright:	� 1994-2019 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "TinyCad.h"
#include "TinyCadDoc.h"
#include "TinyCadRegistry.h"
#include "LineUtils.h"
#include <math.h>
#include "ImagePNG.h"
#include "TinyCadMultiDoc.h"

/////////////////////////////////////////////////////////////////////////////
// CTinyCadDoc

#if 0
IMPLEMENT_DYNCREATE(CTinyCadDoc, CDocument)

BEGIN_MESSAGE_MAP(CTinyCadDoc, CDocument)
//{{AFX_MSG_MAP(CTinyCadDoc)
// NOTE - the ClassWizard will add and remove mapping macros here.
//    DO NOT EDIT what you see in these blocks of generated code!
//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif

/////////////////////////////////////////////////////////////////////////////
// CTinyCadDoc construction/destruction

CTinyCadDoc::CTinyCadDoc(CMultiSheetDoc *pParent)
{

	theOptions.Init(this);

	m_pParent = pParent;
	selectable = NULL;
	m_undo_level = 0;
	m_change_set = FALSE;
	m_InUndoAddAction = FALSE;
	m_DuplicateObjectOnly = FALSE;
	edit = NULL;
	NameDir = 1;
	PinDir = 1;

	// Configure the transform
	m_Transform.SetZoomFactor(1.0);
	m_Transform.SetOrigin(CDPoint(0, 0));

	// Now select a drawing object
	SelectObject(new CDrawEditItem(this));

	// Attempt to load the defaults for the grid
	m_snap.SetAccurateGrid(CTinyCadRegistry::GetDouble("GridSpacingF", NormalGrid));
	m_snap.SetGridSnap(CTinyCadRegistry::GetBool("GridSnap", TRUE));
}

CTinyCadDoc::~CTinyCadDoc()
{
	CTinyCadRegistry::Set("GridSpacingF", m_snap.GetAccurateGrid());
	CTinyCadRegistry::Set("GridSnap", m_snap.GetGridSnap());

	// Remove any editing tool
	if (edit)
	{
		edit->EndEdit();
		delete edit;
	}

	// Unselect all items
	UnSelect();

	// Now delete the contents of this document
	for (drawingIterator i = GetDrawingBegin(); i != GetDrawingEnd(); i++)
	{
		CDrawingObject *pointer = *i;
		delete pointer;
	}

	m_drawing.clear();

	// Remove the undo/redo list
	m_undo_level = 0;
	FlushRedo();
}

// Iterate through the drawing
drawingIterator CTinyCadDoc::GetDrawingBegin()
{
	return m_drawing.begin();
}

drawingIterator CTinyCadDoc::GetDrawingEnd()
{
	return m_drawing.end();
}

/////////////////////////////////////////////////////////////////////////////
// CTinyCadDoc serialization


BOOL CTinyCadDoc::Import(CStream& ar)
{
	drawingCollection drawing;

	if (ReadFile(ar, FALSE, drawing))
	{

		for (drawingCollection::iterator i = drawing.begin(); i != drawing.end(); i++)
		{
			Add(*i);
		}

		// Now select the objects
		UnSelect();

		for (drawingIterator j = drawing.begin(); j != drawing.end(); j++)
		{
			Select(*j);
		}

		return TRUE;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CTinyCadDoc commands


// Save as a PNG file
void CTinyCadDoc::SavePNG(const TCHAR *file_name, CDC &ref_dc, int scaling, bool bw, bool rotate)
{
	// Calculate the boundaries
	CDRect rect = CDRect(0, 0, 0, 0);

	if (IsSelected())
	{
		BOOL first = TRUE;

		selectIterator it = GetSelectBegin();
		while (it != GetSelectEnd())
		{
			CDrawingObject *pointer = *it;

			if (first)
			{
				rect.top = pointer->m_point_a.y;
				rect.bottom = pointer->m_point_a.y;
				rect.left = pointer->m_point_a.x;
				rect.right = pointer->m_point_a.x;
				first = FALSE;
			}

			rect.top = min(rect.top,min(pointer->m_point_a.y,pointer->m_point_b.y));
			rect.bottom = max(rect.bottom,max(pointer->m_point_a.y,pointer->m_point_b.y));
			rect.left = min(rect.left,min(pointer->m_point_a.x,pointer->m_point_b.x));
			rect.right = max(rect.right,max(pointer->m_point_a.x,pointer->m_point_b.x));

			++it;
		}
	}
	else
	{
		rect = CDRect(0, 0, GetDetails().m_szPage.cx, GetDetails().m_szPage.cy);
	}

	rect.InflateRect(0, 0, 10, 10);

	// Transform the origin so the design lies at 0,0
	Transform newTransform;
	newTransform.SetOrigin(CDPoint(rect.left, rect.top));

	// Set the scaling
	newTransform.SetZoomFactor(static_cast<double> (scaling) / 100.0);
	rect.right = (rect.right * scaling) / 100;
	rect.bottom = (rect.bottom * scaling) / 100;

	CImagePNG png;

	CBitmap *bitmap = png.CreateImageBitmap(static_cast<int> (rect.Width()), static_cast<int> (rect.Height()), ref_dc, 32);

	CDC bitmap_dc;
	bitmap_dc.CreateCompatibleDC(&ref_dc);
	CBitmap *old_bitmap = bitmap_dc.SelectObject(bitmap);

	bitmap_dc.SelectStockObject(WHITE_BRUSH);
	bitmap_dc.SelectStockObject(WHITE_PEN);
	bitmap_dc.Rectangle(0, 0, static_cast<int> (rect.Width()), static_cast<int> (rect.Height()));

	// Create the DC and render...
	{
		CContext dc(&bitmap_dc, newTransform);
		if (bw)
		{
			dc.SetBlack(TRUE);
		}

		if (IsSelected())
		{
			// Draw just the selected part
			CJunctionUtils j(this);
			for (selectIterator i = GetSelectBegin(); i != GetSelectEnd(); i++)
			{
				j.AddObjectToTodo(*i);
				(*i)->Paint(dc, draw_normal);
			}
			j.PaintJunctions(dc, draw_normal);
		}
		else
		{
			// Draw the whole design into the metafile
			drawingIterator it = GetDrawingBegin();
			while (it != GetDrawingEnd())
			{
				(*it)->Paint(dc, draw_normal);
				++it;
			}

			// Show the design
			Display(dc);
		}

	}

	bitmap_dc.SelectObject(old_bitmap);

	// Do we rotate?
	if (rotate)
	{
		png.Rotate(ref_dc);
	}

	// ... and save to a png
	png.Save(ref_dc, file_name);
}

// Convert this document into a metafile for pasting into
// other applications
HENHMETAFILE CTinyCadDoc::CreateMetafile(CDC &ref_dc, const TCHAR *file_name, bool bw)
{
	HDC hdcMeta = NULL;
	CRect rect = CRect(0, 0, 0, 0);
	for (int loop = 0; loop < 2; ++loop)
	{
		// Pass 1: Calculate the boundaries
		// Pass 2: Draw in to the metafile

		Transform newTransform;
		CDC render_dc;

		if (loop == 0)
		{
			render_dc.CreateCompatibleDC(AfxGetMainWnd()->GetDC());
		}
		else
		{
			// Transform the origin so the design lies at 10,10
			newTransform.SetOrigin(CDPoint(rect.left - 10, rect.top - 10));

			rect.InflateRect(10, 10);

			// We hard-code these to make the output the same regardless of monitor
			int iWidthMM = ref_dc.GetDeviceCaps(HORZSIZE);
			int iHeightMM = ref_dc.GetDeviceCaps(VERTSIZE);
			int iWidthPels = ref_dc.GetDeviceCaps(HORZRES);
			int iHeightPels = ref_dc.GetDeviceCaps(VERTRES);

			// Convert client coordinates to .01-mm units.
			// Use iWidthMM, iWidthPels, iHeightMM, and
			// iHeightPels to determine the number of
			// .01-millimeter units per pixel in the x-
			//  and y-directions.

			rect.left = (rect.left * iWidthMM * 100) / iWidthPels;
			rect.top = (rect.top * iHeightMM * 100) / iHeightPels;
			rect.right = (rect.right * iWidthMM * 100) / iWidthPels;
			rect.bottom = (rect.bottom * iHeightMM * 100) / iHeightPels;

			// Create the metafile device context.
			CRect ir(0, 0, static_cast<int> (rect.Width()), static_cast<int> (rect.Height()));
			hdcMeta = CreateEnhMetaFile(ref_dc.m_hDC, file_name, &ir, GetDetails().GetTitle());
			render_dc.Attach(hdcMeta);

			if (!hdcMeta)
			{
				AfxMessageBox(_T("Cannot create enhanced metafile for the copy"));
				return NULL;
			}
		}

		// Now render into this dc
		newTransform.SetZoomFactor(1.0);

		// Create the DC and render...
		{
			CContext dc(&render_dc, newTransform);
			dc.SetBlack(bw);

			if (IsSelected())
			{
				// Draw just the selected part
				CJunctionUtils j(this);
				selectIterator it = GetSelectBegin();
				while (it != GetSelectEnd())
				{
					j.AddObjectToTodo(*it);
					(*it)->Paint(dc, draw_normal);
					++it;
				}
				j.PaintJunctions(dc, draw_normal);
			}
			else
			{
				// Draw the whole design into the metafile
				for (drawingIterator i = GetDrawingBegin(); i != GetDrawingEnd(); i++)
				{
					(*i)->Paint(dc, draw_normal);
				}

				// Show the design details
				Display(dc);
			}

			if (loop == 0)
			{
				if (IsSelected())
				{
					rect = dc.GetDrawingExtent();
				}
				else
				{
					rect = CRect(0, 0, GetDetails().m_szPage.cx, GetDetails().m_szPage.cy);
				}
			}
		}

		if (loop == 1)
		{
			render_dc.Detach();
		}

	}

	return CloseEnhMetaFile(hdcMeta);
}

// Tag all the resources being used by this design
void CTinyCadDoc::TagAllResources()
{
	for (drawingIterator i = GetDrawingBegin(); i != GetDrawingEnd(); i++)
	{
		(*i)->TagResources();
	}
}

// Remove all errors from this design
void CTinyCadDoc::DeleteErrors()
{
	// Get rid of any drawing tool
	SelectObject(new CDrawEditItem(this));

	drawingIterator it = GetDrawingBegin();
	//(Don't prefetch GetDrawingEnd here)
	while (it != GetDrawingEnd())
	{
		drawingIterator current = it;
		++it;

		CDrawingObject *pointer = *current;
		if (pointer->GetType() == xError)
		{
			m_drawing.erase(current);
			delete pointer;
			// Reset iterator because it was invalidated
			it = GetDrawingBegin();
		}
	}

	Invalidate();
}

// Is this document editing a library?
bool CTinyCadDoc::IsLibInUse(CLibraryStore *lib)
{
	return false;
}

BOOL CTinyCadDoc::IsSelected() const
{
	return m_selected.size() > 0;
}

selectIterator CTinyCadDoc::GetSelectBegin()
{
	return m_selected.begin();
}

selectIterator CTinyCadDoc::GetSelectEnd()
{
	return m_selected.end();
}

BOOL CTinyCadDoc::IsSelected(CDrawingObject * p) const
{
	return m_selected.find(p) != m_selected.end();
}
BOOL CTinyCadDoc::IsSingleItemSelected() const
{
	return m_selected.size() == 1;
}

CDrawingObject* CTinyCadDoc::GetSingleSelectedItem() const
{
	if (m_selected.size() == 1)
	{
		return * (m_selected.begin());
	}
	else
	{
		return NULL;
	}
}

void CTinyCadDoc::Add(drawingCollection& drawing)
{
	for (drawingCollection::iterator i = drawing.begin(); i != drawing.end(); ++i)
	{
		(*i)->m_pDesign = this;
		m_drawing.push_back(*i);
	}
}

// Add a new object to this drawing
void CTinyCadDoc::Add(CDrawingObject *NewObject)
{
	// Is this a valid object?
	if (m_DuplicateObjectOnly || NewObject == NULL) return;

	// Always append to back?
	if (m_InUndoAddAction || NewObject->GetType() == xError)
	{
		// Append to back of list
		m_drawing.push_back(NewObject);
	}
	else
	{

		bool added = false;
		// Insert the object in front of all error objects
		// This is done so that the Undo action will be recorded for this object.
		for (drawingCollection::reverse_iterator rit = m_drawing.rbegin(); rit != m_drawing.rend(); ++rit)
		{
			// skip all xError objects
			if ( (*rit)->GetType() != xError)
			{
				// convert reverse iterator back to normal iterator
				drawingCollection::iterator it = rit.base();
				// Insert the object in the linked list
				m_drawing.insert(it, NewObject);
				added = true;
				break;
			}
		}

		// in case the list is empty or contains only xError objects...
		if (!added)
		{
			// Append to front of list
			m_drawing.insert(m_drawing.begin(), NewObject);
		}
	}

	// Store object and set the dirty flag
	if (NewObject->GetType() != xError)
	{
		if (!m_InUndoAddAction)
		{
			MarkAdditionForUndo(NewObject);
			SetModifiedFlag(TRUE);
		}
	}
}

void CTinyCadDoc::BeginNewChangeSet()
{
	m_change_set = TRUE;
}

// Flush the Redo Buffer
void CTinyCadDoc::FlushRedo()
{
	for (unsigned int i = m_undo_level; i < m_undo.size(); i++)
	{
		CDocUndoSet &s = m_undo[i];
		CDocUndoSet::actionCollection::iterator act_it = s.m_actions.begin();

		while (act_it != s.m_actions.end())
		{
			delete (*act_it).m_object;
			++act_it;
		}

		s.m_actions.resize(0);
	}

	m_undo.resize(m_undo_level + 1);
}

// Undo the last action
void CTinyCadDoc::Undo(BOOL SingleLevel)
{
	SetSelectable(NULL);
	BOOL action_taken = FALSE;

	m_InUndoAddAction = 1;

	// Is this possible?
	while (CanUndo() && !action_taken)
	{
		if (SingleLevel)
		{
			action_taken = TRUE;
		}
		// Re-apply all of the changes we have done at this level
		CDocUndoSet &s = m_undo[m_undo_level];

		if (s.m_dirty == 2)
		{
			m_pParent->CDocument::SetModifiedFlag(FALSE);
			m_undo_level--;
			continue;
		}

		// Copy dirty flag to Document modified flag
		BOOL dirty = m_pParent->CDocument::IsModified();
		m_pParent->CDocument::SetModifiedFlag(s.m_dirty);
		s.m_dirty = (BYTE) dirty;

		// Go through the list of action and undo each one in the reverse
		// order that they were applied
		CDocUndoSet::actionCollection::reverse_iterator act_it = s.m_actions.rbegin();

		while (act_it != s.m_actions.rend())
		{
			CDocUndoSet::CDocUndoAction &act = *act_it;

			// Look up this item from the index...
			drawingCollection::iterator it = m_drawing.begin();
			drawingCollection::iterator itEnd = m_drawing.end();
			int index = act.m_index;
			while (index > 0 && it != itEnd)
			{
				++it;
				--index;
			}

			if (it != itEnd)
			{
				// Remove old object
				(*it)->Display();
			}
			act.m_object->Display();

			switch (act.m_action)
			{
				case CDocUndoSet::Deletion:
				{
					CDrawingObject *obj = Dup(act.m_object);
					// We must re-insert the deleted objects
					m_drawing.insert(it, obj);
					obj->NotifyEdit(CDocUndoSet::Addition);
					action_taken = TRUE;
				}
					break;
				case CDocUndoSet::Addition:
					// We must remove the additions
					if (it != itEnd)
					{
						CDrawingObject *obj = *it;
						obj->NotifyEdit(CDocUndoSet::Deletion);
						delete *it;
						m_drawing.erase(it);
					}
					action_taken = TRUE;
					break;
				case CDocUndoSet::Change:
					// We convert the old objects into the new objects...
					if (it != itEnd)
					{
						// Keep a copy for the redo...
						CDrawingObject *copy = Dup(*it);
						delete *it;

						copy->Display();
						*it = act.m_object;
						(*it)->NotifyEdit(CDocUndoSet::Change);

						// Action taken when object contents differs
						if (*act.m_object != *copy)
						{
							action_taken = TRUE;
						}

						act.m_object = copy;
					}
					break;
			}

			++act_it;
		}

		m_undo_level--;
	}

	// Update the GUI
	ShowModifiedFlag();

	m_InUndoAddAction = 0;
}

// Redo the last action
void CTinyCadDoc::Redo()
{
	SetSelectable(NULL);
	BOOL action_taken = FALSE;

	m_InUndoAddAction = 1;

	// Is this possible?
	while (CanRedo())
	{
		m_undo_level++;

		// Re-apply all of the changes we have done at this level
		CDocUndoSet &s = m_undo[m_undo_level];

		// Is this a document-saved action?
		if (s.m_dirty == 2)
		{
			m_pParent->CDocument::SetModifiedFlag(FALSE);
			if (!action_taken)
			{
				continue;
			}
			// Stop at this undo level
			break;
		}

		if (action_taken)
		{
			m_undo_level--;
			break;
		}

		// Copy dirty flag to Document modified flag
		BOOL dirty = m_pParent->CDocument::IsModified();
		m_pParent->CDocument::SetModifiedFlag(s.m_dirty);
		s.m_dirty = (BYTE) dirty;

		// Go through the list of action and redo each one
		//
		CDocUndoSet::actionCollection::iterator act_it = s.m_actions.begin();

		while (act_it != s.m_actions.end())
		{
			CDocUndoSet::CDocUndoAction &act = *act_it;

			// Look up this item from the index...
			drawingCollection::iterator it = m_drawing.begin();
			drawingCollection::iterator itEnd = m_drawing.end();
			int index = act.m_index;
			while (index > 0 && it != itEnd)
			{
				++it;
				--index;
			}

			if (it != itEnd)
			{
				(*it)->Display();
			}
			act.m_object->Display();

			switch (act.m_action)
			{
				case CDocUndoSet::Deletion:
					// We must re-delete the deleted objects
					if (it != itEnd)
					{
						CDrawingObject *obj = *it;
						obj->NotifyEdit(CDocUndoSet::Deletion);
						delete *it;
						m_drawing.erase(it);
					}
					action_taken = TRUE;
					break;
				case CDocUndoSet::Addition:
				{
					CDrawingObject *obj = Dup(act.m_object);
					// We must re-insert the additions
					m_drawing.insert(it, obj);
					obj->NotifyEdit(CDocUndoSet::Addition);
					action_taken = TRUE;
				}
					break;
				case CDocUndoSet::Change:
					// We convert the old objects into the new objects...
					if (it != itEnd)
					{
						// Keep a copy for the redo...
						CDrawingObject *copy = Dup(*it);
						delete *it;

						copy->Display();
						*it = act.m_object;
						(*it)->NotifyEdit(CDocUndoSet::Change);

						// Action taken when object contents differs
						if (*act.m_object != *copy)
						{
							action_taken = TRUE;
						}

						act.m_object = copy;
					}
					break;
			}

			++act_it;
		}
	}

	// Update the GUI
	ShowModifiedFlag();

	m_InUndoAddAction = 0;
}

// Make a duplicate of an object..
CDrawingObject* CTinyCadDoc::Dup(CDrawingObject *p)
{
	// Now make a duplicate for the Undo/Redo list
	// Don't actualy store the object
	m_DuplicateObjectOnly = TRUE;
	CDrawingObject *pNewObject = p->Store();
	m_DuplicateObjectOnly = FALSE;
	return pNewObject;
}

void CTinyCadDoc::AddUndoAction(CDocUndoSet::action action, CDrawingObject *index_object)
{
	// Don't store Error objects
	if (index_object && index_object->GetType() == xError)
	{
		return;
	}

	// Look up this index...
	int index = 0;
	if (index_object)
	{
		drawingIterator it = GetDrawingBegin();
		drawingIterator itEnd = GetDrawingEnd();
		while (it != itEnd)
		{
			// No undo action possible for indexes higher than any Error object.
			// This is because error objects will be deleted from the drawing
			// which would cause higher index numbers in the UndoSet to become invalid.
			if ( (*it)->GetType() == xError)
			{
				return;
			}

			if (*it == index_object)
			{
				break;
			}

			++it;
			++index;
		}
	}

	m_InUndoAddAction++;

	// Do we need to increment the undo level?
	if (m_change_set || index_object == NULL || m_undo.size() == 0)
	{
		m_change_set = FALSE;
		// Increment the Undo position...
		m_undo_level++;
		FlushRedo();

		if (m_undo_level >= m_undo.size())
		{
			m_undo.resize(m_undo_level + 1);
		}

		CDocUndoSet &s = m_undo[m_undo_level];
		BOOL dirty = m_pParent->CDocument::IsModified();
		s.m_dirty = (BYTE) dirty;
		if (action == CDocUndoSet::Addition || action == CDocUndoSet::Deletion)
		{
			m_pParent->CDocument::SetModifiedFlag(TRUE);
		}
		else
		{
			if (index_object == NULL)
			{
				// Indicate this is a document-saved action
				s.m_dirty = 2;
				m_change_set = TRUE;
				m_InUndoAddAction--;

				// Update the GUI
				ShowModifiedFlag();
				// Nothing more to do here
				return;
			}

			m_pParent->CDocument::SetModifiedFlag(IsModified());
		}

		// Update the GUI
		ShowModifiedFlag();
	}

	index_object->NotifyEdit(action);

	CDocUndoSet &s = m_undo[m_undo_level];

	// Now add this to the back of the undo action list
	CDocUndoSet::CDocUndoAction act;
	act.m_action = action;
	act.m_index = index;
	act.m_object = Dup(index_object);

	s.m_actions.push_back(act);

	m_InUndoAddAction--;
}

void CTinyCadDoc::MarkDeleteForUndo(CDrawingObject *pObject)
{
	AddUndoAction(CDocUndoSet::Deletion, pObject);
}

void CTinyCadDoc::MarkAdditionForUndo(CDrawingObject *pObject)
{
	AddUndoAction(CDocUndoSet::Addition, pObject);
}

void CTinyCadDoc::MarkDocSavedForUndo()
{
	AddUndoAction(CDocUndoSet::Change, NULL);
}

void CTinyCadDoc::MarkChangeForUndo(CDrawingObject* pObject)
{
	AddUndoAction(CDocUndoSet::Change, pObject);
}

void CTinyCadDoc::MarkSelectChangeForUndo()
{
	selectIterator it = GetSelectBegin();
	while (it != GetSelectEnd())
	{
		MarkChangeForUndo(*it);
		++it;
	}

}

// Real Undo action available
BOOL CTinyCadDoc::IsModified()
{
	if (m_pParent->CDocument::IsModified())
	{
		return TRUE;
	}

	BOOL action_taken = FALSE;
	unsigned int undo_level = m_undo_level;

	// Is this possible?
	while (undo_level > 0 && !action_taken)
	{
		// Re-apply all of the changes we have done at this level
		CDocUndoSet &s = m_undo[undo_level];
		if (s.m_dirty == 2) return FALSE;

		if (s.m_dirty)
		{
			action_taken = TRUE;
			break;
		}

		// Go through the list of action and undo each one in the reverse
		// order that they were applied
		CDocUndoSet::actionCollection::reverse_iterator act_it = s.m_actions.rbegin();

		while (act_it != s.m_actions.rend())
		{
			CDocUndoSet::CDocUndoAction &act = *act_it;

			// Look up this item from the index...
			drawingCollection::iterator it = m_drawing.begin();
			drawingCollection::iterator itEnd = m_drawing.end();
			int index = act.m_index;
			while (index > 0 && it != itEnd)
			{
				++it;
				--index;
			}

			switch (act.m_action)
			{
				case CDocUndoSet::Deletion:
					action_taken = TRUE;
					break;

				case CDocUndoSet::Addition:
					action_taken = TRUE;
					break;

				case CDocUndoSet::Change:

					if (it != itEnd)
					{
						// Action taken when object contents differs
						//CDrawingObject *copy = *it;
						if (*act.m_object != **it)
						{
							action_taken = TRUE;
						}
					}
					break;
			}

			++act_it;
		}

		undo_level--;
	}

	return action_taken;
}

// The object selection functions
// This selects objects in a box
// (We don't select construction objects)
void CTinyCadDoc::Select(CDPoint p1, CDPoint p2)
{
	double left = min(p1.x,p2.x);
	double right = max(p1.x,p2.x);
	double top = min(p1.y,p2.y);
	double bottom = max(p1.y,p2.y);

	// Right-to-Left selects only completely surrounded objects
	BOOL lefttoright = (p1.x < p2.x);

	//UnSelect();

	drawingIterator it = GetDrawingBegin();
	drawingIterator itEnd = GetDrawingEnd();
	while (it != itEnd)
	{
		CDrawingObject *obj = *it;

		if ( (lefttoright ? obj->IsInside(left,right,top,bottom) :
		                    obj->IsCompletelyInside(left,right,top,bottom))
			&&
			(obj->GetType() != xJunction || !GetOption().GetAutoJunc() ) )
		{
			obj->Display();
			Select(obj);
		}

		++it;
	}
}

// Select an object
void CTinyCadDoc::Select(CDrawingObject *obj)
{
	m_selected.insert(obj);
}

// Unselect a single object
void CTinyCadDoc::UnSelect(CDrawingObject *p)
{
	selectIterator it = m_selected.find(p);
	if (it != m_selected.end())
	{
		m_selected.erase(it);
	}
}

// Unselect the objects
void CTinyCadDoc::UnSelect()
{
	// Get rid of any currently selected objects
	selectIterator it = GetSelectBegin();
	while (it != GetSelectEnd())
	{
		(*it)->Display();
		++it;
	}

	m_selected.erase(m_selected.begin(), m_selected.end());
}

// Change the Z-order of the selected objects
void CTinyCadDoc::BringToFront()
{
	if (!IsSelected()) return;

	drawingCollection selectedObjects;
	drawingIterator it = GetDrawingBegin();
	//(Don't prefetch GetDrawingEnd here)
	while (it != GetDrawingEnd())
	{
		drawingIterator current = it;
		++it;

		CDrawingObject *pointer = *current;
		if (IsSelected(pointer))
		{
			MarkDeleteForUndo(pointer);
			m_drawing.erase(current);
			selectedObjects.push_back(pointer);
			// Reset iterator because it was invalidated
			it = GetDrawingBegin();
		}
	}

	// Now push them back on (in the right order)
	drawingCollection::iterator itx = selectedObjects.begin();
	while (itx != selectedObjects.end())
	{
		m_drawing.push_back(*itx);
		MarkAdditionForUndo(*itx);
		(*itx)->Display();
		++itx;
	}

}

BOOL CTinyCadDoc::IsInDrawing(CDrawingObject *obj)
{
	drawingIterator it = GetDrawingBegin();
	drawingIterator itEnd = GetDrawingEnd();
	while (it != itEnd)
	{
		if (obj == *it)
		{
			return TRUE;
		}
		++it;
	}

	return FALSE;
}

// Change the Z-order of the selected objects
void CTinyCadDoc::SendToBack()
{
	if (!IsSelected()) return;

	drawingCollection selectedObjects;
	drawingIterator it = GetDrawingBegin();
	//(Don't prefetch GetDrawingEnd here)
	while (it != GetDrawingEnd())
	{
		drawingIterator current = it;
		++it;

		CDrawingObject *pointer = *current;
		if (IsSelected(pointer))
		{
			MarkDeleteForUndo(pointer);
			m_drawing.erase(current);
			selectedObjects.push_back(pointer);
			// Reset iterator because it was invalidated
			it = GetDrawingBegin();
		}
	}

	// Now re-insert them at the front
	m_drawing.insert(m_drawing.begin(), selectedObjects.begin(), selectedObjects.end());
}

// Remove an item from the drawing...
void CTinyCadDoc::Delete(drawingIterator it)
{
	CDrawingObject *pointer = *it;

	pointer->Display();
	if (pointer == GetSelectable())
	{
		SetSelectable(NULL);
	}
	UnSelect(pointer);

	MarkDeleteForUndo(*it);
	delete *it;
	m_drawing.erase(it);

	SetModifiedFlag(TRUE);
}

// Remove an item from the drawing...
void CTinyCadDoc::Delete(CDrawingObject *p)
{
	drawingIterator it = GetDrawingBegin();
	//(Don't prefetch GetDrawingEnd here)
	while (it != GetDrawingEnd())
	{
		drawingIterator current = it;

		CDrawingObject *pointer = *current;
		if (pointer == p)
		{
			Delete(current);
			break;
		}

		++it;
	}
}

// Delete the selected objects!
void CTinyCadDoc::SelectDelete()
{
	if (!IsSelected()) return;

	CJunctionUtils j(this);

	drawingIterator it = GetDrawingBegin();
	//(Don't prefetch GetDrawingEnd here)
	while (it != GetDrawingEnd())
	{
		drawingIterator current = it;
		++it;

		CDrawingObject *pointer = *current;
		if (IsSelected(pointer))
		{
			if (pointer == GetSelectable())
			{
				SetSelectable(NULL);
			}

			j.AddObjectToTodo(pointer);
			MarkDeleteForUndo(pointer);
			m_drawing.erase(current);
			pointer->Display();
			delete pointer;
			// Reset iterator because it was invalidated
			it = GetDrawingBegin();
		}
	}

	m_selected.erase(m_selected.begin(), m_selected.end());

	// ... and perform the junction requirements...
	j.CheckTodoList(true);

	SetModifiedFlag(TRUE);
}

// Place the Selected objects into the Undo List in the original position
void CTinyCadDoc::SelectUndoMove(CDPoint r)
{

	if (!IsSelected()) return;

	selectIterator it = GetSelectBegin();
	while (it != GetSelectEnd())
	{
		CDrawingObject *obj = *it;

		// Now move the object
		obj->m_point_a.x += r.x;
		obj->m_point_a.y += r.y;
		obj->m_point_b.x += r.x;
		obj->m_point_b.y += r.y;

		// Make a copy of the selected objects (so as to perform add/delete operation for Redo List)
		MarkChangeForUndo(obj);

		// Now move the object back...
		obj->m_point_a.x -= r.x;
		obj->m_point_a.y -= r.y;
		obj->m_point_b.x -= r.x;
		obj->m_point_b.y -= r.y;

		++it;
	}

	SetModifiedFlag(TRUE);

}

// Load a new design
BOOL CTinyCadDoc::Import(BOOL select_import_object)
{
	CFileDialog dlg(TRUE, _T("*.dsn"), NULL, OFN_HIDEREADONLY, _T("Designs (*.dsn)|*.dsn|All files (*.*)|*.*||"), AfxGetMainWnd());

	if (dlg.DoModal() != IDOK) return FALSE;

	// Do we use the text loading procedure or load from a CArchive?
	CFile theFile;
	if (!theFile.Open(dlg.GetPathName(), CFile::modeRead))
	{
		// Could not open file to start loading
		Message(IDS_ABORTLOAD, MB_ICONEXCLAMATION);
		return FALSE;
	}

	CStreamFile stream(&theFile, CArchive::load);
	BOOL r = Import(stream);

	if (r && select_import_object)
	{
		CDrawBlockImport *pImport = new CDrawBlockImport(this);
		selectCollection sel = m_selected;
		SelectObject(pImport);
		m_selected = sel;
		pImport->Import();
	}

	return r;
}

void CTinyCadDoc::ForceSetCursor()
{
	m_pParent->UpdateAllViews(NULL, DOC_UPDATE_SETCURSOR, NULL);
}

// Redraw our window
void CTinyCadDoc::Invalidate()
{
	if (m_pParent)
	{
		m_pParent->UpdateAllViews(NULL, DOC_UPDATE_INVALIDATE, NULL);
	}
}

// Redraw the rulers
void CTinyCadDoc::InvalidateRulers()
{
	if (m_pParent)
	{
		m_pParent->UpdateAllViews(NULL, DOC_UPDATE_RULERS, NULL);
	}
}

void CTinyCadDoc::InvalidateRect(CDRect r, BOOL erase, int grow, BOOL outline_only)
{
	if (outline_only)
	{
		CDRect r1(r.left, r.top, r.right, r.top);
		CDRect r2(r.right, r.top, r.right, r.bottom);
		CDRect r3(r.right, r.bottom, r.left, r.bottom);
		CDRect r4(r.left, r.bottom, r.left, r.top);
		InvalidateRect(r1, erase, grow);
		InvalidateRect(r2, erase, grow);
		InvalidateRect(r3, erase, grow);
		InvalidateRect(r4, erase, grow);
	}
	else
	{
		doc_invalidrect hint;
		hint.r = r;
		hint.grow = grow;
		if (m_pParent)
		{
			m_pParent->UpdateAllViews(NULL, erase ? DOC_UPDATE_INVALIDRECTERASE : DOC_UPDATE_INVALIDRECT, &hint);
		}
	}
}

// No obvious answer to this one, but it is
// a function that should go in here...
CDSize CTinyCadDoc::GetTextExtent(const TCHAR *str, int style)
{
	Transform dummy;
	CContext dc(AfxGetMainWnd(), dummy);
	dc.SelectFont(*GetOptions()->GetFont(static_cast<hFONT>(style)), 0);
	return dc.GetTextExtent(str);
}

// Select/add a new symbol for editing
void CTinyCadDoc::SelectSymbol(CLibraryStoreSymbol *theSymbol)
{
	SelectObject(new CDrawEditItem(this));
	hSYMBOL symbol = GetOptions()->AddSymbol(theSymbol->GetDesignSymbol(this));
	CDrawMethod *NewMethod = new CDrawMethod(this, symbol, theSymbol->m_pParent->orientation);

	SelectObject(NewMethod);
}

// Select a new editing object
void CTinyCadDoc::SelectObject(CDrawingObject *NewO)
{
	if (edit)
	{
		CDrawingObject *pEdited = GetSingleSelectedItem();

		edit->EndEdit();
		delete edit;

		if (pEdited && pEdited->IsEmpty()) {
			// Remove empty object from the document. Otherwise empty labels and
			// text boxes will stay on the canvas as invisible objects that
			// confuse user when they are inadvertently clicked and selected.
			Delete(pEdited);
		}
	}

	if (NewO)
	{
		// Not when it is an editing object
		if (NewO->GetType() != xEditItem && NewO->GetType() != xAnnotation)
		{
			BeginNewChangeSet();
		}
		NewO->BeginEdit(FALSE);
	}

	ShowModifiedFlag();

	edit = NewO;
}

// Draw the design rulers and the details box
void CTinyCadDoc::Display(CContext& dc)
{
	CString pathName;
	CString fileName;
	CString preFix = L"";
	int startChar;
	int maxDisplayCharacters = 45;	//see note below
	CString ellipsisString = L" ... ";
	//Set preFix string to first 45 or fewer characters and append ellipsis string if necessary
	//Note that since this is a proportionally spaced font, it still may not fit and will be right justified and
	//truncated at the far left of the display space.  The length in characters of a proportionally spaced font
	//is very difficult to calculate and this class is not aware of the font and font size to be used.

	pathName = m_pParent->GetPathName();
	startChar = pathName.ReverseFind('\\') + 1;	//Move one character past the backslash
	fileName = pathName.Right(pathName.GetLength() - startChar);
	if (fileName.GetLength() > 50)
	{
		fileName = fileName.Left(50);
	}

	if (fileName.GetLength() < 1)
	{	//No backslash character found in pathName so use the full path name
		fileName = pathName;
		preFix = L"";
	}
	else
	{
		preFix = pathName.Left(pathName.GetLength() - fileName.GetLength());

		//If you add the ellipsis string, you must account for the extra length of the ellipsis string
		if (preFix.GetLength() > maxDisplayCharacters - ellipsisString.GetLength() - fileName.GetLength())
		{
			//if the calculated left string length is negative, the .Left() function will set it to 0
			preFix = preFix.Left(maxDisplayCharacters - ellipsisString.GetLength() - fileName.GetLength()) + ellipsisString;
		}
	}

	// Prior to TinyCAD 2.90.00, the full path was displayed, but frequently there wasn't room and it overran its allocated drawing space
	//	GetDetails().Display(dc, theOptions, m_pParent->GetPathName());
	GetDetails().Display(dc, theOptions, preFix + fileName);
}

void CTinyCadDoc::SetSelectable(CDrawingObject *obj)
{
	// Only redraw if the selectable changes
	if (obj != selectable)
	{
		if (selectable)
		{
			selectable->Display(FALSE);	//djl - something different is needed here - selectable can sometimes be uninitialized and cause an exception at this statement, but I don't how.
		}

		selectable = obj;

		if (selectable)
		{
			selectable->Display(FALSE);
		}
	}
}

CDrawingObject* CTinyCadDoc::GetSelectable()
{
	return selectable;
}

#if 0
BOOL CTinyCadDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	// Is this an emf file?
	CString path = lpszPathName;
	CString extension;
	int brk = path.ReverseFind('.');
	if (brk != -1)
	{
		extension = path.Mid(brk);
	}
	if (extension.CompareNoCase(".emf") == 0)
	{
		// Load a wmf
		HENHMETAFILE data = (HENHMETAFILE)::GetEnhMetaFile( lpszPathName );
		CDrawMetaFile *pObject = new CDrawMetaFile( this );
		pObject->setMetaFile( data );
		Add( pObject );
	}
	else
	{
		// Load normally
		if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	}

	return TRUE;
}
#endif

// Called after a paste or import to enable the
// document to sort out the imported block when
// necessary
void CTinyCadDoc::PostPaste()
{
}

// Called after a paste or import to enable the
// document to sort out the imported block when
// necessary
void CTinyCadDoc::UngroupSymbols()
{
	// Scan and convert any imported symbols
	// into their component parts
	drawingIterator it = GetDrawingBegin();
	bool specialEndOfDrawing = false;

	//(Don't prefetch GetDrawingEnd here)
	while (specialEndOfDrawing == false)
	{
		if (it != GetDrawingEnd())
		{
			drawingIterator current = it;
			++it;

			specialEndOfDrawing = (it == GetDrawingEnd());

			CDrawingObject *pObject = *current;

			// Is this a method object?
			if (pObject->GetType() == xMethodEx3 && IsSelected(pObject))
			{
				// Convert to the actual type
				CDrawMethod *pMethod = static_cast<CDrawMethod*> (pObject);

				// Get the symbol data
				CDPoint tr;
				drawingCollection method;
				pMethod->ExtractSymbol(tr, method);

				// Remove the method from the linked list
				UnSelect(pMethod);
				Delete(pMethod);

				// Now re-insert using the offset of the main
				// method
				CDPoint offset = method.front()->m_point_a;
				for (drawingIterator it = method.begin(); (it != method.end());)
				{
					CDrawingObject *pInsertObject = *it;
					CDrawingObject *pDup = pInsertObject->Store();

					pDup->m_point_a += offset;
					pDup->m_point_b += offset;

					Select(pDup);

					++it;
					specialEndOfDrawing |= (it != method.end());
				}
			}
		}
		else
		{
			specialEndOfDrawing = true;
		}
	}
}

CDPoint CTinyCadDoc::GetStickyPoint(CDPoint no_snap_q, BOOL pins, BOOL wires, BOOL &is_stuck, BOOL &is_junction)
{
	CDPoint r(0, 0);
	bool first = true;
	int items = 0;
	double min_distance = 0;
	double range = GetOption().GetAutoSnapRange();

	CDPoint q = m_snap.Snap(no_snap_q);
	CDPoint q_snap_x = CDPoint(q.x, no_snap_q.y);
	CDPoint q_snap_y = CDPoint(no_snap_q.x, q.y);

	if (GetTransform().GetZoomFactor() > 1.0)
	{
		range /= GetTransform().GetZoomFactor();

		// Range can not be smaller than 0.5*SQRT(1^2+1^2)*Grid = 0.7071*Grid
		// Which is the maximum distance from a snap point.
		// Otherwise it can snap to the correct point but still outside the range.
		if (range < m_snap.GetGrid() * 0.71)
		{
			range = m_snap.GetGrid() * 0.71;
		}
	}

	if (!GetOption().GetAutoSnap())
	{
		is_stuck = FALSE;
		is_junction = FALSE;
		return q;
	}

	// Search for methods, and look at their pins
	drawingIterator it = GetDrawingBegin();
	drawingIterator itEnd = GetDrawingEnd();
	while (it != itEnd)
	{
		CDrawingObject *ObjPtr = *it;

		switch (ObjPtr->GetType())
		{
			case xWire:
#define theLine ((CDrawLine*)ObjPtr)
				if (wires)
				{
					CDPoint d;
					CDPoint d2;
					double distance;
					double distance2;

					CLineUtils l(theLine->m_point_a, theLine->m_point_b);

					// Diagonal lines
					if (l.IsDiagonal())
					{
						// Snap on X or Y position
						// whichever is closest to the line
						distance = l.DistanceFromPointY(q_snap_x, d);
						distance2 = l.DistanceFromPointX(q_snap_y, d2);

						if (distance2 < distance)
						{
							distance = distance2;
							d = d2;
						}
					}
					else
					{
						distance = l.DistanceFromPoint(q, d);
					}

					// Calculate real distance from sticky point to mouse position
					double dx = d.x - no_snap_q.x;
					double dy = d.y - no_snap_q.y;
					distance = sqrt(dx * dx + dy * dy);

					if (d == r)
					{
						items++;
					}

					if (first || distance < min_distance)
					{
						if (r != d)
						{
							// have we split this wire?
							if (d != theLine->m_point_a && d != theLine->m_point_b)
							{
								items = 2;
							}
							else
							{
								items = 1;
							}
						}
						r = d;
						first = false;
						min_distance = distance;
					}
				}
				break;
			default:
				if (pins)
				{
					CDRect s(ObjPtr->m_point_a.x, ObjPtr->m_point_a.y, ObjPtr->m_point_b.x, ObjPtr->m_point_b.y);
					s.NormalizeRect();
					s.left -= range;
					s.right += range;
					s.bottom += range;
					s.top -= range;

					if (s.PtInRect(no_snap_q))
					{
						CActiveNode a;
						ObjPtr->GetActiveListFirst(a);
						while (ObjPtr->GetActive(a))
						{
							// This is a valid pin...
							CDPoint d = a.m_a;
							double dx = d.x - no_snap_q.x;
							double dy = d.y - no_snap_q.y;
							double distance = sqrt(dx * dx + dy * dy);

							if (r == d)
							{
								items++;
							}

							if (first || distance < min_distance)
							{
								if (r != d)
								{
									items = 1;
								}

								r = d;
								first = false;
								min_distance = distance;
							}
						}
					}
				}
				break;
		}

		++it;
	}

	if (!first && min_distance <= range)
	{
		is_stuck = TRUE;
		is_junction = GetOption().GetAutoJunc() ? items > 1 : FALSE;
		return r;
	}
	else
	{
		is_junction = FALSE;
		is_stuck = FALSE;
	}

	return q;
}

// Replace all of the symbols in our drawing with a different one...
void CTinyCadDoc::ReplaceSymbol(hSYMBOL old_symbol, hSYMBOL new_symbol, bool keep_old_fields)
{
	// Search for methods, and look at their pins
	drawingIterator it = GetDrawingBegin();
	while (it != GetDrawingEnd())
	{
		CDrawingObject *ObjPtr = *it;
		if (ObjPtr->GetType() == xMethodEx3)
		{
			CDrawMethod *pMethod = static_cast<CDrawMethod*> (ObjPtr);
			pMethod->ReplaceSymbol(old_symbol, new_symbol, keep_old_fields);
			pMethod->Display(TRUE);
		}

		++it;
	}
}

// Duplicate the selected objects
void CTinyCadDoc::SelectDup()
{
	drawingCollection newSelection;

	selectIterator it = GetSelectBegin();
	while (it != GetSelectEnd())
	{
		CDrawingObject *obj = *it;

		CDrawingObject *pNewObject = obj->Store();
		newSelection.push_back(pNewObject);

		obj->Display();
		++it;
	}

	// Swap over the selection to the new
	// objects...
	UnSelect();
	drawingCollection::iterator itx = newSelection.begin();
	while (itx != newSelection.end())
	{
		Select(*itx);

		// ... and mark for change...
		MarkChangeForUndo(*itx);
		++itx;
	}
}

// Move the selected items (the CPoint is the relative shift)
void CTinyCadDoc::SelectMove(CDPoint r)
{
	CJunctionUtils j(this);

	if (!IsSelected()) return;

	// Remove all the objects from the screen
	selectIterator it = GetSelectBegin();
	while (it != GetSelectEnd())
	{
		CDrawingObject *obj = *it;

		// Redraw it's old position
		obj->Display();

		// Add this object to the todo list..
		j.AddObjectToTodo(obj);

		// Move the object
		obj->Shift(r);

		// Add the new position to the todo list
		j.AddObjectToTodo(obj);

		obj->Display();

		++it;
	}

	j.CheckTodoList(false);
}

// Select all objects
void CTinyCadDoc::SelectAll()
{
	drawingIterator it = GetDrawingBegin();
	drawingIterator itEnd = GetDrawingEnd();
	while (it != itEnd)
	{
		CDrawingObject *obj = *it;

		obj->Display();
		Select(obj);

		++it;
	}
}

void CTinyCadDoc::AddNewJunction(CDPoint q)
{
	// A junction is required, create it!
	CDrawJunction *junction = new CDrawJunction(this);
	junction->m_point_a = q;
	junction->m_point_b = q;

	m_drawing.push_back(junction);
	MarkAdditionForUndo(junction);
	junction->Display();
}

//-------------------------------------------------------------------------
CDetails& CTinyCadDoc::GetDetails()
{
	return m_oDetails;
}
//-------------------------------------------------------------------------
// Get the current options for this document
COption *CTinyCadDoc::GetOptions()
{
	return &theOptions;
}
//-------------------------------------------------------------------------
// Get the current options for this document
COption& CTinyCadDoc::GetOption()
{
	return theOptions;
}

void CTinyCadDoc::ShowModifiedFlag()
{
	if (m_pParent)
	{
		// Update the window titles
		m_pParent->DelayUpdateFrameTitle();
	}
}

//-------------------------------------------------------------------------
void CTinyCadDoc::SetModifiedFlag(BOOL bModified)
{
	if (m_pParent)
	{
		if (!m_InUndoAddAction)
		{
			// bModified == FALSE means the document has just
			// been saved or loaded.
			if (bModified == FALSE)
			{
				// Tell MFC this document is not modified anymore
				m_pParent->SetModifiedFlag(FALSE);
				// And store this state in the undo buffer
				MarkDocSavedForUndo();
			}
		}

		// Update the 'Modified' flag in the window caption
		ShowModifiedFlag();
	}
}

//-------------------------------------------------------------------------
CString CTinyCadDoc::GetSheetName() const
{
	return m_sheet_name;
}
//-------------------------------------------------------------------------
bool CTinyCadDoc::IsHierarchicalSymbol() const
{
	return false;
}
//-------------------------------------------------------------------------
void CTinyCadDoc::SetSheetName(const TCHAR *n)
{
	m_sheet_name = n;
}
//-------------------------------------------------------------------------
void CTinyCadDoc::AddImage(CDrawMetaFile *pObject)
{
	Add(pObject);
	UnSelect();
	CDrawBlockImport *pImport = new CDrawBlockImport(this);
	SelectObject(pImport);
	Select(pObject);
	pImport->Import();
}
//-------------------------------------------------------------------------

// Format a filename for display
CString CTinyCadDoc::formatFilename(const TCHAR *filename)
{
	TCHAR *brk = (TCHAR *) _tcsrchr(filename, '\\');
	if (brk)
	{
		return brk + 1;
	}
	else
	{
		return filename;
	}
}
//-------------------------------------------------------------------------

// Format a filename for saving
CString CTinyCadDoc::formatXMLFilename(const TCHAR *filename)
{

	TCHAR out[MAX_PATH];
	if (PathRelativePathTo(out, m_pParent->GetXMLPathName(), 0, filename, 0))
	{
		return out;
	}
	else
	{
		return filename;
	}
}

//-------------------------------------------------------------------------

// Unformat a filename after loading
CString CTinyCadDoc::unformatXMLFilename(const TCHAR *filename)
{
	TCHAR out[MAX_PATH];

	CString s = m_pParent->GetXMLPathName();
	int brk = s.ReverseFind('\\');
	if (brk != -1)
	{
		s = s.Left(brk);
	}

	if (PathCombine(out, s, filename))
	{
		return out;
	}
	else
	{
		return filename;
	}
}
//-------------------------------------------------------------------------

CString CTinyCadDoc::getDefaultReferenceString() const
{
	// this is important for documents which are
	// used as embedded hierarchical designs
	return "H?";
}
