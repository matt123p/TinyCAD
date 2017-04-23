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
IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
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
//	ON_MESSAGE(WM_DDE_EXECUTE, OnDDEExecute)
	ON_REGISTERED_MESSAGE(AFX_WM_RESETTOOLBAR, OnToolbarReset)
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
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1) return -1;

	// enable Office XP look:
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));

	//const CRect r(10, 10, 10, 10);
	const CRect r(1, 1, 1, 1);

	const DWORD dwCtrlStyle = TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CBRS_SIZE_DYNAMIC;
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_GRIPPER | CBRS_HIDE_INPLACE | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC;
	
	if (!m_wndGetFindBox.Create(_T("Find Symbol"), this, TRUE, MAKEINTRESOURCE(CDlgGetFindBox::IDD),
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI, CDlgGetFindBox::IDD))
	{
		TRACE0("Failed to create symbol pane\n");
		return -1; // fail to create
	}


	CMFCToolBarInfo tbi1;
	if (!m_wndToolBar.CreateEx(this, dwCtrlStyle, dwStyle | CBRS_TOP, r, IDR_MAINFRAME) ||
		!m_wndToolBar.LoadToolBarEx(IDR_MAINFRAME, tbi1, TRUE))
	{
		TRACE0("Failed to create toolbar\n");
		return -1; // fail to create
	}
	//CSize szImage, szButton;
	//szImage = m_wndToolBar.GetImageSize();
	//szButton.cx = szImage.cx + 6;
	//szButton.cy = szImage.cy + 6;
	//m_wndToolBar.SetMenuSizes(szButton, szImage);  // button size must be at least 6 bigger than image size

	//m_wndToolBar.AttachToolbarImages(16, IDR_MAINFRAME, 0, 0);

	CMFCToolBarInfo tbi2;
	if (!m_wndToolBarDraw.CreateEx(this, dwCtrlStyle, dwStyle | CBRS_TOP, r, IDR_DRAWING) ||
		!m_wndToolBarDraw.LoadToolBarEx(IDR_DRAWING, tbi2, TRUE))
	{
		TRACE0("Failed to create toolbar for drawing\n");
		return -1; // fail to create
	}
	//szImage = m_wndToolBarDraw.GetImageSize();
	//szButton.cx = szImage.cx + 6;
	//szButton.cy = szImage.cy + 6;
	//m_wndToolBarDraw.SetMenuSizes(szButton, szImage);  // button size must be at least 6 bigger than image size

	CMFCToolBarInfo tbi3;
	if (!m_wndToolBarDrawBus.CreateEx(this, dwCtrlStyle, dwStyle | CBRS_TOP, r, IDR_DRAWBUS) ||
		!m_wndToolBarDrawBus.LoadToolBarEx(IDR_DRAWBUS, tbi3, TRUE))
	{
		TRACE0("Failed to create toolbar for buses\n");
		return -1; // fail to create
	}

	CMFCToolBarInfo tbi4;
	if (!m_wndToolBarDrawGroup.CreateEx(this, dwCtrlStyle, dwStyle | CBRS_TOP, r, IDR_DRAWGROUP) ||
		!m_wndToolBarDrawGroup.LoadToolBarEx(IDR_DRAWGROUP, tbi4, TRUE))
	{
		TRACE0("Failed to create toolbar for group operations\n");
		return -1; // fail to create
	}

	CMFCToolBarInfo tbi5;

	if (!m_wndToolBarAnnotate.CreateEx(this, dwCtrlStyle, dwStyle | CBRS_TOP, r, IDR_ANNOTATE) ||
		!m_wndToolBarAnnotate.LoadToolBarEx(IDR_ANNOTATE, tbi5, TRUE))
	{
		TRACE0("Failed to create toolbar for annotations\n");
		return -1; // fail to create
	}

	CMFCToolBarInfo tbi6;
	if (!m_wndPositionBox.CreateEx(this, dwCtrlStyle, dwStyle | CBRS_TOP, r, IDR_POSITION) ||
		!m_wndPositionBox.LoadToolBarEx(IDR_POSITION, tbi6, TRUE))
	{
		TRACE0("Failed to create toolbar for position\n");
		return -1; // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators, sizeof (indicators) / sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1; // fail to create
	}

	EnableDocking(CBRS_ALIGN_ANY);
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBarDraw.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBarAnnotate.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBarDrawBus.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBarDrawGroup.EnableDocking(CBRS_ALIGN_ANY);
	m_wndPositionBox.EnableDocking(CBRS_ALIGN_ANY);
	//m_wndPositionBox.EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM);
	m_wndGetFindBox.EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);
	
	// This will show the toolbars in the proper order
	DockPane(&m_wndPositionBox);
	DockPaneLeftOf(&m_wndToolBarDrawGroup, &m_wndPositionBox);
	DockPaneLeftOf(&m_wndToolBarDrawBus, &m_wndToolBarDrawGroup);
	DockPaneLeftOf(&m_wndToolBarAnnotate, &m_wndToolBarDrawBus);
	DockPaneLeftOf(&m_wndToolBarDraw, &m_wndToolBarAnnotate);
	DockPaneLeftOf(&m_wndToolBar, &m_wndToolBarDraw);

	DockPane(&m_wndGetFindBox);

