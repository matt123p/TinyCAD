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

#pragma once

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"
#include "LibraryStore.h"

//*************************************************************************
//*                                                                       *
//*                  Main application class                               *
//*                                                                       *
//*************************************************************************
class CTinyCadApp: public CWinApp
{
	//=====================================================================
	//== class variables                                                 ==
	//=====================================================================
	//-- singleton instance of this class
private:
	static CTinyCadApp m_oInstance;

	// The document template for all TinyCadDoc types
	static CMultiDocTemplate* m_pDocTemplate;

	// The document template for library viewing/editing
	static CMultiDocTemplate* m_pLibTemplate;

	// The document template for editing text files
	static CMultiDocTemplate* m_pTxtTemplate;

	static bool m_LockOutSymbolRedraw;

	// The list of colours used in the colour selection dialogue
	static COLORREF m_colours[16];

	// Handle to accelerator table
	static HACCEL m_hAccelTable;

	static bool m_translateAccelerator;

	//=====================================================================
	//== ctor/dtor/initializing                                          ==
	//=====================================================================
	virtual BOOL InitInstance();

public:
	CTinyCadApp();
	virtual ~CTinyCadApp();

	//=====================================================================
	//== accessor                                                        ==
	//=====================================================================
	static bool IsWinNT();
	static CString GetVersion();
	static CString GetReleaseType();
	static CString GetName();
	static CString GetMainDir();

	//=====================================================================
	//== mutator                                                         ==
	//=====================================================================
	//-- Read the settings from the registry
private:
	void ReadRegistry();
public:
	static void SetLockOutSymbolRedraw(bool r);
	static void SetTranslateAccelerator(bool b);

	//=====================================================================
	//==  manage views                                                   ==
	//=====================================================================
public:
	static void EditSymbol(CLibraryStore* pLib, CLibraryStoreNameSet& symbol);
	static void EditLibrary(CLibraryStore* pLib);
	static void ResetAllSymbols();
	static void SaveAll();
	static bool IsLibInUse(CLibraryStore* pLib);
	static void EditTextFile(const TCHAR *filename);
	static void EditDesign(const TCHAR *filename);
	static BOOL ChooseColor(COLORREF &col);

	//=====================================================================
	//== Message handler                                                 ==
	//=====================================================================
	virtual BOOL OnIdle(LONG lCount);
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);

	//{{AFX_MSG(CTinyCadApp)
	afx_msg void OnAppAbout();
	afx_msg void OnLibLib();
	afx_msg void OnHelpOpenTinyCADUserManual();
	afx_msg void OnHelpGototinycadwebsite();
	afx_msg void OnHelpHelp();
	afx_msg void OnHelpEmailforsupport();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
//=========================================================================
extern int Message(int Resource, int Type = MB_ICONINFORMATION | MB_OK, const TCHAR *NameString = _T(""));
extern CString NameLength(const TCHAR *, int);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
