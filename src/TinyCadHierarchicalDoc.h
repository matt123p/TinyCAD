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

#pragma once


// CTinyCadHierarchicalDoc document

#include "TinyCadDoc.h"

class CTinyCadHierarchicalDoc : public CTinyCadDoc
{
public:
	CTinyCadHierarchicalDoc(CMultiSheetDoc*pParent = NULL);
	virtual ~CTinyCadHierarchicalDoc();

	/////////////////////////////////////////////////////////////////////////////
	//
	// Sheet operators
	//
	virtual CString	GetSheetName() const;
	virtual void 	SetSheetName( const TCHAR *sName );
	virtual bool	IsHierarchicalSymbol() const;
	virtual CString GetXMLTag();

	// Called after a paste or import to enable the
	// document to sort out the imported block when
	// necessary
	virtual void PostPaste();

	// Select/add a new symbol for editing
	virtual void SelectSymbol( CLibraryStoreSymbol *theSymbol );

	// Unassigned reference designator:
	virtual CString getDefaultReferenceString() const;

protected:
};
