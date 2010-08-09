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

// NOTE: This is never compiled in.  It is used to 
// make VS.NET recognise that this is an MFC project.
#ifdef _DUMMY_
static CWinApp	theApp;
#endif




//*************************************************************************
//*                                                                       *
//*                  Main application class                               *
//*                                                                       *
//*************************************************************************

CTinyCadApp::CTinyCadApp()
{
}

CTinyCadApp::~CTinyCadApp()
{
	CLibraryCollection::Clear();
}



BEGIN_MESSAGE_MAP(CTinyCadApp, CWinApp)
	//{{AFX_MSG_MAP(CTinyCadApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(IDM_LIBLIB, OnLibLib)
	ON_COMMAND(ID_HELP_OPENTINYCADUSERMANUAL, OnHelpOpenTinyCADUserManual)
	ON_COMMAND(ID_HELP_GOTOTINYCADWEBSITE, OnHelpGototinycadwebsite)
	ON_COMMAND(ID_HELP_HELP, OnHelpHelp)
	ON_COMMAND(ID_HELP_EMAILFORSUPPORT, OnHelpEmailforsupport)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

//=========================================================================
//== init class variables                                                ==
//=========================================================================
// base class ctor creates the one and only application object
CTinyCadApp			CTinyCadApp::m_oInstance;
CMultiDocTemplate*	CTinyCadApp::m_pDocTemplate	= NULL;
CMultiDocTemplate*	CTinyCadApp::m_pLibTemplate	= NULL;
CMultiDocTemplate*	CTinyCadApp::m_pTxtTemplate	= NULL;
bool				CTinyCadApp::m_LockOutSymbolRedraw = false;
COLORREF			CTinyCadApp::m_colours[16];
HACCEL				CTinyCadApp::m_hAccelTable;


//=========================================================================
//== ctor/dtor/initializing                                              ==
//=========================================================================
BOOL CTinyCadApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

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

	// Load the accelerator table for the ProcessMessageFilter function
	m_hAccelTable = LoadAccelerators(AfxGetInstanceHandle(),
	                                 MAKEINTRESOURCE(IDR_MAINFRAME));

	// Change the registry key under which our settings are stored.
	SetRegistryKey( CTinyCadApp::GetName() );

	// Read the registry
	ReadRegistry();

	// Load standard INI file options (including MRU)
	LoadStdProfileSettings(6);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.
	m_pDocTemplate = new CMultiDocTemplate(
		IDR_TCADTYPE,
		RUNTIME_CLASS(CTinyCadMultiDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CTinyCadView));
	AddDocTemplate(m_pDocTemplate);

	m_pLibTemplate = new CMultiDocTemplate(
		IDR_LIBTYPE,
		RUNTIME_CLASS(CLibraryDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CLibraryView));
	AddDocTemplate(m_pLibTemplate);

	m_pTxtTemplate = new CMultiDocTemplate(
		IDR_TXTTYPE,
		RUNTIME_CLASS(CTextEditDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CTextEditView));
	AddDocTemplate(m_pTxtTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	//First free the string allocated by MFC at CWinApp startup.
	//The string is allocated before InitInstance is called.
	free((void*)m_pszHelpFilePath);
	//Change the name of the .HLP file.
	//The CWinApp destructor will free the memory.
	m_pszHelpFilePath=_tcsdup(GetMainDir() + _T("TinyCAD.chm"));
	SetHelpMode( afxHTMLHelp );


	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	if( CTinyCadRegistry::GetMaximize() && m_nCmdShow == 1)
	{
		m_nCmdShow = SW_SHOWMAXIMIZED;
	}

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	CAutoSave::Start();

	return TRUE;
}
//-------------------------------------------------------------------------

//=========================================================================
//== accessor                                                            ==
//=========================================================================