//	EnableToolTips(TRUE);


	//LoadBarState(_T("Settings"));

	g_EditToolBar.Create(this);
	GetWindowRect(m_oldRect);

	//RecalcLayout();

	//UpdateWindow();

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
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}

#endif //_DEBUG
/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIFrameWndEx::OnSize(nType, cx, cy);

	if (nType != SIZE_MINIMIZED)
	{
		CRect newRect;
		GetWindowRect(newRect);

		if (g_EditToolBar)
		{
			CRect currentRect;
			g_EditToolBar.GetWindowRect(currentRect);
			currentRect -= CPoint(m_oldRect.right - newRect.right, m_oldRect.top - newRect.top);
			g_EditToolBar.MoveWindow(currentRect);
		}

		m_oldRect = newRect;

		CTinyCadRegistry::SetMaximize(IsZoomed() != 0);

		RecalcLayout();
	}
}

void CMainFrame::OnMove(int x, int y)
{
	CMDIFrameWndEx::OnMove(x, y);

	CRect newRect;
	GetWindowRect(newRect);

	CPoint deltaright(m_oldRect.right - newRect.right, m_oldRect.top - newRect.top);

	// Move tool window
	CRect currentRect;
	g_EditToolBar.GetWindowRect(currentRect);
	currentRect -= deltaright;
	g_EditToolBar.MoveWindow(currentRect);

	// Make sure we are not minimizing or restoring
	CPoint deltaleft(m_oldRect.left - newRect.left, m_oldRect.top - newRect.top);
	if (abs(deltaleft.x) + abs(deltaleft.y) < 50000)
	{
		// Move floating symbols window too
		if (m_wndGetFindBox.IsVisible() && m_wndGetFindBox.IsFloating())
		{
			m_wndGetFindBox.GetParent()->GetWindowRect(currentRect);
			currentRect -= deltaleft;
			m_wndGetFindBox.GetParent()->MoveWindow(currentRect);
		}
	}
	m_oldRect = newRect;

}

void CMainFrame::OnClose()
{
	SaveBarState(_T("Settings"));

	CMDIFrameWndEx::OnClose();
}

void CMainFrame::OnViewAnnotationtoolbar()
{
	ShowPane(&m_wndToolBarAnnotate, !(m_wndToolBarAnnotate.IsVisible()), FALSE, TRUE);
	RecalcLayout();
}

void CMainFrame::OnUpdateViewAnnotationtoolbar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndToolBarAnnotate.IsWindowVisible());
}

void CMainFrame::OnViewBustoolbar()
{
	ShowPane(&m_wndToolBarDrawBus, !(m_wndToolBarDrawBus.IsVisible()), FALSE, TRUE);
	RecalcLayout();
}

void CMainFrame::OnUpdateViewBustoolbar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndToolBarDrawBus.IsWindowVisible());
}

void CMainFrame::OnViewEdittoolbar()
{
	ShowPane(&m_wndToolBarDraw, !(m_wndToolBarDraw.IsVisible()), FALSE, TRUE);
	RecalcLayout();
}

void CMainFrame::OnUpdateViewEdittoolbar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndToolBarDraw.IsWindowVisible());
}

