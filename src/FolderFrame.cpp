////////////////////////////////////////////////////////////////
// MSDN Magazine -- October 2002
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with Visual Studio 6.0 on Windows XP. Tab size=3.
//
#include "stdafx.h"
#include "FolderFrame.h"
#include <afxpriv.h> // for WM_INITIALUPDATE


const int MINSCROLLBARWIDTH = 8*GetSystemMetrics(SM_CXVSCROLL);

BEGIN_WINDOW_MAP(MyFrameMap)
BEGINROWS(WRCT_TOFIT,0,0)
  BEGINCOLS(WRCT_REST,0,0)
	 RCREST(AFX_IDW_PANE_FIRST)
	 RCTOFIT(AFX_IDW_VSCROLL_FIRST)
  ENDGROUP()
  BEGINCOLS(WRCT_TOFIT,0,0)
    RCTOFIT(ID_WIN_FOLDERTABS)
    RCFIXED(ID_WIN_SIZERBAR,8)
    RCREST(AFX_IDW_HSCROLL_FIRST)
    RCTOFIT(ID_WIN_SIZEBOX)
  ENDGROUP()
ENDGROUP()
END_WINDOW_MAP()

IMPLEMENT_DYNAMIC(CFolderFrame, CWnd)
BEGIN_MESSAGE_MAP(CFolderFrame, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_REGISTERED_MESSAGE(WM_WINMGR, OnWinMgr)
END_MESSAGE_MAP()

CFolderFrame::CFolderFrame() : m_winMgr(MyFrameMap)
{
	m_cxFolderTabCtrl=hide;
}

CFolderFrame::~CFolderFrame()
{
}

//////////////////
// pre-creation: use client edge (assumes Win4)
//
BOOL CFolderFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style |= WS_CLIPCHILDREN;
	cs.style &= ~WS_BORDER;
   return CWnd::PreCreateWindow(cs);
}

//////////////////
// Create window: register class, create view
//
BOOL CFolderFrame::Create(CWnd* pParent, CRuntimeClass* pViewClass,
	CCreateContext* pcc, UINT nIDRes, int cxFolderTabCtrl, DWORD dwStyle)
{
	ASSERT(pParent);
	ASSERT(dwStyle & WS_CHILD);

	static LPCTSTR lpClassName = NULL;
	if (!lpClassName) {
		lpClassName = _T("PxlFolderFrame");
		WNDCLASS wc;
		memset(&wc, 0, sizeof(wc));
		wc.style = 0;
		wc.lpfnWndProc = (WNDPROC)::DefWindowProc; // will get hooked by MFC
		wc.hInstance = AfxGetInstanceHandle();
      wc.hCursor = LoadCursor(NULL, IDC_ARROW);
      wc.hbrBackground = NULL; //CreateSolidBrush(GetSysColor(COLOR_3DFACE));
      wc.lpszMenuName = NULL;
      wc.lpszClassName = lpClassName;
		VERIFY(AfxRegisterClass(&wc));
	}
	m_nIDRes = nIDRes;

	CRect rc(0,0,0,0);
	if (!CreateEx(0,lpClassName,NULL,dwStyle,rc,pParent,AFX_IDW_PANE_FIRST))
		return FALSE;
	
	CWnd* pView = (CWnd*)pViewClass->CreateObject();
	ASSERT(pView);
	if (!pView->Create(NULL, NULL, dwStyle, rc, this, AFX_IDW_PANE_FIRST, pcc))
		return FALSE;

	return TRUE;
}

//////////////////
// Now create scroll bars and folder tab control
//
int CFolderFrame::OnCreate(LPCREATESTRUCT lpcs)
{
	CRect rc(0,0,0,0);
	VERIFY(m_wndSBHorz.Create(WS_VISIBLE|WS_CHILD|SBS_HORZ,
		rc, this, AFX_IDW_HSCROLL_FIRST));
	VERIFY(m_wndSBVert.Create(WS_VISIBLE|WS_CHILD|SBS_VERT,
		rc, this, AFX_IDW_VSCROLL_FIRST));
	VERIFY(m_wndSBBox.Create(WS_VISIBLE|WS_CHILD|SBS_SIZEBOX,
		rc, this, ID_WIN_SIZEBOX));
	
	CFolderTabCtrl& ftc = m_wndFolderTabCtrl;
	VERIFY(ftc.Create(WS_CHILD|WS_VISIBLE, rc, this, ID_WIN_FOLDERTABS,
		FTS_BUTTONS));
	if (m_nIDRes)
		ftc.Load(m_nIDRes);

	ftc.AddItem(_T("Sheet 1"));

	// Create sizer bar for license/logo group
	VERIFY(m_wndSizerBar.Create(WS_VISIBLE|WS_CHILD,
		this, m_winMgr, ID_WIN_SIZERBAR));

	m_cxFolderTabCtrl = m_wndFolderTabCtrl.GetDesiredWidth();

	return 0;
}

