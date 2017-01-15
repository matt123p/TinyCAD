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

#ifndef __LIBRARY_H__
#define __LIBRARY_H__

#include "Symbol.h"
#include "LibraryStore.h"

////// The Library class which encapsulates each library in use //////
class CLibraryFile: public CLibraryStore
{
	CFile theMethodsFile; // The Method's file
	BOOL methodsFileOpen; // Is the method's file & archive open?
	BYTE revision; // The revision of the methods file to use
	CString MethodsFileName(); // Return the current methods file name

	// Write the method into either it's own file or the index
	// file.
	void WriteMethod(CStreamFile &ar, CTinyCadDoc &design);

public:
	CLibraryFile(); // The constructor
	virtual ~CLibraryFile(); // The destructor

	virtual BOOL Attach(const TCHAR *filename) OVERRIDE; // Attach this library to a file

	// Write a symbol to this library
	virtual void Store(CLibraryStoreNameSet *nwSymbol, CTinyCadMultiSymbolDoc &document) OVERRIDE;

	// Delete a symbol from this library
	virtual void DeleteSymbol(CLibraryStoreNameSet &symbol) OVERRIDE;

	virtual void OnIdle() OVERRIDE
	{
		CloseMethodFile();
	}

	virtual CStream *GetMethodArchive(CLibraryStoreNameSet *symbol) OVERRIDE; // Get the methods file for this library
	void CloseMethodFile();
	BOOL OpenIndexFile(CFile &, int); // Get the symbols file for this library


	// Is an upgrade required before editing this library?
	virtual BOOL MustUpgrade() OVERRIDE;

	// Upgrade to the latest version of the library system
	virtual BOOL Upgrade(CLibraryStore *) OVERRIDE;

	// Create a new library database
	virtual bool Create(const TCHAR *filename) OVERRIDE;
};

// The dialog for renaming a symbol
class CDlgSymbolRename: public CDialog
{
	CString *name;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnOK();
	afx_msg void OnCancel();
public:
	CDlgSymbolRename(CString *NewName, CWnd *pParentWnd = NULL) :
		CDialog(IDD_SYMBOLRENAME, pParentWnd)
	{
		name = NewName;
	}
};

#endif
