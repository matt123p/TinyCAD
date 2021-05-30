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

#include "stdafx.h"
#include "TinyCad.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "TinyCadView.h"
#include "TinyCadRegistry.h"
#include "LibraryView.h"
#include "LibraryDoc.h"
#include "TinyCadMultiSymbolDoc.h"
#include "TinyCadMultiDoc.h"
#include "LibraryDb.h"
#include "LibrarySQLite.h"
#include "LibraryCollection.h"
#include "DlgLibraryBox.h"
#include "DlgAbout.h"
#include "AutoSave.h"
#include "TextEditDoc.h"
#include "TextEditView.h"
#include "BuildID.h"
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include "UpdateCheck.h"
#include "DlgUpdateCheck.h"


// NOTE: This is never compiled in.  It is used to 
// make VS.NET recognise that this is an MFC project.
#ifdef _DUMMY_
static CWinAppEx theApp;
#endif

CTinyCadRegistry * g_pRegistry = NULL;

CTinyCadCommandLineInfo::CTinyCadCommandLineInfo()
{ //Constructor
	m_bGenerateSpiceFile = FALSE;
	m_bGenerateXMLNetlistFile = FALSE;
	m_eLastFlag=TCFlag_Unknown;
	m_OutputDirectory = _T("");
	m_bConsoleIORequired = FALSE;
	m_bConsoleAcquired = FALSE;
}

CTinyCadCommandLineInfo::~CTinyCadCommandLineInfo()
{
}

BOOL CTinyCadCommandLineInfo::IsShellOpen()
{
	return m_nShellCommand == FileOpen; //This flag is parsed by the base class and is set to FileOpen solely if there is a file name on the command line and no other conflicting options.
}

BOOL CTinyCadCommandLineInfo::IsGenerateSpiceFile()
{
	return m_bGenerateSpiceFile;
}

BOOL CTinyCadCommandLineInfo::IsGenerateXMLNetlistFile()
{
	return m_bGenerateXMLNetlistFile;
}

BOOL CTinyCadCommandLineInfo::IsConsoleApp()
{
	return m_bConsoleIORequired;
}

CString CTinyCadCommandLineInfo::getOutputDirectory()
{
	return m_OutputDirectory;
}

// RedirectIOToConsole() is adapted from the information provided at http://dslweb.nwnexus.com/~ast/dload/guicon.htm
DWORD CTinyCadCommandLineInfo::RedirectIOToConsole()
{
	//This function is used to associate a console window with this process for command options that require a console
	int hConHandle=0;
	size_t lStdHandle=0;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp=NULL;
	DWORD retCode=0;

	retCode = AttachConsole(ATTACH_PARENT_PROCESS);	//A Windows GUI app has already detached from the console, so it is necessary to reattach
	if (retCode == 0)
	{
		DWORD errorCode = GetLastError();
		//Note:  Error code==6 will be returned when running under the debugger because a parent console process already exists and you are not allowed to attach to it.  The code indicates an invalid handle was used.
		ATLTRACE2(_T("CTinyCadCommandLineInfo::RedirectIOToConsole():  AttachConsole failed and returned code=%d.  The GetLastError() function returned %u\n"), retCode, errorCode);
	}
	else
	{
		fprintf(stdout,"Console is now re-attached\n");
	}

	if (retCode == 0)	//Reattach to existing console was not successful, for whatever reason
	{
		ATLTRACE2("Unable to attach to an existing console (error code = %d) - allocating a new one instead\n", retCode);
		retCode = AllocConsole();
		if (retCode == 0)
		{
			DWORD errorCode = GetLastError();
			//errorCode == 5 is caused by trying to create a console while one already exists.
			ATLTRACE2("Unable to allocate a new console (error code = %d, GetLastError() reports error = %u.\nThis can supposedly only be caused if the process already has a console as it is only allowed to have one.\n", retCode, errorCode);
		}
		else
		{
			// set the screen buffer to be big enough to let us scroll text, but don't override the user's choice if it is already large enough
			GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
			short maxConsoleLines = 200;
			coninfo.dwSize.Y = maxConsoleLines;
			SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
		}
	}

	//A non-zero return code at this point means that a valid console has either been re-attached to, or created.
	if (retCode != 0)
	{
		(void) freopen("CONIN$", "r", stdin);
		(void) freopen("CONOUT$", "w", stdout);
		(void) freopen("CONOUT$", "w", stderr);

		// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog 
		// point to console as well
		std::ios::sync_with_stdio();
	}
	return retCode;
}

