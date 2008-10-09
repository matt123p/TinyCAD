////////////////////////////////////////////////////////////////
// MSDN Magazine -- October 2002
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with Visual Studio 6.0 on Windows XP. Tab size=3.
//
#include "StdAfx.h"
#include "WinMgr.h"


// standard sizing cursors
static HCURSOR hcSizeEW = ::LoadCursor(NULL,(LPCTSTR)IDC_SIZEWE);
static HCURSOR hcSizeNS = ::LoadCursor(NULL,(LPCTSTR)IDC_SIZENS);

IMPLEMENT_DYNCREATE(CSizerBar, CWnd)
BEGIN_MESSAGE_MAP(CSizerBar, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_CHAR()
END_MESSAGE_MAP()

CSizerBar::CSizerBar()
{
	m_pWinMgr = NULL;
	m_bHorz = -1;			// undetermined; I will compute
	m_bDragging=FALSE;	// not dragging yet
}

CSizerBar::~CSizerBar()
{
}

BOOL CSizerBar::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= SS_NOTIFY; // because static doesn't usually send mouse events
	return CStatic::PreCreateWindow(cs);
}

BOOL CSizerBar::Create(DWORD dwStyle, CWnd* pParentWnd, CWinMgr& wm,
	UINT nID, const RECT& rc)
{
	m_pWinMgr = &wm;
	return CStatic::Create(NULL, dwStyle, rc, pParentWnd, nID);
}

void CSizerBar::OnPaint()
{
	CPaintDC dc(this);
	CRect rc;
	GetClientRect(&rc);
	dc.FillSolidRect(&rc,GetSysColor(COLOR_3DFACE));
	dc.Draw3dRect(&rc,GetSysColor(COLOR_3DLIGHT),
		GetSysColor(COLOR_3DDKSHADOW));
	rc.DeflateRect(1,1);
	dc.Draw3dRect(&rc,GetSysColor(COLOR_3DHIGHLIGHT),
		GetSysColor(COLOR_3DSHADOW));
}

//////////////////
// Determine whether I am horizontal or vertical by looking at dimensions.
// Remember the result for speed.
//
BOOL CSizerBar::IsHorizontal()
{
	if (!m_hWnd)
		return FALSE;	 // not created yet: doesn't matter
	if (m_bHorz!=-1)
		return m_bHorz; // I already figured it out
	
	// If width is greater than height, I must be horizontal. Duh.
	CRect rc;
	GetWindowRect(&rc);
	return m_bHorz = (rc.Width() > rc.Height());
}

//////////////////
// Set cursor to indicate sizing is possible
//
BOOL CSizerBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT msg)
{
	::SetCursor(IsHorizontal() ? hcSizeNS : hcSizeEW);
	return TRUE;
}