//-------------------------------------------------------------------------
bool CTinyCadApp::IsWinNT()
{
	bool			bReturn = true;
	OSVERSIONINFO	oVersion;

	oVersion.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	GetVersionEx( &oVersion );

	bReturn = (oVersion.dwPlatformId == VER_PLATFORM_WIN32_NT);

	return bReturn;
}
//-------------------------------------------------------------------------
bool CTinyCadApp::IsLibInUse( CLibraryStore* pLib )
{
	POSITION p = m_pDocTemplate->GetFirstDocPosition();

	while (p != NULL)
	{
		CTinyCadMultiDoc *t = static_cast<CTinyCadMultiDoc*>(m_pDocTemplate->GetNextDoc(p));
		if (t->IsLibInUse( pLib ))
		{
			return true;
		}
	}

	p = m_pLibTemplate->GetFirstDocPosition();

	while (p != NULL)
	{
		CLibraryDoc *t = static_cast<CLibraryDoc*>(m_pLibTemplate->GetNextDoc(p));
		if (t->IsLibInUse( pLib ))
		{
			return true;
		}
	}

	return false;
}
//-------------------------------------------------------------------------
CString CTinyCadApp::GetVersion()
{
	CString				sReturn;
	TCHAR				szModulePath[MAX_PATH];
	DWORD				dwSize;
	DWORD				dwZero;
	char*				pBuffer;
	VS_FIXEDFILEINFO*	pFixedInfo;
	UINT				uVersionLen;

	GetModuleFileName(NULL, szModulePath, MAX_PATH);
	dwSize = GetFileVersionInfoSize(szModulePath, &dwZero);

	if( dwSize > 0 )
	{
		pBuffer = new char[ dwSize ];

			GetFileVersionInfo(szModulePath, dwZero, dwSize, pBuffer);
		VerQueryValue(pBuffer, _T("\\"), (void**) &pFixedInfo, (UINT*) &uVersionLen);

		sReturn.Format( _T("Version %u.%02u.%02u Build #%s%s"), 
										HIWORD(pFixedInfo->dwProductVersionMS),
										LOWORD(pFixedInfo->dwProductVersionMS),
										HIWORD(pFixedInfo->dwProductVersionLS),
										SVN_WCRANGE,
										SVN_MODIFICATIONS_POSTFIX);
//										LOWORD(pFixedInfo->dwProductVersionLS));
		delete pBuffer;
	}

	return sReturn;
}
//-------------------------------------------------------------------------
CString CTinyCadApp::GetReleaseType()
{
//	return "Debug/Test Release (uncontrolled)";
//	return "Alpha Release";
//	return "Beta Release";
	return "Production Release";
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
	TCHAR 	theBuffer[1024];
	DWORD 	theBytes = GetModuleFileName(NULL,theBuffer,sizeof(theBuffer) - 1);

	if( theBytes != 0 )
	{
		TCHAR* thePtr = theBuffer + theBytes;

		while (thePtr >= theBuffer && *thePtr != _TCHAR('\\'))
  	{
			thePtr--;
		}
		*thePtr = _TCHAR('\0');

		sReturn = CString( theBuffer ) + "\\";
  }

	return sReturn;
}
//-------------------------------------------------------------------------

//=========================================================================
//== mutator                                                             ==
//=========================================================================
	

void CTinyCadApp::SetLockOutSymbolRedraw( bool r )
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
	new CTinyCadRegistry();

	// Is there a list of libraries in the registry?
	CStringList*	colLibs = CTinyCadRegistry::GetLibraryNames();

	// Iterate through the list in head-to-tail order.
	for( POSITION pos = colLibs->GetHeadPosition(); pos != NULL; )
	{
		// One single library
		CString			sLibName = colLibs->GetNext( pos );
		CLibraryStore*	oLib	= NULL;

		// Is this a new file library or an old library type?
		CString sSearch = sLibName + _T(".idx");
		FindFile theFind( sSearch );
		if (theFind.Success())
		{	//the .idx file was found, so it must be an old library file (i.e., non-database format)
			oLib = new CLibraryFile;
		}
		else
		{
			CString sSearch = sLibName + _T(".mdb");
			FindFile theFind( sSearch );
			if (theFind.Success())
			{	//the .mdb file was found, so it must be a jet library file
				oLib = new CLibraryDb;
			}
			else
			{
				CString sSearch = sLibName + _T(".TCLib");
				FindFile theFind( sSearch );
				if (theFind.Success())
				{	//the .TCLib file was found, so it must be a new SQLIte library file
					oLib = new CLibrarySQLite;
				}
				else
				{	//no known library format was found
					CString s;
					s.Format(_T("Library not found:\r\n%s"), sLibName);
					AfxMessageBox( s );
				}
			}
		}

		if (oLib)
		{
			oLib->Attach( sLibName );
			CLibraryCollection::Add( oLib );
		}
	}

	delete colLibs;
}