void CTinyCadCommandLineInfo::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
{
	if (bFlag)
	{ //This is a command line option (Windows calls them "flags")
		const CString strParam(pszParam);
		CString optionName, optionValue;

		//See if this option is of the form <option>=<string>
		int index = strParam.Find(_T('='));
		if (index >= 0)
		{
			optionName = strParam.Left(index);
			optionValue = strParam.Right(strParam.GetLength() - index - 1);
		}
		else
		{
			optionName = strParam;
			optionValue = _T("");
		}

		//Note that options have been stripped by Windows of the leading '/' or '-' that signifies that this is an option
		if ((optionName.CompareNoCase(_T("s")) == 0) || (optionName.CompareNoCase(_T("-gen_spice_netlist")) == 0))
		{
			m_bGenerateSpiceFile = TRUE;
			m_eLastFlag = TCFlag_GenerateSpiceFile;
			m_bConsoleIORequired = TRUE;
			ATLTRACE2("CTinyCadCommandLineInfo::ParseParam():  Found command line option /s or --gen_spice_netlist\n");
		}
		else if ((optionName.CompareNoCase(_T("x")) == 0) || (optionName.CompareNoCase(_T("-gen_xml_netlist")) == 0))
		{
			m_bGenerateXMLNetlistFile = TRUE;
			m_eLastFlag = TCFlag_GenerateXMLNetListFile;
			m_bConsoleIORequired = TRUE;
			ATLTRACE2("CTinyCadCommandLineInfo::ParseParam():  Found command line option /x or --gen_xml_netlist\n");
		}
		else if (optionName.CompareNoCase(_T("-out_directory")) == 0)
		{
			m_bOutputDirectory = TRUE;
			m_OutputDirectory = optionValue;
			m_eLastFlag = TCFlag_OutputDirectory;
			m_bConsoleIORequired = TRUE;
			ATLTRACE2("CTinyCadCommandLineInfo::ParseParam():  Found command line option --output_dir=\"%S\"\n", pszParam);
		}
		else if ((optionName.CompareNoCase(_T("-help")) == 0) || (optionName.CompareNoCase(_T("h")) == 0) || (optionName.CompareNoCase(_T("?")) == 0))
		{
			m_bConsoleIORequired = TRUE;
			if (!m_bConsoleAcquired) {
				m_bConsoleAcquired = RedirectIOToConsole();
			}
			fwprintf(stderr, _T("\nTinyCAD Version %s copyright (c) 1994-2019 Matt Pyne.  Licensed under GNU LGPL 2.1 or newer\n"), (LPCTSTR) (CTinyCadApp::GetVersion()));
			fwprintf(stderr, _T("Correct usage is:\n"));
			fwprintf(stderr,_T("tinycad <design file name with optional path and mandatory file type extension (.dsn for design files)> [options]\n"));
			fwprintf(stderr,_T("Optional command line options:\n"));
			fwprintf(stderr,_T("\t/s                         Generate Spice netlist file with same base name as the design file\n"));
			fwprintf(stderr,_T("\t--gen_spice_netlist        Generate Spice netlist file with same base name as the design file\n"));
			fwprintf(stderr,_T("\t/x                         Generate XML netlist file with same base name as the design file\n"));
			fwprintf(stderr,_T("\t--gen_xml_netlist          Generate XML netlist file with same base name as the design file\n"));
			fwprintf(stderr,_T("\t--out_directory=<filepath> Specify a full or partial file path to use for generated netlist file(s)\n\t                           A trailing '\\' is optional\n"));
			//fwprintf(stderr,_T("\t/dde                       Used by Windows for Dynamic Data Exchange.  This is a Windows service\n"));
			//fwprintf(stderr,_T("\t/p                         Print the design file on the default printing device.  This is a Windows service\n"));
			//fwprintf(stderr,_T("\t/pt <printer name>         Print the design file to the specified printer name.  This is a Windows service\n"));
			fwprintf(stderr,_T("\t/h                         Display this help information\n"));
		}
		else
		{
			ATLTRACE2("CTinyCadCommandLineInfo::ParseParam():  Found non-TinyCAD command line option \"%S\"\n", pszParam);
			m_eLastFlag = TCFlag_Unknown;
			CStringA temp(pszParam);	//Convert TCHAR string to char string
			ParseParamFlag(temp.GetString()); //Not one of TinyCad's - let the regular CCommandLineInfo object parse it.  This function expects a char string, not a TCHAR string.
		}
	}
	else
	{ 
		//This is a command line parameter, not an option flag.  However, it may "belong" to the last option flag parsed, or it may be a standalone parameter
		//For example, an optional output directory may be specified as "--out_directory=somewhere", or as "--out_directory somewhere"
		ATLTRACE2("CTinyCadCommandLineInfo::ParseParam():  Found command line parameter=\"%S\".\n", pszParam);
		switch(m_eLastFlag)
		{
			case TCFlag_OutputDirectory:
				if (m_OutputDirectory.IsEmpty()) m_OutputDirectory = pszParam;
				else 
				{
					ATLTRACE2("CTinyCadCommandLineInfo::ParseParam():  Found unexpected command line parameter \"%S\" following option --out_directory\n", pszParam);
				}
				break;
			case TCFlag_GenerateSpiceFile:
			case TCFlag_GenerateXMLNetListFile:
				ATLTRACE2("CTinyCadCommandLineInfo::ParseParam():  Found unexpected command line parameter \"%S\".  Discarding parameter and continuing.\n", pszParam);
				break;
			case TCFlag_Unknown:
			default:
				ParseParamNotFlag(pszParam); //Not one of TinyCad's - let the regular Windows CCommandLineInfo object parse it.
				break;
		}
	}

	//If bLast is true, then there are no more parameters on the command line.  A missing, but required parameter could be detected here, if needed.
	if (bLast && m_bConsoleIORequired && !m_bConsoleAcquired)
	{
		m_bConsoleAcquired = RedirectIOToConsole();
	}
	ParseLast(bLast);
}

//*************************************************************************
//*                                                                       *
//*                  Main application class                               *
//*                                                                       *
//*************************************************************************

CTinyCadApp::CTinyCadApp()
{
	m_UpdateAvailable = FALSE;
}

CTinyCadApp::~CTinyCadApp()
{
	CLibraryCollection::Clear();
	delete g_pRegistry;
}