//////////////////
// User pressed mouse: intialize and enter drag state
//
void CSizerBar::OnLButtonDown(UINT nFlags, CPoint pt)
{
	ClientToScreen(&pt);
	m_ptPrevious = pt;
	
	SetCapture();				 // all mouse messages are MINE
	m_hwndPrevFocus = ::SetFocus(m_hWnd);  // set focus to me to get Escape key

	ASSERT(m_pWinMgr);
	CWinMgr& wm = *m_pWinMgr;

	// get WINRECTs on either side of me
	WINRECT* pwrcSizeBar = wm.FindRect(GetDlgCtrlID());
	ASSERT(pwrcSizeBar);
	WINRECT* prev = pwrcSizeBar->Prev();
	ASSERT(prev);
	WINRECT* next = pwrcSizeBar->Next();
	ASSERT(next);

	// get rectangles on either side of me
	CRect rcPrev = prev->GetRect();
	CRect rcNext = next->GetRect();

	// get parent window
	CWnd * pParentWnd = GetParent();
	ASSERT(pParentWnd);

	// Get size info for next/prev rectangles, so I know what the min/max
	// sizes are and don't violate them. Max size never tested.
	SIZEINFO szi;
	wm.OnGetSizeInfo(szi, prev, pParentWnd);
	CRect rcPrevMin(rcPrev.TopLeft(),szi.szMin);
	CRect rcPrevMax(rcPrev.TopLeft(),szi.szMax);

	wm.OnGetSizeInfo(szi, next, pParentWnd);
	CRect rcNextMin(rcNext.BottomRight()-szi.szMin, rcNext.BottomRight());
	CRect rcNextMax(rcNext.BottomRight()-szi.szMax, rcNext.BottomRight());

	// Initialize rcClip. This is the box the user is allowed to move
	// the sizer bar in. Can't go outside of this--would violate min/max
	// constraints of windows on either side.
	CRect rcClip;
	rcClip.SetRect(
		max(rcPrevMin.right, rcNextMax.left),
		max(rcPrevMin.bottom,rcNextMax.top),
		min(rcPrevMax.right, rcNextMin.left),
		min(rcPrevMax.bottom,rcNextMin.top));
	pParentWnd->ClientToScreen(&rcClip); // to screen coords

	// Now adjust rcClip for the fact the bar is not a pure line, but
	// has solid width -- so I have to make a little bigger/smaller according
	// to the offset of mouse coords within the sizer bar iteself.
	CRect rcBar;
	GetWindowRect(&rcBar);	 // bar location in screen coords
	if (IsHorizontal()) {
		rcClip.left = rcBar.left;
		rcClip.top += pt.y-rcBar.top;
		rcClip.right = rcBar.right;
		rcClip.bottom -= rcBar.top-pt.y;
	} else {
		rcClip.left += pt.x-rcBar.left;
		rcClip.top = rcBar.top;
		rcClip.right -= rcBar.right-pt.x;
		rcClip.bottom = rcBar.bottom;
	}
	if (rcClip.right < rcClip.left)
		rcClip.left = rcClip.right = pt.x;
	if (rcClip.bottom < rcClip.top)
		rcClip.bottom = rcClip.top = pt.y;

	ClipCursor(&rcClip); // clip it!
	m_bDragging=TRUE;
}

//////////////////
// User moved mouse: erase old bar and draw in new position. XOR makes this
// easy. Keep track of previous point.
//
void CSizerBar::OnMouseMove(UINT nFlags, CPoint pt)
{
	if (m_bDragging) {
		ClientToScreen(&pt);
		if (pt!=m_ptPrevious) {
			CPoint ptDelta = pt-m_ptPrevious;
			NotifyMoved(ptDelta);				 // notify parent
			m_ptPrevious = pt;
		}
	}
}

//////////////////
// User let go of mouse: leave size-drag mode
//
void CSizerBar::OnLButtonUp(UINT nFlags, CPoint pt)
{
	if (m_bDragging) {
		ClientToScreen(&pt);
		CPoint ptDelta = pt-m_ptPrevious; // distance moved
		CancelDrag();							 // cancel drag mode
		NotifyMoved(ptDelta);				 // notify parent
	}
}

//////////////////
// Cancel drag mode: release capture, erase bar, restore focus
//
void CSizerBar::CancelDrag()
{
	ReleaseCapture();						// release mouse
	ClipCursor(NULL);						// free to roam now
	::SetFocus(m_hwndPrevFocus);		// restore original focus window
	m_bDragging = FALSE;					// stop dragging
}

//////////////////
// Escape key cancels sizing.
//
void CSizerBar::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar==VK_ESCAPE && m_bDragging) {
		CancelDrag();
		return;
	}
	CStatic::OnChar(nChar, nRepCnt, nFlags);
}

//////////////////
// Handle WM_CANCELMODE. This is probably unnecessary.
//
void CSizerBar::OnCancelMode()
{
	if (m_bDragging)
		CancelDrag();
}

//////////////////
// Notify parent I moved. It's up to the parent to actually move me by
// calling CWinMgr. I just report how much to move by.
//
void CSizerBar::NotifyMoved(CPoint ptDelta)
{
	NMWINMGR nmr;
	nmr.code = NMWINMGR::SIZEBAR_MOVED;		 // notification subcode
	nmr.idFrom = GetDlgCtrlID();				 // my ID
	nmr.sizebar.ptMoved = ptDelta;			 // distance moved
	GetParent()->SendMessage(WM_WINMGR, nmr.idFrom, (LPARAM)&nmr);
}
