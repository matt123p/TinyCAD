/*
 TinyCAD program for schematic capture
 Copyright 1994-2011 Matt Pyne.

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
	//			CCommandLineInfo::FileNothing   Turns off the display of a new MDI child window on startup. By design, Application Wizard-generated MDI applications display a new child window on startup. To turn off this feature, an application can use CCommandLineInfo::FileNothing as the shell command when calling ProcessShellCommand. ProcessShellCommand is called by the InitInstance( ) of all CWinAppEx derived classes.

	//for convenience maintain a variable for each custom command line option to indicate the param passed. 
	BOOL m_bGenerateSpiceFile;     //for /s	This allows the user to generate Spice netlist files from a script processor
	BOOL m_bGenerateXMLNetlistFile;	//for /x	This allows the user to generate XML netlist files from a script processor
	BOOL m_bOutputDirectory;	//for --output_dir  This allows the user to specify an output directory separately from the input directory or the working directory
	BOOL m_bConsoleIORequired;
	enum TCFlag 
	{
		TCFlag_Unknown=0,
		TCFlag_OutputDirectory=1,
		TCFlag_GenerateSpiceFile=2,
		TCFlag_GenerateXMLNetListFile=3
	};
	TCFlag m_eLastFlag;
	CString m_OutputDirectory;

	//public methods for checking these.
public:
	CTinyCadCommandLineInfo();	//Constructor
	BOOL IsShellOpen();
	BOOL IsGenerateSpiceFile();
	BOOL IsGenerateXMLNetlistFile();
	BOOL IsConsoleApp();
	BOOL m_bConsoleAcquired;
	CString getOutputDirectory();
	virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);

	DWORD RedirectIOToConsole();	//This function is used to associate a console window with this process for command options that require a console

	~CTinyCadCommandLineInfo();	//Destructor
};


//*************************************************************************
//*                                                                       *
//*                  Main application class                               *
//*                                                                       *
//*************************************************************************
class CTinyCadApp: public CWinAppEx
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
	static bool GetWindowsVersionName(wchar_t* str, int bufferSize);
	static CString GetVersion();
	static CString GetReleaseType();
	static CString GetName();
	static CString GetMainDir();	//Gets the installed path location of the directory that contains the currently executing copy of TinyCAD
	static CString GetDefaultLibraryDir();	//Gets the default library directory path
	static CString GetLongFileName(CString shortFilename);	//Gets the long (i.e., non-DOS 8.3) filename

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
	void OnMyFileOpen();

	//{{AFX_MSG(CTinyCadApp)
	afx_msg void OnAppAbout();
	afx_msg void OnLibLib();
	afx_msg void OnHelpOpenTinyCADUserManual();
	afx_msg void OnHelpGototinycadwebsite();
	afx_msg void OnHelpHelp();
	afx_msg void OnHelpEmailforsupport();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	virtual BOOL LoadWindowPlacement(CRect& rectNormalPosition, int& nFflags, int& nShowCmd);
};
//=========================================================================
extern int Message(int Resource, int Type = MB_ICONINFORMATION | MB_OK, const TCHAR *NameString = _T(""));
extern CString NameLength(const TCHAR *, int);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