BEGIN_MESSAGE_MAP(CTinyCadApp, CWinAppEx)
//{{AFX_MSG_MAP(CTinyCadApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_HELP_CHECKFORUPDATES, OnAppUpdateCheck)
	ON_COMMAND(ID_AUTOUPDATE, OnAppUpdateChecked)
	ON_COMMAND(IDM_LIBLIB, OnLibLib)
	ON_COMMAND(ID_HELP_OPENTINYCADUSERMANUAL, OnHelpOpenTinyCADUserManual)
	ON_COMMAND(ID_HELP_GOTOTINYCADWEBSITE, OnHelpGototinycadwebsite)
	ON_COMMAND(ID_HELP_HELP, OnHelpHelp)
	ON_COMMAND(ID_HELP_SUPPORTFORUM, OnHelpSupport)

	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinAppEx::OnFileNew)
//	ON_COMMAND(ID_FILE_OPEN, CWinAppEx::OnFileOpen)	//The standard OnFileOpen accesses buggy Microsoft MFC code that manifests only in Windows 8.1.  I replaced it with CTinyCadApp::OnMyFileOpen().  See http://yourprosoft.blogspot.com/2012/01/mfc-encountered-improper-argument.html
	ON_COMMAND(ID_FILE_OPEN, CTinyCadApp::OnMyFileOpen)

	ON_UPDATE_COMMAND_UI(ID_INDICATOR_UPDATE, &CTinyCadApp::OnUpdateUpdateIndicator)

	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinAppEx::OnFilePrintSetup)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=========================================================================
//== init class variables                                                ==
//=========================================================================
// base class ctor creates the one and only application object
CTinyCadApp CTinyCadApp::m_oInstance;
CMultiDocTemplate* CTinyCadApp::m_pDocTemplate = NULL;
CMultiDocTemplate* CTinyCadApp::m_pLibTemplate = NULL;
CMultiDocTemplate* CTinyCadApp::m_pTxtTemplate = NULL;
bool CTinyCadApp::m_LockOutSymbolRedraw = false;
COLORREF CTinyCadApp::m_colours[16];

bool m_hiddenWindow = false;

