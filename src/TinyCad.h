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
//
//This is a minor command line parser support class
//It allows us to define custom command line arguments that can be passed in when TinyCAD is started
//Possible extensions include 
//	1.  Generate a SPICE netlist
//	2.  Generate an XML netlist
//	3.  Anything else that you think that you might want to access from a build script or automation script
//
	
class CTinyCadCommandLineInfo : public CCommandLineInfo
{
	//Documentation on the contents of Window's Shell command line standard definitions 
	//can be found here:  http://msdn.microsoft.com/en-us/library/zaydx040(VS.80).aspx
	//			enum {
	//			   FileNew,
	//			   FileOpen,
	//			   FilePrint,
	//			   FilePrintTo,
	//			   FileDDE,
	//			   FileNothing = -1
	//			} m_nShellCommand;
	//			For a brief description of these values, see the following list. 
	//			CCommandLineInfo::FileNew   Indicates that no filename was found on the command line.
	//			CCommandLineInfo::FileOpen   Indicates that a filename was found on the command line and that none of the following flags were found on the command line: /p, /pt, /dde.
	//			CCommandLineInfo::FilePrint   Indicates that the /p flag was found on the command line.
	//			CCommandLineInfo::FilePrintTo   Indicates that the /pt flag was found on the command line.
	//			CCommandLineInfo::FileDDE   Indicates that the /dde flag was found on the command line.
	//			CCommandLineInfo::FileNothing   Turns off the display of a new MDI child window on startup. By design, Application Wizard-generated MDI applications display a new child window on startup. To turn off this feature, an application can use CCommandLineInfo::FileNothing as the shell command when calling ProcessShellCommand. ProcessShellCommand is called by the InitInstance( ) of all CWinApp derived classes.

	//for convenience maintain 4 variables to indicate the param passed. 
	BOOL m_bGenerateSpiceFile;     //for /s	This allows the user to generate Spice netlist files from a script processor
	BOOL m_bGenerateXMLNetlistFile;	//for /x	This allows the user to generate XML netlist files from a script processor

	//public methods for checking these.
public:
	CTinyCadCommandLineInfo()
	{
		m_bGenerateSpiceFile = m_bGenerateXMLNetlistFile = FALSE;
	}

	BOOL IsShellOpen() 
	{
		return this->m_strFileName.GetLength() != 0;	//there is no flag for passing a file name on the command line with no other conflicting options.
	}

	BOOL IsGenerateSpiceFile()
	{
		return m_bGenerateSpiceFile;
	}

	BOOL IsGenerateXMLNetlistFile()
	{
		return m_bGenerateXMLNetlistFile;
	}
	 
	virtual void ParseParam(const char* pszParam, BOOL bFlag, BOOL bLast)
	{
		if (0 == strcmp(pszParam, "/s"))
		{
			m_bGenerateSpiceFile = TRUE;
			TRACE("CTinyCadCommandLineInfo::ParseParam():  Found command line option /s (hijacked this one for generating Spice files)\n");
		}
		else if (0 == strcmp(pszParam, "/x"))
		{
			m_bGenerateXMLNetlistFile = TRUE;
			TRACE("CTinyCadCommandLineInfo::ParseParam():  Found command line option /x (hijacked this one for generating XML netlist files)\n");
		}
		else
		{
			TRACE("CTinyCadCommandLineInfo::ParseParam():  No special command line options found in command line parameter string=\"%s\"\n", pszParam);
		}
	}
};


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
	static COLORREF m_colours[16];	//These are the 16 different custom colors defined by the user.  TinyCAD presently doesn't save and restore these from the registry and needs to! - djl 06/15/2011

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
	static CString GetLongFileName(CString filename);	//Gets the long (i.e., non-DOS 8.3) filename

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
