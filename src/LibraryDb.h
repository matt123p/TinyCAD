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

#if !defined(AFX_LIBRARYDB_H__0CCD1C42_BEDE_4447_95C4_0C8B663415CB__INCLUDED_)
#define AFX_LIBRARYDB_H__0CCD1C42_BEDE_4447_95C4_0C8B663415CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Symbol.h"
#include "LibraryStore.h"

class CLibraryDb: public CLibraryStore
{
protected:
	// Here is our connection to the library
	CDaoDatabase m_database;

	// Delete a symbol from the database
	void DeleteSymbol(int SymbolID);

public:
	CLibraryDb();
	virtual ~CLibraryDb() OVERRIDE;

	// Attach this library to a file
	virtual BOOL Attach(const TCHAR *filename) OVERRIDE;

	// Write a symbol to this library
	virtual void Store(CLibraryStoreNameSet *nwSymbol, CTinyCadMultiSymbolDoc &document) OVERRIDE;

	// Delete a symbol from this library
	virtual void DeleteSymbol(CLibraryStoreNameSet &symbol) OVERRIDE;

	// Do any idle time tasks...
	virtual void OnIdle() OVERRIDE;

	// Get the Archive to load from
	virtual CStream *GetMethodArchive(CLibraryStoreNameSet *symbol) OVERRIDE;

	// Create a new library database
	virtual bool Create(const TCHAR *filename) OVERRIDE;

	// Is an upgrade required before editing this library?
	virtual BOOL MustUpgrade() OVERRIDE;

	// Upgrade to the latest version of the library system
	virtual BOOL Upgrade(CLibraryStore *) OVERRIDE;
};

#endif // !defined(AFX_LIBRARYDB_H__0CCD1C42_BEDE_4447_95C4_0C8B663415CB__INCLUDED_)