#pragma comment(linker, "\"/manifestdependency:type='win32'\
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//=========================================================================
//== ctor/dtor/initializing                                              ==
//=========================================================================
BOOL CTinyCadApp::InitInstance()
{

	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//djl - this next line doesn't work, presumably because this is not the console!
	_cwprintf(_T("CTinyCadApp::InitInstance():  This is displayed by _cwprintf() prior to InitCommonControls()\r\n"));
	

	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	//InitCommonControls();
	INITCOMMONCONTROLSEX CommonControls;
	CommonControls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	// I could not see any effect of the specific value here
	CommonControls.dwICC = ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&CommonControls);

	// Now run the standard CWinAppEx initInstance() function
	CWinAppEx::InitInstance();


	InitContextMenuManager();
	InitShellManager();
	InitKeyboardManager();
	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	GetTooltipManager()->
		SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	//djl - this next line doesn't work, presumably because this is not the console!
	_cwprintf(_T("CTinyCadApp::InitInstance():  This is displayed by _cwprintf() after call to CWinAppEx::InitInstance()\r\n"));

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Change the registry key under which our settings are stored.
	SetRegistryKey(CTinyCadApp::GetName());

	// Read the registry
	ReadRegistry();

	// Load standard INI file options (including MRU)
	LoadStdProfileSettings(6);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.
	m_pDocTemplate = new CMultiDocTemplate(IDR_TCADTYPE, RUNTIME_CLASS(CTinyCadMultiDoc), RUNTIME_CLASS(CChildFrame), // custom MDI child frame
			RUNTIME_CLASS(CTinyCadView));
	AddDocTemplate(m_pDocTemplate);

	m_pLibTemplate = new CMultiDocTemplate(IDR_LIBTYPE, RUNTIME_CLASS(CLibraryDoc), RUNTIME_CLASS(CChildFrame), // custom MDI child frame
			RUNTIME_CLASS(CLibraryView));
	AddDocTemplate(m_pLibTemplate);

	m_pTxtTemplate = new CMultiDocTemplate(IDR_TXTTYPE, RUNTIME_CLASS(CTextEditDoc), RUNTIME_CLASS(CChildFrame), // custom MDI child frame
			RUNTIME_CLASS(CTextEditView));
	AddDocTemplate(m_pTxtTemplate);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open, but don't process the commands until later in this function after the windows have been created and opened in hidden mode
	CTinyCadCommandLineInfo cmdInfo; //This is the TinyCAD overridden command line parser class
	ParseCommandLine(cmdInfo); //This parses all of the options on the command line

	CPane::m_bHandleMinSize = true;

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	pMainFrame->runAsConsoleApp = cmdInfo.IsConsoleApp(); //Check for all commands that need to run as a console app here
	pMainFrame->consoleAppRetCode = -1; //default value indicates failure - will be set to 0 or an exit code by any console app functions that are run.  Not used in GUI mode.

	if (pMainFrame->runAsConsoleApp)
	{ //This is a TinyCAD specific custom command line argument - hide all windows
		m_nCmdShow = SW_HIDE; //This flag will be explicitly checked by the NOTOOL window to see whether it should be shown or hidden
		m_hiddenWindow = true;
	}
	else if (CTinyCadRegistry::GetMaximize() && m_nCmdShow == 1)
	{
		m_nCmdShow = SW_SHOWMAXIMIZED;
	}

	if (!pMainFrame->LoadFrame(IDR_MAINFRAME)) return FALSE;

	m_pMainWnd = pMainFrame;

	//Set up the proper help file mode and strings
	free((void*) m_pszHelpFilePath);	//Free the string allocated by MFC at CWinAppEx startup to avoid a memory leak.  The string is allocated before InitInstance is called.
	//Change the name of the .HLP file.
	//The CWinAppEx destructor will free the memory.
	m_pszHelpFilePath = _tcsdup(GetMainDir() + _T("TinyCAD.html"));	//Create the new help file path name
	SetHelpMode(afxHTMLHelp);

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	if (cmdInfo.IsShellOpen())
	{
		//Depending on Windows registry settings, Explorer or a command shell may choose to pass in an old fashioned DOS 8.3 filename.
		//Lookup the long version of this filename in the current working directory and then open the long version of the filename.
		ATLTRACE2("CTinyCad::InitInstance() received a file open command from the Windows Shell processor.  Filename=\"%S\"\n", cmdInfo.m_strFileName);
		CString longName = GetLongFileName(cmdInfo.m_strFileName); //Convert potential DOS 8.3 short file name into a long file name
		cmdInfo.m_strFileName = longName; //Replace the short filename with the long filename
		ATLTRACE2("CTinyCad::InitInstance():                                                          long file name=\"%S\"\n", longName);
	}

	// Now dispatch all TinyCAD custom commands specified on the command line, including the DDE commands such as FileOpen, FilePrint, etc.
	BOOL successful = ProcessShellCommand(cmdInfo);	//This executes all standard shell commands and ignores any custom flags
	ATLTRACE2("CTinyCad::InitInstance() received %s Shell command (numeric command = %d).  Filename=\"%S\"\n", successful ? "successful" : "unsuccessful", (int) cmdInfo.m_nShellCommand, cmdInfo.m_strFileName);

	if (!successful)
	{
		if (cmdInfo.IsGenerateSpiceFile() || cmdInfo.IsGenerateXMLNetlistFile())
		{
			if ((cmdInfo.m_nShellCommand == cmdInfo.FileNew) || (cmdInfo.m_nShellCommand == cmdInfo.FileOpen))
			{
				TCHAR workingDirectory[1024];
				workingDirectory[0] = _T('\0');
				GetCurrentDirectory(1024, workingDirectory);	//This may cause an error on systems older than WinXP

				fprintf(stderr,"Error #1:  TinyCAD cannot find or navigate to the file named \"%s\" starting from location \"%s\"\n", (char *) cmdInfo.m_strFileName.GetBuffer(), (char *)workingDirectory);

				ATLTRACE2(_T("TinyCAD cannot find or navigate to the file named \"%s\" starting from location \"%s\"\n"), cmdInfo.m_strFileName, (char *) workingDirectory);
			}
		}
		return FALSE;
	}

	CTinyCadMultiDoc *pDesign = NULL; //This will be used to hold the FileOpen document, if running as a console app.  Not used otherwise.
	int retCode = pMainFrame->consoleAppRetCode; //Save the return code so it can be used for console mode after the mainframe document is destroyed.

	if (cmdInfo.IsConsoleApp())
	{
		if (cmdInfo.IsGenerateSpiceFile() || cmdInfo.IsGenerateXMLNetlistFile())
		{ //This is a TinyCAD specific custom command line argument
			if (cmdInfo.IsGenerateSpiceFile())
			{ //Run spice netlister here in hidden mode since this has been invoked from a command prompt
				TRACE("CTinyCad::InitInstance() received TinyCad command argument to run the Spice netlister.\n");

				// The main window has been initialized, so show and update it in hidden display mode.
				pMainFrame->ShowWindow(m_nCmdShow);
				pMainFrame->UpdateWindow();

				//Retrieve a pointer to the newly opened CTinyCadMultiDoc (i.e., the dsn file that the command prompt just opened)
				POSITION localPosition = m_pDocTemplate->GetFirstDocPosition(); //The open design is the only design file in the template collection at this point
				pDesign = static_cast<CTinyCadMultiDoc *> (m_pDocTemplate->GetNextDoc(localPosition));

				static_cast<CTinyCadView *> (pMainFrame->GetActiveView())->CommandPromptCreatespicefile(pDesign, cmdInfo.m_strFileName, cmdInfo.getOutputDirectory()); //create the spice file
				retCode = pMainFrame->consoleAppRetCode; //Save the return code so it can be used for console mode after the mainframe document is destroyed.
			}
			else
			{ //Run XML netlister here!
				TRACE("CTinyCad::InitInstance() received TinyCad command argument to run the XML netlister.\n");
				//retCode = pMainFrame->consoleAppRetCode; //Save the return code so it can be used for console mode after the mainframe document is destroyed.
			}
			//Now take an early exit
			ATLTRACE2("CTinyCad::InitInstance():  Console mode operation is completed.  Sending Quit message\n");

			//Close opened document(s) and anything else created here in InitInstance that needs closing
			//Note:  If due to some unanticipated error in the netlist, a dialog box does manage to pop up, not closing it here will cause memory leaks.
			//The only way to handle this properly is to either write a stand-alone command line program, or anticipate all of the possible errors, or simply accept that a memory leak will occur!
			if (pDesign)
				pDesign->OnCloseDocument();

		}
		//Send the WinApp::run() loop a message to quit and also transfer the desired error code back to Windows.  A batch file running a console mode TinyCAD command will receive this code.
		AfxPostQuitMessage(retCode);
	}
	else
	{ // The main window has been initialized, so show and update it.
		pMainFrame->ShowWindow(m_nCmdShow);
		pMainFrame->UpdateWindow();

		//Turn on the auto-save functionality
		CAutoSave::Start(); 

		// Check for updates, if enabled
		switch (CTinyCadRegistry::GetAutomaticUpdatesOn())
		{
		case 0:
			// Auto updates are off
			break;
		case 1:
			// Auto updates are on
			// Do we really need to check?
			{
				CString previousUpdateVersion = CTinyCadRegistry::GetLastAutomaticUpdateVersion();
				CString currentVersion = GetVersion();

				if (m_UpdateCheck.VersionAsNumber(previousUpdateVersion) > m_UpdateCheck.VersionAsNumber(currentVersion))
				{
					// No need to check, we already know there is a new version
					m_UpdateAvailable = TRUE;
				}
				else
				{
					// Check on-line for a new version
					m_UpdateCheck.checkForUpdates(pMainFrame->m_hWnd, false);
				}
			}
			break;
		default:
			// Auto-updates are not configured
			AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_HELP_CHECKFORUPDATES);
			break;
		}
	}

	return TRUE;
}
//-------------------------------------------------------------------------