//-------------------------------------------------------------------------
// Edit a library using the doc/view
void CTinyCadApp::EditSymbol(CLibraryStore* pLib, CLibraryStoreNameSet &symbol)
{
	CTinyCadMultiSymbolDoc *pMulti = new CTinyCadMultiSymbolDoc( pLib, symbol );

	CFrameWnd *pFrame = m_pDocTemplate->CreateNewFrame( pMulti, NULL );
	m_pDocTemplate->InitialUpdateFrame( pFrame, pMulti, TRUE );
}


//-------------------------------------------------------------------------
// Edit a text file using the doc/view
void CTinyCadApp::EditTextFile( const TCHAR *filename )
{
	CTextEditDoc *pDoc = static_cast<CTextEditDoc *>(m_pTxtTemplate->CreateNewDocument());
	if (pDoc != NULL)
	{
		if (!pDoc->LoadFile( filename, true ))
		{
			delete pDoc;
			return;
		}

		CFrameWnd *pFrame = m_pTxtTemplate->CreateNewFrame( pDoc, NULL );
		m_pTxtTemplate->InitialUpdateFrame( pFrame, pDoc, TRUE );
	}
}

//-------------------------------------------------------------------------
// Edit a design file using the doc/view
void CTinyCadApp::EditDesign( const TCHAR *filename )
{
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
		CLibraryDoc *t = static_cast<CLibraryDoc*>(m_pLibTemplate->GetNextDoc(p));
		if (t->IsLibInUse( pLib ))
		{
			POSITION v = t->GetFirstViewPosition();
			
			// Active it's views
		   while (v != NULL)
		   {
			  CView* pView = t->GetNextView(v);
			  pView->GetParentFrame()->BringWindowToTop();
		   }   

		   return;
		}		
	}

	CLibraryDoc *pDoc = static_cast<CLibraryDoc *>(m_pLibTemplate->CreateNewDocument());

	if (pDoc != NULL)
	{
		pDoc->setLibrary( pLib );
		CFrameWnd *pFrame = m_pLibTemplate->CreateNewFrame( pDoc, NULL );
		m_pLibTemplate->InitialUpdateFrame( pFrame, pDoc, TRUE );
	}
}
//-------------------------------------------------------------------------
void CTinyCadApp::ResetAllSymbols()
{
	if (!m_LockOutSymbolRedraw)
	{
		static_cast<CMainFrame*>(AfxGetMainWnd())->ResetAllSymbols();
	}
	
	POSITION p = m_pLibTemplate->GetFirstDocPosition();

	while (p != NULL)
	{
		CLibraryDoc *t = static_cast<CLibraryDoc*>(m_pLibTemplate->GetNextDoc(p));
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
		CTinyCadMultiDoc *t = static_cast<CTinyCadMultiDoc*>(m_pDocTemplate->GetNextDoc(p));
		
		// Force an auto-save...
		t->AutoSave();
	}

}
//-------------------------------------------------------------------------

//=========================================================================
//== Message handler                                                     ==
//=========================================================================

