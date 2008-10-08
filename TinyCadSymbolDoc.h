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


// TinyCadSymbolDoc.h: interface for the CTinyCadSymbolDoc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TINYCADSYMBOLDOC_H__843EC4AD_D70A_4DB4_865D_633006592CCB__INCLUDED_)
#define AFX_TINYCADSYMBOLDOC_H__843EC4AD_D70A_4DB4_865D_633006592CCB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TinyCadDoc.h"

class CTinyCadSymbolDoc : public CTinyCadDoc
{
	int	m_part;

public:
	void setSymbol();
	CTinyCadSymbolDoc(CMultiSheetDoc *pParent = NULL);
	virtual ~CTinyCadSymbolDoc();

	virtual BOOL IsEditLibrary() { return TRUE; }

	// Select/add a new symbol for editing
	virtual void SelectSymbol( CLibraryStoreSymbol *theSymbol );

	// Set which part in the package to edit
	void EditPartInPackage( int p );

	// How many parts per package are there currently?
	int GetPartsPerPackage();

	// Enforce parts per package
	void SetPartsPerPackage(int p);


	// Get the current part number
	int GetPart() const
	{ 
		return m_part; 
	}


	// Called after a paste or import to enable the
	// document to sort out the imported block when
	// necessary
	virtual void PostPaste();
};

#endif // !defined(AFX_TINYCADSYMBOLDOC_H__843EC4AD_D70A_4DB4_865D_633006592CCB__INCLUDED_)