//=========================================================================
//== accessor                                                            ==
//=========================================================================

//-------------------------------------------------------------------------
bool CTinyCadApp::IsLibInUse(CLibraryStore* pLib)
{
	POSITION p = m_pDocTemplate->GetFirstDocPosition();

	while (p != NULL)
	{
		CTinyCadMultiDoc *t = static_cast<CTinyCadMultiDoc*> (m_pDocTemplate->GetNextDoc(p));
		if (t->IsLibInUse(pLib))
		{
			return true;
		}
	}

	p = m_pLibTemplate->GetFirstDocPosition();

	while (p != NULL)
	{
		CLibraryDoc *t = static_cast<CLibraryDoc*> (m_pLibTemplate->GetNextDoc(p));
		if (t->IsLibInUse(pLib))
		{
			return true;
		}
	}

	return false;
}
//-------------------------------------------------------------------------
CString CTinyCadApp::GetVersion()
{
	CString sReturn;
	TCHAR szModulePath[MAX_PATH];
	DWORD dwSize;
	DWORD dwZero;
	char* pBuffer;
	VS_FIXEDFILEINFO* pFixedInfo;
	UINT uVersionLen;

	GetModuleFileName(nullptr, szModulePath, MAX_PATH - 1);

	dwSize = GetFileVersionInfoSize(szModulePath, &dwZero);

	if (dwSize > 0)
	{
		pBuffer = new char[dwSize];

		GetFileVersionInfo(szModulePath, dwZero, dwSize, pBuffer);
		VerQueryValue(pBuffer, _T("\\"), (void**) &pFixedInfo, (UINT*) &uVersionLen);

		sReturn.Format(_T("%u.%02u.%02u"), HIWORD(pFixedInfo->dwProductVersionMS), LOWORD(pFixedInfo->dwProductVersionMS), HIWORD(pFixedInfo->dwProductVersionLS));
		delete[] pBuffer;
	}

	return sReturn;
}
//-------------------------------------------------------------------------
CString CTinyCadApp::GetReleaseType()
{
	// If we are building from "master" then assume it is a release build,
	// otherwise use the branch name
	if (strcmp(GIT_BRANCH, "master")==0)
	{
		return "Production Release";
	}
	return GIT_BRANCH;
}

//-------------------------------------------------------------------------
CString CTinyCadApp::GetName()
{
	return "TinyCAD";
}
//-------------------------------------------------------------------------
CString CTinyCadApp::GetMainDir()
{
	CString sReturn;
	TCHAR theBuffer[1024];
	DWORD theBytes = GetModuleFileName(NULL, theBuffer, (sizeof (theBuffer) - 1) / sizeof(TCHAR));
	TRACE("CTinyCadApp::GetModuleFileName() returned \"%S\"\n", theBuffer);
	if (theBytes != 0)
	{
		TCHAR* thePtr = theBuffer + theBytes;

		while (thePtr >= theBuffer && *thePtr != _TCHAR('\\'))
		{
			thePtr--;
		}
		*thePtr = _TCHAR('\0');

		sReturn = CString(theBuffer) + "\\";
	}

	return sReturn;
}