void CMainFrame::OnViewFiletoolbar()
{
	ShowPane(&m_wndToolBar, !(m_wndToolBar.IsVisible()), FALSE, TRUE);
	RecalcLayout();
}

void CMainFrame::OnUpdateViewFiletoolbar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndToolBar.IsWindowVisible());
}

void CMainFrame::OnViewGrouptoolbar()
{
	ShowPane(&m_wndToolBarDrawGroup, !(m_wndToolBarDrawGroup.IsVisible()), FALSE, TRUE);
	RecalcLayout();
}

void CMainFrame::OnUpdateViewGrouptoolbar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndToolBarDrawGroup.IsWindowVisible());
}

void CMainFrame::OnViewshowpos()
{
	ShowPane(&m_wndPositionBox, !(m_wndPositionBox.IsVisible()), FALSE, TRUE);
	RecalcLayout();
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
	ShowPane(&m_wndGetFindBox, !(m_wndGetFindBox.IsVisible()), FALSE, TRUE);
	RecalcLayout();
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
void CMainFrame::setPositionText(const CString sPos)
{
	m_wndPositionBox.SetPosition(sPos);
}
//-------------------------------------------------------------------------
void CMainFrame::setGridSize(double grid, int units)
{
	m_wndPositionBox.SetGridSize(grid, units);
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
//LRESULT CMainFrame::OnDDEExecute(WPARAM wParam, LPARAM lParam)
//{
//	// unpack the DDE message
//	UINT_PTR unused;
//	HGLOBAL hData;
//	//IA64: Assume DDE LPARAMs are still 32-bit
//	VERIFY(UnpackDDElParam(WM_DDE_EXECUTE, lParam, &unused, (UINT_PTR*)&hData));
//
//	// get the command string
//	TCHAR szCommand[_MAX_PATH * 2];
//	LPCTSTR lpsz = (LPCTSTR) GlobalLock(hData);
//	int commandLength = lstrlen(lpsz);
//	lstrcpyn(szCommand, lpsz, sizeof (szCommand) / sizeof(TCHAR));
//	if (commandLength >= sizeof (szCommand))
//	{
//		// The command would be truncated. This could be a security problem
//		TRACE0("Warning: Command was ignored because it was too long.\n");
//		return 0;
//	}
//	GlobalUnlock(hData);
//
//	// acknowledge now - before attempting to execute
//	::PostMessage((HWND) wParam, WM_DDE_ACK, (WPARAM) m_hWnd,
//	//IA64: Assume DDE LPARAMs are still 32-bit
//	ReuseDDElParam(lParam, WM_DDE_EXECUTE, WM_DDE_ACK, (UINT) 0x8000, (UINT_PTR) hData));
//
//	// don't execute the command when the window is disabled
//	if (!IsWindowEnabled())
//	{
//		TRACE(traceAppMsg, 0, _T("Warning: DDE command '%s' ignored because window is disabled.\n"), szCommand);
//		return 0;
//	}
//
//	// execute the command
//	if (!AfxGetApp()->OnDDECommand(szCommand)) TRACE(traceAppMsg, 0, _T("Error: failed to execute DDE command '%s'.\n"), szCommand);
//
//	return 0L;
//}

LRESULT CMainFrame::OnToolbarReset(WPARAM wp, LPARAM)
{
	UINT uiToolBarId = (UINT)wp;

	switch (uiToolBarId)
	{
		case IDR_POSITION:
		{
			// Create label on toolbar
			CToolbarLabel position(POSITIONBOX_POS, 120);
			position.m_strText = "";
			m_wndPositionBox.ReplaceButton(POSITIONBOX_POS, position);

			// grid POSITIONBOX_GRIDSIZE
			CMFCToolBarEditBoxButton gridSize(POSITIONBOX_GRIDSIZE, GetCmdMgr()->GetCmdImage(POSITIONBOX_GRIDSIZE, FALSE), 128UL, 40);
			gridSize.m_strText = "";
			m_wndPositionBox.ReplaceButton(POSITIONBOX_GRIDSIZE, gridSize);

			// units POSITIONBOX_GRIDUNITS
			CToolbarLabel units(POSITIONBOX_GRIDUNITS, 30);
			units.m_strText = "";
			m_wndPositionBox.ReplaceButton(POSITIONBOX_GRIDUNITS, units);

			break;
		}
	}

	return 0;
}