/*
	TinyCAD program for schematic capture
	Copyright 1994/1995/2002 Matt Pyne.

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

#if !defined(AFX_TINYCADDOC_H__34C3ADE5_C40F_411E_B21B_E123312BD3BE__INCLUDED_)
#define AFX_TINYCADDOC_H__34C3ADE5_C40F_411E_B21B_E123312BD3BE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "context.h"
#include "object.h"
#include "Library.h"
#include "details.h"
#include "DSize.h"
class CTinyCadView;

// The update hints
#define		DOC_UPDATE_INVALIDATE		100
#define		DOC_UPDATE_INVALIDRECTERASE	101
#define		DOC_UPDATE_INVALIDRECT		102
#define		DOC_UPDATE_SETCURSOR		103
#define		DOC_UPDATE_TABS				104

struct doc_invalidrect : public CObject
{
	CDRect r;
	int grow;
};

typedef std::set<CDrawingObject*> selectCollection;
typedef selectCollection::iterator selectIterator;

class CDocUndoSet
{
public:

	enum action
	{
		Addition,
		Deletion,
		Change
	};

	class CDocUndoAction
	{
	public:
		action			m_action;
		int				m_index;
		CDrawingObject*	m_object;
	};

	typedef std::list<CDocUndoAction> actionCollection;
	actionCollection	m_actions;

};

typedef std::vector<CDocUndoSet>	 undoCollection;


class CMultiSheetDoc;


class CTinyCadDoc 
{

// Operations
public:

// Implementation
public:
	CTinyCadDoc(CMultiSheetDoc*pParent = NULL);
	virtual ~CTinyCadDoc();

protected:

	// The current object being used to draw on this object
	CDrawingObject	*edit;			// The object currently being edited

	int 	NameDir;				// +ve/-ve name increments when repeating
	int 	PinDir;					// +ve/-ve pin number increments when repeating

	CDrawingObject	*selectable;	// The object that will be selected when the user clicks

	CMultiSheetDoc	*m_pParent;

	CString	m_sheet_name;			// The name of this sheet

	// Create the options
	COption theOptions;

	CDetails			m_oDetails;

	Transform m_Transform;			// The transform of the design to the window


protected:
	drawingCollection	m_drawing;
	selectCollection	m_selected;
	unsigned int		m_undo_level;
	undoCollection		m_undo;
	BOOL				m_change_set;
	BOOL				m_InUndoAddAction;

public:
	CDrawingObject*		Dup( CDrawingObject *p );
	void				AddUndoAction( CDocUndoSet::action action, CDrawingObject *index_object );

	Transform&			GetTransform() { return m_Transform; }

	/////////////////////////////////////////////////////////////////////////////
	//
	// Drawing collection operators
	//

	// Iterate through the drawing
	drawingIterator GetDrawingBegin();
	drawingIterator GetDrawingEnd();
	
	// Add an object to this drawing
	void Add( CDrawingObject * );
	void Add( drawingCollection& drawing );
	void Delete( drawingIterator it );
	void Delete( CDrawingObject * );

	/////////////////////////////////////////////////////////////////////////////
	//
	// Insert an image
	//
	void AddImage( CDrawMetaFile *pObject );

	/////////////////////////////////////////////////////////////////////////////
	//
	// Sheet operators
	//
	virtual CString	GetSheetName() const;
	virtual void 	SetSheetName( const TCHAR *sName );
	virtual bool	IsHierarchicalSymbol() const;
	CMultiSheetDoc	*GetParent() { return m_pParent; }

	/////////////////////////////////////////////////////////////////////////////
	//
	// Selection operators
	//

	// Iterate through the selected objects
	selectIterator GetSelectBegin();
	selectIterator GetSelectEnd();
	BOOL IsSelected( CDrawingObject * p ) const;
	BOOL IsSingleItemSelected() const;
	BOOL IsSelected() const;
	CDrawingObject* GetSingleSelectedItem();

	// Select an area
	void Select(CDPoint,CDPoint);

	// Select an object
	void Select(CDrawingObject *);

	// Unselect all objects
	void UnSelect();

	// Unselect a single object
	void UnSelect( CDrawingObject *p );

	// Delete the selected objects
	void SelectDelete();

	// Move the selected objects
	void SelectMove(CDPoint);

	// Duplicate the selected objects
	void SelectDup();

	// Undo the move of the selected objects
	void SelectUndoMove(CDPoint);

	// Change the Z-order of the selected objects
	void BringToFront();
	void SendToBack();

	// Is this object still in the drawing (to compenstate for different things
	// that can delete objects in drawings, whilst others have pointers to them)
	BOOL IsInDrawing( CDrawingObject *obj );

	CDrawingObject* GetSelectable();
	void SetSelectable( CDrawingObject *obj );

	// The current grid settings
	TransformSnap	m_snap;

	// Is this document editing a library?
	virtual bool IsLibInUse( CLibraryStore *lib );
	void Initalize(CTinyCadView *window);

	void SetModifiedFlag(BOOL Changed = TRUE);
	CDetails&	GetDetails();

public:
	void Display( CContext & );

	BOOL Import( BOOL select_import_object );
	BOOL Import( CStream& );
	BOOL ReadFile(CStream &theArchive);
	BOOL ReadFileXML(CXMLReader &xml, BOOL AlreadyStarted );
	BOOL ReadFile(CStream &theArchive, BOOL Details, drawingCollection &drawing);
	BOOL ReadFileXML(CXMLReader &xml, BOOL Details, drawingCollection &drawing, BOOL AlreadyStarted );
	BOOL Save(BOOL GetName = TRUE, BOOL SaveSelect = FALSE);
	void SaveXML(CXMLWriter&, drawingCollection &drawing, BOOL Details = FALSE, BOOL SaveSelect = FALSE, BOOL SaveResources = TRUE);
	void SaveXML(CXMLWriter&, BOOL Details = FALSE, BOOL SaveSelect = FALSE);

	virtual CString GetXMLTag();


	void DeleteErrors();
	void TagAllResources();

	// The Undo/Redo commands
	void BeginNewChangeSet();
	void MarkChangeForUndo( CDrawingObject* pObject );
	void MarkDeleteForUndo( CDrawingObject *pObject );
	void MarkAdditionForUndo( CDrawingObject *pObject );
	void MarkSelectChangeForUndo();

	void Undo();
	void Redo();

	void FlushRedo();						// Remove the Redo Buffer


	BOOL CanUndo() const { return m_undo_level > 0; }
	BOOL CanRedo() const { return m_undo_level + 1 < static_cast<int>(m_undo.size()); }


	// Replace all of the symbols in our drawing with a different one...
	void ReplaceSymbol( hSYMBOL old_symbol, hSYMBOL new_symbol, bool keep_old_fields );


	// Redraw our window
	void Invalidate();
	void InvalidateRect( CDRect r, BOOL erase, int grow, BOOL outline_only = FALSE );
	void ForceSetCursor();


	// Set the next repeat object for the editing view
	int GetNameDir() { return NameDir; }
	int GetPinDir() { return PinDir; }
	void SetNameDir( int i ) { NameDir = i; }
	void SetPinDir( int i ) { PinDir = i; }

	// Snap to grid on/oof
	BOOL GetSnapToGrid() { return m_snap.GetGridSnap(); }
	void SetSnapToGrid( BOOL r ) { m_snap.SetGridSnap(r); }

	// Select the editing object
	void SelectObject(CDrawingObject *NewO);

	// Select/add a new symbol for editing
	virtual void SelectSymbol( CLibraryStoreSymbol *theSymbol );

	// Get the current edit object
	CDrawingObject *GetEdit() { return edit; }

	// Edit a library symbol
	virtual BOOL IsEditLibrary() { return FALSE; }


	// Get the nearest point that matches that attaches
	// to a schema point...
	CDPoint GetStickyPoint( CDPoint q, BOOL pins, BOOL wires, BOOL &is_stuck, BOOL &is_junction );

	// Add a new junction to the drawing at the specified point
	void AddNewJunction( CDPoint q );

	// No obvious answer to this one, but it is a function that should go in here...
	CDSize GetTextExtent(const TCHAR *str, int style);

	// Get the current options for this document
	// Now deprecated, use reference semantic from GetOption() instead
	COption*	GetOptions();

	// Get the current options for this document
	COption& 	GetOption();

	// Save as a PNG file
	void SavePNG( const TCHAR *file_name, CDC &ref_dc, int scaling, bool bw, bool rotate );

	// Create an enhanced metafile from this function
	HENHMETAFILE CreateMetafile( CDC &ref_dc, const TCHAR *file_name, bool bw );

	// Format a filename for display
	CString formatFilename( const TCHAR *filename );

	// Format a filename for saving
	CString formatXMLFilename( const TCHAR *filename );

	// Unformat a filename after loading
	CString unformatXMLFilename( const TCHAR *filename );

	// Used by derived classes to break apart placed
	// symbols
	void UngroupSymbols();

	// Called after a paste or import to enable the
	// document to sort out the imported block when
	// necessary
	virtual void PostPaste();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TINYCADDOC_H__34C3ADE5_C40F_411E_B21B_E123312BD3BE__INCLUDED_)
