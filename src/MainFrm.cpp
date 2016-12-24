/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "startup.h"
#include "TinyCad.h"
#include "TinyCadDoc.h"
#include "EditToolBar.h"
#include "DlgPositionBox.h"
#include "TinyCadRegistry.h"
#include "MainFrm.h"
#include "NewTypes.h"
#include "AutoSave.h"

#include <dde.h>

CEditToolbar g_EditToolBar;
/////////////////////////////////////////////////////////////////////////////
// CMainFrame
IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_COMMAND(ID_VIEW_ANNOTATIONTOOLBAR, OnViewAnnotationtoolbar)
	ON_COMMAND(ID_VIEW_EDITTOOLBAR, OnViewEdittoolbar)
	ON_COMMAND(ID_VIEW_FILETOOLBAR, OnViewFiletoolbar)
	ON_COMMAND(ID_VIEW_GROUPTOOLBAR, OnViewGrouptoolbar)
	ON_COMMAND(ID_VIEW_BUSTOOLBAR, OnViewBustoolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BUSTOOLBAR, OnUpdateViewBustoolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ANNOTATIONTOOLBAR, OnUpdateViewAnnotationtoolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_EDITTOOLBAR, OnUpdateViewEdittoolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FILETOOLBAR, OnUpdateViewFiletoolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GROUPTOOLBAR, OnUpdateViewGrouptoolbar)
	ON_COMMAND(IDM_VIEWSHOWPOS, OnViewshowpos)
	ON_UPDATE_COMMAND_UI(IDM_VIEWSHOWPOS, OnUpdateViewshowpos)
	ON_COMMAND(ID_RESIZE, OnResize)
	ON_COMMAND(ID_OPTIONS_TOOLBARS_SYMBOL, OnOptionsToolbarsSymbol)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_TOOLBARS_SYMBOL, OnUpdateOptionsToolbarsSymbol)
	ON_MESSAGE(WM_DDE_EXECUTE, OnDDEExecute)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	runAsConsoleApp = false;
	consoleAppRetCode = 0;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1) return -1;

	CRect r(0, 0, 0, 0);

	if (!m_wndGetFindBox.Create(this, CDlgGetFindBox::IDD, WS_CHILD | WS_VISIBLE | CBRS_LEFT | CBRS_GRIPPER | CBRS_SIZE_DYNAMIC, CDlgGetFindBox::IDD))
	{
		TRACE0("Failed to create toolbar for annotations\n");
		return -1; // fail to create
	}

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, r, IDR_MAINFRAME) || !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1; // fail to create
	}
	m_wndToolBar.AttachToolbarImages(16, IDR_MAINFRAME, 0, 0);

	if (!m_wndToolBarDraw.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, r, IDR_DRAWING) || !m_wndToolBarDraw.LoadToolBar(IDR_DRAWING))
	{
		TRACE0("Failed to create toolbar for drawing\n");
		return -1; // fail to create
	}
	m_wndToolBarDraw.AttachToolbarImages(16, IDR_DRAWING, 0, 0);

	if (!m_wndToolBarDrawBus.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, r, IDR_DRAWBUS) || !m_wndToolBarDrawBus.LoadToolBar(IDR_DRAWBUS))
	{
		TRACE0("Failed to create toolbar for buses\n");
		return -1; // fail to create
	}
	m_wndToolBarDrawBus.AttachToolbarImages(16, IDR_DRAWBUS, 0, 0);

	if (!m_wndToolBarDrawGroup.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, r, IDR_DRAWGROUP) || !m_wndToolBarDrawGroup.LoadToolBar(IDR_DRAWGROUP))
	{
		TRACE0("Failed to create toolbar for group operations\n");
		return -1; // fail to create
	}
	m_wndToolBarDrawGroup.AttachToolbarImages(16, IDR_DRAWGROUP, 0, 0);

	if (!m_wndToolBarAnnotate.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, r, IDR_ANNOTATE) || !m_wndToolBarAnnotate.LoadToolBar(IDR_ANNOTATE))
	{
		TRACE0("Failed to create toolbar for annotations\n");
		return -1; // fail to create
	}
	m_wndToolBarAnnotate.AttachToolbarImages(16, IDR_ANNOTATE, 0, 0);

	if (!m_wndPositionBox.Create(this, CDlgPositionBox::IDD, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CDlgPositionBox::IDD))
	{
		TRACE0("Failed to create toolbar for annotations\n");
		return -1; // fail to create
	}

	if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(indicators, sizeof (indicators) / sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1; // fail to create
	}

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBarDraw.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBarAnnotate.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBarDrawBus.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBarDrawGroup.EnableDocking(CBRS_ALIGN_ANY);
	m_wndPositionBox.EnableDocking(CBRS_ALIGN_ANY);
	m_wndGetFindBox.EnableDocking(CBRS_ALIGN_LEFT);

	EnableDocking(CBRS_ALIGN_ANY);

	DockControlBar(&m_wndToolBar, AFX_IDW_DOCKBAR_TOP);
	RecalcLayout();

	CRect rect;
	m_wndToolBar.GetWindowRect(&rect);
	rect.left = rect.right - 2;
	rect.right = rect.right + 40;

	DockControlBar(&m_wndToolBarDraw, AFX_IDW_DOCKBAR_TOP, &rect);
	rect.left++;
	DockControlBar(&m_wndToolBarAnnotate, AFX_IDW_DOCKBAR_TOP, &rect);
	rect.left++;
	DockControlBar(&m_wndToolBarDrawBus, AFX_IDW_DOCKBAR_TOP, &rect);
	rect.left++;
	DockControlBar(&m_wndToolBarDrawGroup, AFX_IDW_DOCKBAR_TOP, &rect);
	rect.left++;
	DockControlBar(&m_wndPositionBox, AFX_IDW_DOCKBAR_TOP, &rect);

	rect.top++;
	rect.bottom++;
	DockControlBar(&m_wndGetFindBox, AFX_IDW_DOCKBAR_LEFT, &rect);

	LoadBarState(_T("Settings"));

	g_EditToolBar.Create(this);
	GetWindowRect(m_oldRect);

	new CDlgStartUpWindow(this);
	return 0;
}

