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

// TinyCadHierarchicalDoc.cpp : implementation file
//

#include "stdafx.h"
#include "TinyCad.h"
#include "TinyCadHierarchicalDoc.h"


// CTinyCadHierarchicalDoc

CTinyCadHierarchicalDoc::CTinyCadHierarchicalDoc(CMultiSheetDoc*pParent)
: CTinyCadDoc( pParent )
{
}

CTinyCadHierarchicalDoc::~CTinyCadHierarchicalDoc()
{
}

CString	CTinyCadHierarchicalDoc::GetSheetName() const
{
	return "Hierarchical Symbol";
}

void CTinyCadHierarchicalDoc::SetSheetName( const TCHAR *sName )
{
}

bool CTinyCadHierarchicalDoc::IsHierarchicalSymbol() const
{
	return true;
}

CString CTinyCadHierarchicalDoc::GetXMLTag()
{
	return "HierarchicalSymbol";    //Historically, this was misspelled "HierachicalSymbol" and has been fixed in many places in a backwards compatible way
}


// Called after a paste or import to enable the
// document to sort out the imported block when
// necessary
void CTinyCadHierarchicalDoc::PostPaste()
{
	UngroupSymbols();
}

// Select/add a new symbol for editing
void CTinyCadHierarchicalDoc::SelectSymbol( CLibraryStoreSymbol *theSymbol )
{
	SelectObject( NULL );

	CDesignFileSymbol *pSymbol = theSymbol->GetDesignSymbol( this );

	drawingCollection method;
	if (pSymbol)
	{
		pSymbol->GetMethod( 0, false, method );

		drawingIterator it = method.begin();
		while (it != method.end())
		{
			CDrawingObject *q = Dup(*it);
			Add( q );
			Select( q );
			++ it;
		}

		delete pSymbol;
	}	

	CDrawBlockImport *pImport = new CDrawBlockImport(this);
	SelectObject(pImport); 
	pImport->Import();
}

CString CTinyCadHierarchicalDoc::getDefaultReferenceString() const
{
	return CTinyCadDoc::getDefaultReferenceString();
}