//////////////////
// window was sized: move view and scroll bars
//
void CFolderFrame::OnSize(UINT nType, int cx, int cy)
{
	CWnd* pView = GetView();
	if (pView) {
		m_winMgr.CalcLayout(cx,cy,this);
		m_winMgr.SetWindowPositions(this);
	}
}

//////////////////
// Handle WM_WINMGR
//
LRESULT CFolderFrame::OnWinMgr(WPARAM wp, LPARAM lp)
{
	ASSERT(lp);
	NMWINMGR& nmw = *(NMWINMGR*)lp;
	if (nmw.code==NMWINMGR::GET_SIZEINFO) {
		switch (wp) {
		case AFX_IDW_HSCROLL_FIRST:
			nmw.sizeinfo.szMin =
				CSize(MINSCROLLBARWIDTH, GetSystemMetrics(SM_CYHSCROLL));
			return TRUE;
		case AFX_IDW_VSCROLL_FIRST:
			nmw.sizeinfo.szMin.cx = GetSystemMetrics(SM_CXVSCROLL);
			return TRUE;
		case ID_WIN_SIZEBOX:
			nmw.sizeinfo.szMin = CSize(GetSystemMetrics(SM_CXVSCROLL),
				GetSystemMetrics(SM_CYHSCROLL));
			return TRUE;
		case ID_WIN_FOLDERTABS:
			nmw.sizeinfo.szDesired.cx = m_cxFolderTabCtrl;
			return TRUE;
		}

	} else if (nmw.code==NMWINMGR::SIZEBAR_MOVED) {
		// User moved a sizer bar: call WinMgr to do it!
		m_winMgr.MoveRect(wp, nmw.sizebar.ptMoved, this);
		m_winMgr.SetWindowPositions(this);
		m_cxFolderTabCtrl += nmw.sizebar.ptMoved.x;
		return TRUE;
	}
	return FALSE; // not handled
}

//////////////////
// Show controls (scroll bars and folder tab control).
//
void CFolderFrame::ShowControls(int width)
{
	if (width == bestFit)
		width = m_wndFolderTabCtrl.GetDesiredWidth();

	m_cxFolderTabCtrl = width;

	int iShow = m_cxFolderTabCtrl>0 ? SW_SHOW : SW_HIDE;
	m_wndSBVert.ShowWindow(iShow);
	m_wndSBHorz.ShowWindow(iShow);
	m_wndSBBox.ShowWindow(iShow);
	m_wndFolderTabCtrl.ShowWindow(iShow);

	CWnd* pView = GetView();
	if (pView) 
	{
		CRect r;
		GetClientRect( r );
		m_winMgr.CalcLayout(r.Width(),r.Height(),this);
		m_winMgr.SetWindowPositions(this);
	}
}


CScrollBar* CFolderFrame::GetScrollBar(int nBar) const
{
	return nBar==SB_HORZ ? const_cast<CScrollBar*>(&m_wndSBHorz)
		: nBar==SB_VERT ? const_cast<CScrollBar*>(&m_wndSBVert) : NULL;
}

//////////////////
// pass scroll messages to view
//
void CFolderFrame::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	GetView()->SendMessage(WM_HSCROLL,
		MAKELONG(nSBCode, nPos), (LPARAM)pScrollBar->m_hWnd);
}
void CFolderFrame::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	GetView()->SendMessage(WM_VSCROLL,
		MAKELONG(nSBCode, nPos), (LPARAM)pScrollBar->m_hWnd);
}

//////////////////
// command routing: pass commands to view and myself
//
BOOL CFolderFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra,
	AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (GetView()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;
	return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

////////////////////////////////////////////////////////////////
// CFolderView
//
IMPLEMENT_DYNAMIC(CFolderView, CView)
BEGIN_MESSAGE_MAP(CFolderView, CView)
	ON_NOTIFY(FTN_TABCHANGED, 1, OnChangedTab)
	ON_NOTIFY(FTN_CONTEXTMENU, 1, OnContextMenu)
END_MESSAGE_MAP()

/////////////////
// Get scrollbar: send message to parent
//
CScrollBar* CFolderView::GetScrollBarCtrl(int nBar) const
{
	return GetFolderFrame()->GetScrollBar(nBar);
}

//////////////////
// Changed tab
//
void CFolderView::OnChangedTab(NMHDR* nmtab, LRESULT* pRes)
{
	NMFOLDERTAB& nmft = *(NMFOLDERTAB*)nmtab;
	OnChangedFolder(nmft.iItem);
}

void CFolderView::OnContextMenu(NMHDR* nmtab, LRESULT* pRes)
{
	//NMFOLDERTAB& nmft = *(NMFOLDERTAB*)nmtab;
	OnFolderContextMenu();
}


void CFolderView::OnChangedFolder(int iPage)
{
	// you must override
}

void CFolderView::OnFolderContextMenu()
{
	// you must override
}

CFolderFrame* CFolderView::GetFolderFrame() const
{
	CFolderFrame* pFolderFrame = (CFolderFrame*)GetParent();
	ASSERT(pFolderFrame);
	ASSERT_KINDOF(CFolderFrame, pFolderFrame);
	return pFolderFrame;
}