void CMainFrame::ResetAllSymbols()
{
	m_wndGetFindBox.ResetAllSymbols();
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG
/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIFrameWnd::OnSize(nType, cx, cy);

	if (nType != SIZE_MINIMIZED)
	{
		CRect newRect;
		GetWindowRect(newRect);

		CRect currentRect;
		g_EditToolBar.GetWindowRect(currentRect);
		currentRect -= CPoint(m_oldRect.right - newRect.right, m_oldRect.top - newRect.top);
		g_EditToolBar.MoveWindow(currentRect);

		m_oldRect = newRect;

		CTinyCadRegistry::SetMaximize(IsZoomed() != 0);

		RecalcLayout();
	}
}

void CMainFrame::OnMove(int x, int y)
{
	CMDIFrameWnd::OnMove(x, y);

	CRect newRect;
	GetWindowRect(newRect);
	CRect currentRect;
	g_EditToolBar.GetWindowRect(currentRect);
	currentRect -= CPoint(m_oldRect.right - newRect.right, m_oldRect.top - newRect.top);
	g_EditToolBar.MoveWindow(currentRect);
	m_oldRect = newRect;

}

void CMainFrame::OnClose()
{
	SaveBarState(_T("Settings"));

	CMDIFrameWnd::OnClose();
}

void CMainFrame::OnViewAnnotationtoolbar()
{
	ShowControlBar(&m_wndToolBarAnnotate, (m_wndToolBarAnnotate.IsWindowVisible() == 0), FALSE);
}

void CMainFrame::OnUpdateViewAnnotationtoolbar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndToolBarAnnotate.IsWindowVisible());

}

void CMainFrame::OnViewBustoolbar()
{
	ShowControlBar(&m_wndToolBarDrawBus, (m_wndToolBarDrawBus.IsWindowVisible() == 0), FALSE);

}

void CMainFrame::OnUpdateViewBustoolbar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndToolBarDrawBus.IsWindowVisible());

}

void CMainFrame::OnViewEdittoolbar()
{
	ShowControlBar(&m_wndToolBarDraw, (m_wndToolBarDraw.IsWindowVisible() == 0), FALSE);
}

