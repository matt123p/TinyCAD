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

#pragma once
#include "drawingobject.h"

class CDrawHierarchicalSymbol :
	public CDrawMethod
{

protected:
	CString				m_Filename;

	// The loaded data
	bool				m_Loaded;
	drawingCollection	m_Symbol;
	CDPoint				m_tr;

	void SetFilenameField();
	void clearSymbol();

	friend class CEditDlgHierarchicalEdit;

public:
	CDrawHierarchicalSymbol(CTinyCadDoc *pDesign);
	CDrawHierarchicalSymbol(const CDrawHierarchicalSymbol& b);
	virtual ~CDrawHierarchicalSymbol(void);

	// Get the definition of this symbol
	virtual BOOL ExtractSymbol( CDPoint &tr, drawingCollection &method );
	virtual BOOL IsNoSymbol();

	virtual CDrawingObject* Store();

	virtual ObjType GetType();			// Get this object's type
	virtual int getMenuID();
	virtual int GetContextMenu();
	virtual void ContextMenu( CDPoint p, UINT id );


	// Load a new drawing
	BOOL SelectFile();
	BOOL Load( const TCHAR *filename );

	// Show this on the screen
	virtual void Paint(CContext &,paint_options);

	// Editing
	virtual void BeginEdit(BOOL re_edit);
	virtual void EndEdit();

	// Get the filename associated with this symbol
	CString	GetFilename();

	// The copy operator
	CDrawHierarchicalSymbol& operator=( const CDrawHierarchicalSymbol& b );

	// Loading and saving
	virtual void SaveXML( CXMLWriter &xml );
	virtual void LoadXML( CXMLReader &xml );
	static const CString GetXMLTag();
	static const CString GetAltXMLTag();	//supports old misspelled version of keyword
};