CString CTinyCadApp::GetMyDocumentDir(CString subfolder)
{
	/*
	 * Gets the default location of the TinyCAD libraries.
	 * So that we can install TinyCAD for "all users" on a PC, the examples and libraries are stored
	 * in the app's installation folder, we then copy them on first run in to My Documents so that they
	 * can be edited and genrally used.
	 * This also has the benefit that if a user uninstalls TinyCAD then the user's libraries are not
	 * also deleted.
	 * The My Documents path is found by concatenating the environment variable "USERPROFILE" contents with 
	 *		Windows XP:  "\My Documents\TinyCAD\libraries"
	 *		Windows Vista/7/8:  "\documents\TinyCAD\libraries"
	*/
	CString sReturn;
	TCHAR szPath[MAX_PATH];

	if(SUCCEEDED(SHGetFolderPath(NULL, 
								 CSIDL_PERSONAL|CSIDL_FLAG_CREATE, 
								 NULL, 
								 0, 
								 szPath))) 
	{
		TRACE("CTinyCadApp::GetLibraryDir() - SHGetFolderPath(CSIDL_PERSONAL) returned \"%S\"\n", szPath);
		PathAppend(szPath, TEXT("TinyCAD"));
		PathAppend(szPath, subfolder);
		TRACE("CTinyCadApp::GetLibraryDir() - concatenated string = \"%S\"\n", szPath);
	}
	else {	//May be pre- Windows XP - use the old method
		TRACE("CTinyCadApp::GetLibraryDir() - SHGetFolderPath(CSIDL_PERSONAL) returned failure code\n");
		szPath[0]='\0';

		DWORD theBytes = GetModuleFileName(NULL, szPath, (sizeof (szPath) - 1) / sizeof(TCHAR));
		TRACE("CTinyCadApp::GetModuleFileName() returned \"%S\"\n", szPath);
		if (theBytes != 0)
		{
			PathRemoveFileSpec(szPath);
			PathAppend(szPath, TEXT("library"));
		}
	}
	sReturn = CString(szPath) + "\\";

	return sReturn;
}


CString CTinyCadApp::GetAppDir(CString subfolder)
{
	CString sReturn;
	TCHAR szPath[MAX_PATH];

	szPath[0] = '\0';

	DWORD theBytes = GetModuleFileName(NULL, szPath, (sizeof(szPath) - 1) / sizeof(TCHAR));
	TRACE("CTinyCadApp::GetModuleFileName() returned \"%S\"\n", szPath);
	if (theBytes != 0)
	{
		PathRemoveFileSpec(szPath);
		PathAppend(szPath, subfolder);
	}
	sReturn = CString(szPath) + "\\";

	return sReturn;
}

CString CTinyCadApp::GetLongFileName(const CString shortFilename)
{
	//This function returns the newer format long filename (i.e., non-DOS 8.3 format) from a short file name.
	//It should work ok with a normal long filename also, if all you are trying to do is retrieve the full path.
	//It looks in the current working directory, so this must be set appropriately.
	TCHAR longFilename[MAX_PATH];
	CString sTemp = shortFilename;
	DWORD count = GetLongPathName(sTemp, longFilename, (sizeof (longFilename) - 1) / sizeof(TCHAR));
	if (count == 0 || longFilename[0] == 0) return CString(shortFilename); //error during GetLongPathName() or long pathname is too long for buffer or simply not available due to file system historical creation
	else return CString(longFilename);
}
//-------------------------------------------------------------------------

//=========================================================================
//== mutator                                                             ==
//=========================================================================


void CTinyCadApp::SetLockOutSymbolRedraw(bool r)
{
	m_LockOutSymbolRedraw = r;

	if (!m_LockOutSymbolRedraw)
	{
		ResetAllSymbols();
	}
}


//-------------------------------------------------------------------------

//=========================================================================
//==    manage views                                                     ==
//=========================================================================
//-------------------------------------------------------------------------
// Read the registry
void CTinyCadApp::ReadRegistry()
{
	// create singleton registry
	g_pRegistry = new CTinyCadRegistry();

	// Is there a list of libraries in the registry?
	CStringList* colLibs = CTinyCadRegistry::GetLibraryNames();

	// Iterate through the list in head-to-tail order.
	CString sSearch;
	for (POSITION pos = colLibs->GetHeadPosition(); pos != NULL;)
	{
		// One single library
		CString sLibName = colLibs->GetNext(pos); //This actually gets the current string that corresponds to *pos, and increments pos to point to the next string
		CLibraryStore* oLib = NULL;

		//TRACE("\nProcessing library:  sLibName=\"%S\"\n", sLibName);

		CString sSearch;

		//Library names are stored without type information.  Look for the newest library types first (.TCLib), then the older .mdb, then theh oldest .idx
		sSearch.Format(_T("%s.TCLib"), (LPCTSTR)sLibName);
		//TRACE("  1.  Looking to see if sSearch=\"%S\" can be opened\n",sSearch);
		FindFile theFind(sSearch);
		if (theFind.Success())
		{
			//the .TCLib file was found, so it must be a new SQLIte library file
			TRACE("    Found library file \"%S\".  This is an SQLite3 library.\n", sSearch);
			oLib = new CLibrarySQLite;
		}
		else
		{
			sSearch.Format(_T("%s.mdb"), (LPCTSTR)sLibName);
			//TRACE("  2.  Looking to see if sSearch=\"%S\" can be opened\n",sSearch);
			FindFile theFind(sSearch);
			if (theFind.Success())
			{ //the .mdb file was found, so it must be one of the older JET/DAO library files
				TRACE("    Found library file \"%S\".  This is a Microsoft JET/DAO library.\n", sSearch);
				oLib = new CLibraryDb;
			}
			else
			{
				sSearch.Format(_T("%s.idx"), (LPCTSTR)sLibName);
				FindFile theFind(sSearch);
				//TRACE("  3.  Looking to see if sSearch=\"%S\" can be opened\n",sSearch);
				if (theFind.Success())
				{ //the .idx file was found, so it must be an old library file (i.e., non-database format)
					TRACE("    Found library file \"%S\".  This is the oldest library type.\n", sSearch);
					oLib = new CLibraryFile;
				}
				else
				{ //no known library format was found
					CString s;
					s.Format(_T("    Library not found in any format:\r\n\t\"%s\"\r\nwhile looking for this library with one of the following extensions:  [.TCLib, .mdb, .idx]"), (LPCTSTR)sLibName);
					AfxMessageBox(s);
				}
			}
		}

		if (oLib)
		{
			//TRACE("Adding opened library \"%S\" to CLibraryCollection\n",sSearch);
			oLib->Attach(sLibName);
			CLibraryCollection::Add(oLib);
		}
	}

	delete colLibs;
}

