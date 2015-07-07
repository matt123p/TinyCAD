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

#ifndef __LIBRARYSTORE_H__
#define __LIBRARYSTORE_H__

#include "Symbol.h"

class CTinyCadMultiSymbolDoc;

class CLibraryStore
{
public:
	typedef std::map<int, CLibraryStoreNameSet> symbolCollection;
	symbolCollection m_Symbols; // The collection of symbols, indexed by symbol id

	CString m_name; ///< The file name of this library

	CLibraryStore(); // The constructor
	virtual ~CLibraryStore(); // The destructor


	// Get a symbol from this library
	CLibraryStoreNameSet *Extract(const TCHAR *);

	// Adds symbols matching given string to a CListBox
	virtual void AddToListBox(const TCHAR *, CListBox *);

	void AddToTreeCtrl(const TCHAR *theString, CTreeCtrl* Tree, HTREEITEM hLib);
	int GetMatchCount(const TCHAR *theString);

	// Check to see if the item exists in this library
	virtual BOOL DoesExist(CLibraryStoreNameSet *pSymbol);

	// Re-read this library
	virtual void ReRead();

	// Write this library to an XML file
	virtual void SaveXML(const TCHAR *filename, int id = -1);

	// Import library symbols from an XML file
	virtual void LoadXML(const TCHAR *filename);

	// Get the Archive to load from
	virtual CStream *GetMethodArchive(CLibraryStoreNameSet *symbol) = 0;

	// Attach this library to a file
	virtual void Attach(const TCHAR *filename) = 0;

	// Is an upgrade required before editing this library?
	virtual BOOL MustUpgrade() = 0;

	// Upgrade to the latest version of the library system
	virtual BOOL Upgrade(CLibraryStore *) = 0;

	// Delete a symbol from this library
	virtual void DeleteSymbol(CLibraryStoreNameSet &symbol) = 0;

	// Write a symbol to this library
	virtual void Store(CLibraryStoreNameSet *nwSymbol, CTinyCadMultiSymbolDoc &document) = 0;

	// Do any idle time tasks...
	virtual void OnIdle() = 0;

	// Create a new library database
	virtual bool Create(const TCHAR *filename) = 0;
};

typedef std::list<CLibraryStore*> libraryCollection;
typedef libraryCollection::iterator libraryIterator;

#endif // __LIBRARYSTORE_H__