//-------------------------------------------------------------------------
// This is the idle time processing
BOOL CTinyCadApp::OnIdle( LONG nCount )
{
  	CWinApp::OnIdle( nCount );

  	if( nCount == 0 )
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
    if(code >= 0)
	{
		if(m_hAccelTable && m_pMainWnd)
		{
			// If a library has been deleted since the last time that TinyCAD 			
			// was started, m_pMainWnd will be null when bringing up a dialog 			
			// box that will tell you that the library is missing.  For some reason,			
			// the other tests below did not catch this condition and caused a			
			// null pointer exception.			
			//
			BOOL translate = TRUE;

			// Allow simple text editing in dialogs.
			if(WM_KEYDOWN == lpMsg->message)
			{
				if(::GetKeyState(VK_CONTROL) < 0)
				{
					switch(lpMsg->wParam)
					{
					case 'Z':	// Undo
					case 'X':	// Cut
					case 'C':	// Copy
					case 'V':	// Paste
						translate = FALSE;
					}
				}
				else 
				{
					switch(lpMsg->wParam)
					{
					case VK_DELETE:	// Delete
						translate = FALSE;
					}
				}
			}

			// Not for popup windows like message boxes or modal dialogs
			if (translate && !((::GetWindowLong(::GetParent(lpMsg->hwnd), GWL_STYLE)) & WS_POPUP))
			{
				if (::TranslateAccelerator(m_pMainWnd->m_hWnd, m_hAccelTable, lpMsg)) 
					return TRUE;
			}
		}
	}	
    return CWinApp::ProcessMessageFilter(code, lpMsg);
}

//-------------------------------------------------------------------------
void CTinyCadApp::OnLibLib()
{
	CDlgLibraryBox( AfxGetMainWnd() ).DoModal();
}
//-------------------------------------------------------------------------
void CTinyCadApp::OnHelpOpenTinyCADUserManual() 
{
	// Open the default path folder - debug only
//	ShellExecute( AfxGetMainWnd()->m_hWnd, _T("open"), _T(".\\"), NULL, NULL, SW_SHOWNORMAL );

	// Open the TinyCAD User's Manual
	// For now, this will only work at the default installation 
	// directory on the c drive.  When I figure out how, I will 
	// upgrade this to use the directory that TinyCAD was started from.
	TRACE("Opening the TinyCAD User Manual using \"c:\\Program Files\\TinyCAD\\TinyCAD_Manual.pdf\"\n");
	int ret_code = 0;
	ret_code = (int) ShellExecute( AfxGetMainWnd()->m_hWnd, _T("open"), _T("c:\\Program Files\\TinyCAD\\TinyCAD_Manual.pdf"), NULL, NULL, SW_SHOWNORMAL );	
	TRACE("ShellExecute returned code %d\n", ret_code);
}
//-------------------------------------------------------------------------
void CTinyCadApp::OnHelpGototinycadwebsite() 
{
	// Open a browser for our web site
	ShellExecute( AfxGetMainWnd()->m_hWnd, _T("open"), _T("http://tinycad.sourceforge.net"), NULL, NULL, SW_SHOWNORMAL );	
}

//-------------------------------------------------------------------------

void CTinyCadApp::OnHelpEmailforsupport() 
{
	// Open a browser for our web site
	// ??ShellExecute( AfxGetMainWnd()->m_hWnd, "open", "emailto:don_lucas@sourceforge.net", NULL, NULL, SW_SHOWNORMAL );	
}


//-------------------------------------------------------------------------
void CTinyCadApp::OnAppAbout()
{
	CDlgAbout().DoModal();
}
//-------------------------------------------------------------------------
void CTinyCadApp::OnHelpHelp() 
{
	HtmlHelp(0,HH_DISPLAY_TOPIC);
}
//-------------------------------------------------------------------------
BOOL CTinyCadApp::ChooseColor(	COLORREF &col ) 
{
	// Bring up the colour dialogue...
	CHOOSECOLOR c;
	c.lStructSize = sizeof(c);
	c.hwndOwner = AfxGetMainWnd()->m_hWnd;
	c.hInstance = NULL;
	c.rgbResult = col;
	c.Flags = CC_ANYCOLOR | CC_RGBINIT;
	c.lCustData = 0;
	c.lpfnHook = NULL;
	c.lpTemplateName = NULL;
	c.lpCustColors = m_colours;
	
	if (::ChooseColor( &c ))
	{
		col = c.rgbResult;
		return TRUE;
	}

	return FALSE;
}