//-------------------------------------------------------------------------
// Edit a library using the doc/view
void CTinyCadApp::EditSymbol(CLibraryStore* pLib, CLibraryStoreNameSet &symbol)
{
	CTinyCadMultiSymbolDoc *pMulti = new CTinyCadMultiSymbolDoc(pLib, symbol);

	CFrameWnd *pFrame = m_pDocTemplate->CreateNewFrame(pMulti, NULL);
	m_pDocTemplate->InitialUpdateFrame(pFrame, pMulti, TRUE);
}

//-------------------------------------------------------------------------
// Edit a text file using the doc/view
void CTinyCadApp::EditTextFile(const TCHAR *filename)
{
	TRACE("CTinyCADApp::EditTextFile(\"%S\")\n", filename);
	CTextEditDoc *pDoc = static_cast<CTextEditDoc *> (m_pTxtTemplate->CreateNewDocument());
	if (pDoc != NULL)
	{
		if (!pDoc->LoadFile(filename, true))
		{
			delete pDoc;
			return;
		}

		CFrameWnd *pFrame = m_pTxtTemplate->CreateNewFrame(pDoc, NULL);
		m_pTxtTemplate->InitialUpdateFrame(pFrame, pDoc, TRUE);
	}
}

//-------------------------------------------------------------------------
// Edit a design file using the doc/view
//
void CTinyCadApp::EditDesign(const TCHAR *filename)
{
	TRACE("CTinyCADApp::EditDesign(\"%S\")\n", filename);
	AfxGetApp()->OpenDocumentFile(filename);
}

//-------------------------------------------------------------------------
// Edit a library using the doc/view
void CTinyCadApp::EditLibrary(CLibraryStore* pLib)
{
	// Is this library already being edited?
	POSITION p = m_pLibTemplate->GetFirstDocPosition();
	while (p != NULL)
	{
		CLibraryDoc *t = static_cast<CLibraryDoc*> (m_pLibTemplate->GetNextDoc(p));
		if (t->IsLibInUse(pLib))
		{
			POSITION v = t->GetFirstViewPosition();

			// Activate it's views
			while (v != NULL)
			{
				CView* pView = t->GetNextView(v);
				pView->GetParentFrame()->BringWindowToTop();
			}

			return;
		}
	}

	CLibraryDoc *pDoc = static_cast<CLibraryDoc *> (m_pLibTemplate->CreateNewDocument());

	if (pDoc != NULL)
	{
		pDoc->setLibrary(pLib);
		CFrameWnd *pFrame = m_pLibTemplate->CreateNewFrame(pDoc, NULL);
		m_pLibTemplate->InitialUpdateFrame(pFrame, pDoc, TRUE);
	}
}
//-------------------------------------------------------------------------
void CTinyCadApp::ResetAllSymbols()
{
	if (!m_LockOutSymbolRedraw)
	{
		static_cast<CMainFrame*> (AfxGetMainWnd())->ResetAllSymbols();
	}

	POSITION p = m_pLibTemplate->GetFirstDocPosition();

	while (p != NULL)
	{
		CLibraryDoc *t = static_cast<CLibraryDoc*> (m_pLibTemplate->GetNextDoc(p));
		t->ResetSymbols();
	}
}
//-------------------------------------------------------------------------
void CTinyCadApp::SaveAll()
{
	// Save a backup copy of all of the files so far
	POSITION p = m_pDocTemplate->GetFirstDocPosition();

	while (p != NULL)
	{
		CTinyCadMultiDoc *t = static_cast<CTinyCadMultiDoc*> (m_pDocTemplate->GetNextDoc(p));

		// Force an auto-save...
		t->AutoSave();
	}

}
//-------------------------------------------------------------------------

//=========================================================================
//== Message handler                                                     ==
//=========================================================================
//---------------------------------------------------------------------

void CTinyCadApp::OnMyFileOpen()
{
	//manual open using CFileDialog to get around buggy Microsoft MFC code that affects Windows 8.1
	//example code copied and modified taken from http://yourprosoft.blogspot.com/2012/01/mfc-encountered-improper-argument.html
	
	CFileDialog pDlg(TRUE, _T("*.dsn"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					_T("TinyCAD file (*.dsn)|*.dsn|All files (*.*)|*.*||"), AfxGetMainWnd());

	if(pDlg.DoModal()==IDOK)
	{
		CString strDocFileName = pDlg.GetPathName();

		//the next line is optained from the following mfc source file
		//C:\Program Files\Microsoft Visual Studio 9.0\VC\atlmfc\src\mfc\docmgr.cpp
		AfxGetApp()->OpenDocumentFile(strDocFileName);
	}
}

//-------------------------------------------------------------------------
// This is the idle time processing
BOOL CTinyCadApp::OnIdle(LONG nCount)
{
	CWinAppEx::OnIdle(nCount);

	if (nCount == 0)
	{
		CLibraryCollection::DoIdle();
	}

	return FALSE;
}

//-------------------------------------------------------------------------
// Process the main window accelerator keys when a MFC dialog has the focus.
// This is an implementation suggested by Microsoft KB100770.
BOOL CTinyCadApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
	return CWinAppEx::ProcessMessageFilter(code, lpMsg);
}

