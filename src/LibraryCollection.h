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

#include "LibraryStore.h"

//*************************************************************************
//*                                                                       *
//*  Stores all libraries as pointers to CLibraryStore objects            *
//*  in one singleton.                                                    *
//*                                                                       *
//*  The pointer handling is a asynchron. They were created outside this  *
//*  class and passed to the Add method, the but removed in the           *
//*  Delete and Clear methods.                                            *
//*                                                                       *
//*************************************************************************
class CLibraryCollection
{
private:
	//=====================================================================
	//== class variables                                                 ==
	//=====================================================================
	//-- singleton instance of this class
	static CLibraryCollection m_oInstance;

	//-- The linked list of libraries
	static std::list<CLibraryStore*> m_colLibs;

	//=====================================================================
	//== ctor/dtor/initializing                                          ==
	//=====================================================================
	//-- This private default ctor is only used creating the singleton.
	//-- Purpose: Hiding the public default ctor
	CLibraryCollection(){}

public:
	//=====================================================================
	//== mutator                                                         ==
	//=====================================================================
	//-- Removes  all library objects (and deletes all pointers)
	static void Clear();

	//-- Add one library
	static void Add(CLibraryStore* pLib);

	//-- Removes one library(and deletes its pointer)
	static void Remove(CLibraryStore* pLib);

	//-- Doing some idle tasks thru calling all the idle time
	//-- processing of the libraries
	static void DoIdle();

	//=====================================================================
	//== accessor                                                        ==
	//=====================================================================
	//-- Returns true if this library exist in the collection
	static bool Contains(CLibraryStore* pLib);

	//-- Returns true if this symbol exist in any library
	static bool ContainsSymbol(CLibraryStoreNameSet* pSymbol);

	//-- Returns the library with this name
	static CLibraryStore* GetLibrary(CString sName);

	//-- Returns a symbol identified by its name, searching in all libraries
	static CLibraryStoreNameSet* GetSymbol(CString sKey);

	//-- Fills listbox entries with library names
	static void FillLibraryNames(CListBox* lstLib);

	//-- Fills a menu entries with library names
	static void FillLibraryNames(int start_id, CMenu* menuLib);

	//-- Find a library given it's index in the array
	static CLibraryStore* GetLibraryByIndex(int library_id);

	//-- Fills listbox with all symbols(of all libraries), whoose symbol name
	//-- matches the search string
	static void FillMatchingSymbols(CListBox* lstSymbols, CString sSearch, CListBox* mask);
	static void FillMatchingSymbols(CTreeCtrl* Tree, const CString& sSearch);

	//-- Write the all library file pathes back to the registry
	static void SaveToRegistry();

	//-------------------------------------------------------------------------
	//-- Clanup a library (runs VACUUM)
	static void Cleanup(CLibraryStore* pLib);

	//-------------------------------------------------------------------------
	//-- Converts a library from the old format to
	//-- the new Microsoft Access database format.
	static CLibraryStore *Upgrade(CLibraryStore* pOldLib);
};