void CMainFrame::OnUpdateViewEdittoolbar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndToolBarDraw.IsWindowVisible());

}

void CMainFrame::OnViewFiletoolbar()
{
	ShowControlBar(&m_wndToolBar, (m_wndToolBar.IsWindowVisible() == 0), FALSE);

}

void CMainFrame::OnUpdateViewFiletoolbar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndToolBar.IsWindowVisible());

}

void CMainFrame::OnViewGrouptoolbar()
{
	ShowControlBar(&m_wndToolBarDrawGroup, (m_wndToolBarDrawGroup.IsWindowVisible() == 0), FALSE);

}

void CMainFrame::OnUpdateViewGrouptoolbar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndToolBarDrawGroup.IsWindowVisible());

}

void CMainFrame::OnViewshowpos()
{
	m_wndPositionBox.ShowWindow(m_wndPositionBox.IsWindowVisible() ? SW_HIDE : SW_SHOW);
}

void CMainFrame::OnUpdateViewshowpos(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPositionBox.IsWindowVisible());

}

void CMainFrame::OnResize()
{
	RecalcLayout();
}

void CMainFrame::OnOptionsToolbarsSymbol()
{
	ShowControlBar(&m_wndGetFindBox, (m_wndGetFindBox.IsWindowVisible() == 0), FALSE);

}

void CMainFrame::OnUpdateOptionsToolbarsSymbol(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndGetFindBox.IsWindowVisible());

}
//-------------------------------------------------------------------------
//--
void CMainFrame::OnTimer(UINT t)
{
	// Stop the timer during the save
	CAutoSave::Stop();

	// Perform the save
	CTinyCadApp::SaveAll();

	// Restart the timer...
	CAutoSave::Start();
}
//-------------------------------------------------------------------------
void CMainFrame::setPositionText(CString sPos)
{
	m_wndPositionBox.SetPosition(sPos);
}
//-------------------------------------------------------------------------
CLibraryStoreSymbol* CMainFrame::GetSelectSymbol()
{
	m_wndGetFindBox.AddToMRU();

	return m_wndGetFindBox.GetSelectSymbol();
}
//-------------------------------------------------------------------------

// COPIED OUT OF WINFRM.CPP DUE TO BUG IN MFC 7.1!
// always ACK the execute command - even if we do nothing
LRESULT CMainFrame::OnDDEExecute(WPARAM wParam, LPARAM lParam)
{
	// unpack the DDE message
	UINT_PTR unused;
	HGLOBAL hData;
	//IA64: Assume DDE LPARAMs are still 32-bit
	VERIFY(UnpackDDElParam(WM_DDE_EXECUTE, lParam, &unused, (UINT_PTR*)&hData));

	// get the command string
	TCHAR szCommand[_MAX_PATH * 2];
	LPCTSTR lpsz = (LPCTSTR) GlobalLock(hData);
	int commandLength = lstrlen(lpsz);
	lstrcpyn(szCommand, lpsz, sizeof (szCommand) / sizeof(TCHAR));
	if (commandLength >= sizeof (szCommand))
	{
		// The command would be truncated. This could be a security problem
		TRACE0("Warning: Command was ignored because it was too long.\n");
		return 0;
	}
	GlobalUnlock(hData);

	// acknowledge now - before attempting to execute
	::PostMessage((HWND) wParam, WM_DDE_ACK, (WPARAM) m_hWnd,
	//IA64: Assume DDE LPARAMs are still 32-bit
	ReuseDDElParam(lParam, WM_DDE_EXECUTE, WM_DDE_ACK, (UINT) 0x8000, (UINT_PTR) hData));

	// don't execute the command when the window is disabled
	if (!IsWindowEnabled())
	{
		TRACE(traceAppMsg, 0, _T("Warning: DDE command '%s' ignored because window is disabled.\n"), szCommand);
		return 0;
	}

	// execute the command
	if (!AfxGetApp()->OnDDECommand(szCommand)) TRACE(traceAppMsg, 0, _T("Error: failed to execute DDE command '%s'.\n"), szCommand);

	return 0L;
}