//-------------------------------------------------------------------------
void CTinyCadApp::OnLibLib()
{
	CDlgLibraryBox(AfxGetMainWnd()).DoModal();
}
//-------------------------------------------------------------------------
void CTinyCadApp::OnHelpOpenTinyCADUserManual()
{
	// Open a browser for our web site
	ShellExecute(AfxGetMainWnd()->m_hWnd, _T("open"), _T("https://www.tinycad.net/Home/Documentation"), NULL, NULL, SW_SHOWNORMAL);
}
//-------------------------------------------------------------------------
void CTinyCadApp::OnHelpGototinycadwebsite()
{
	// Open a browser for our web site
	ShellExecute(AfxGetMainWnd()->m_hWnd, _T("open"), _T("https://www.tinycad.net"), NULL, NULL, SW_SHOWNORMAL);
}

//-------------------------------------------------------------------------

void CTinyCadApp::OnHelpSupport()
{
	// Open a browser for our web site
	ShellExecute(AfxGetMainWnd()->m_hWnd, _T("open"), _T("https://forum.tinycad.net"), NULL, NULL, SW_SHOWNORMAL);
}

//-------------------------------------------------------------------------
void CTinyCadApp::OnAppAbout()
{
	CDlgAbout().DoModal();
}
//-------------------------------------------------------------------------
void CTinyCadApp::OnAppUpdateCheck()
{
	if (CDlgUpdateCheck().DoModal() == ID_CHECK_NOW)
	{
		m_UpdateCheck.checkForUpdates(AfxGetMainWnd()->m_hWnd, true);
	}
}

void CTinyCadApp::OnAppUpdateChecked()
{
	// Is there a new update to tell the user about
	CString previousUpdateVersion = CTinyCadRegistry::GetLastAutomaticUpdateVersion();
	CString currentVersion = GetVersion();
	CString latestVersion = m_UpdateCheck.getLatestVersion();

	// Is there an update to be had?
	if (m_UpdateCheck.VersionAsNumber(currentVersion) < m_UpdateCheck.VersionAsNumber(latestVersion))
	{
		// Set the indicator
		m_UpdateAvailable = TRUE;

		// Have we never told the user about this or have we been asked to
		// report the outcome?
		if (latestVersion != previousUpdateVersion || m_UpdateCheck.getInformNoUpdate())
		{
			// Tell the user
			CTinyCadRegistry::SetLastAutomaticUpdateVersion(latestVersion);
			int result = AfxMessageBox(_T("There is a new verson TinyCAD available!\r\nClick OK to go to www.tinycad.net to download it or press cancel to dismiss this dialog."), MB_OKCANCEL);
			if (result == IDOK)
			{
				// Open a browser for our web site
				ShellExecute(AfxGetMainWnd()->m_hWnd, _T("open"), _T("https://www.tinycad.net/Home/Download"), NULL, NULL, SW_SHOWNORMAL);
			}

		}
	}
	else
	{
		// No new update
		m_UpdateAvailable = FALSE;
		if (m_UpdateCheck.getInformNoUpdate())
		{
			AfxMessageBox(_T("There is no new version of TinyCAD."), MB_OK);
		}
	}
}

void CTinyCadApp::OnUpdateUpdateIndicator(CCmdUI* pCmdUI)
{
	if (m_UpdateAvailable)
	{
		pCmdUI->Enable(TRUE);
		pCmdUI->SetText(_T("New version of TinyCAD available"));
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}

//-------------------------------------------------------------------------
void CTinyCadApp::OnHelpHelp()
{
	CString url = "file://" + CString(m_pszHelpFilePath);
	ShellExecute(AfxGetMainWnd()->m_hWnd, _T("open"), url, NULL, NULL, SW_SHOWNORMAL);

}
//-------------------------------------------------------------------------
BOOL CTinyCadApp::ChooseColor(COLORREF &col)
{
	// Bring up the colour dialogue...
	CHOOSECOLOR c;
	c.lStructSize = sizeof (c);
	c.hwndOwner = AfxGetMainWnd()->m_hWnd;
	c.hInstance = NULL;
	c.rgbResult = col;
	c.Flags = CC_ANYCOLOR | CC_RGBINIT;
	c.lCustData = 0;
	c.lpfnHook = NULL;
	c.lpTemplateName = NULL;
	c.lpCustColors = &m_colours[0];

	if (::ChooseColor(&c))
	{
		col = c.rgbResult;
		return TRUE;
	}

	return FALSE;
}

BOOL CTinyCadApp::LoadWindowPlacement(CRect& rectNormalPosition, int& nFflags, int& nShowCmd)
{
	if (m_hiddenWindow)
	{	// Force window to be hidden
		nShowCmd = SW_HIDE;
		BOOL result = CWinAppEx::LoadWindowPlacement(rectNormalPosition, nFflags, nShowCmd);
		nShowCmd = SW_HIDE;
		m_nCmdShow = SW_HIDE;
		return result;
	}
	return CWinAppEx::LoadWindowPlacement(rectNormalPosition, nFflags, nShowCmd);
}
