////////////////////////////////////////////////////////////////
// MSDN Magazine -- October 2002
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with Visual Studio 6.0 on Windows XP. Tab size=3.
//
#include "stdafx.h"
#include "resource.h"
#include "ftab.h"


//////////////////
// Private class to represent one folder tab
//
class CFolderTab {
private:
	CString	m_sText; // tab text
	CRect		m_rect;			// bounding rect
	CRgn		m_rgn;			// polygon region to fill (trapezoid)
	COLORREF	m_colour;		// the colour of the text

	int		ComputeRgn(CDC& dc, int x);
	int		Draw(CDC& dc, CFont& font, BOOL bSelected);
	BOOL		HitTest(CPoint pt)			{ return m_rgn.PtInRegion(pt); }
	CRect		GetRect() const				{ return m_rect; }
	void		GetTrapezoid(const CRect& rc, CPoint* pts) const;

	friend class CFolderTabCtrl;

public:
	CFolderTab(LPCTSTR lpszText) : m_sText(lpszText) 
	{
		m_colour = GetSysColor(COLOR_WINDOWTEXT);
	}
	LPCTSTR	GetText() const				{ return m_sText; }
	void  	SetText(LPCTSTR lpszText)	{ m_sText = lpszText; }
};

const int CXOFFSET = 8;		// defined pitch of trapezoid slant
const int CXMARGIN = 2;		// left/right text margin
const int CYMARGIN = 1;		// top/bottom text margin
const int CYBORDER = 1;		// top border thickness
const int CXBUTTON = GetSystemMetrics(SM_CXVSCROLL);

//////////////////
// Compute the the points, rect and region for a tab.
// Input x is starting x pos.
//
int CFolderTab::ComputeRgn(CDC& dc, int x)
{
	m_rgn.DeleteObject();

	CRect& rc = m_rect;
	rc.SetRectEmpty();

	// calculate desired text rectangle
	dc.DrawText(m_sText, &rc, DT_CALCRECT);
	rc.right  += 2*CXOFFSET + 2*CXMARGIN;						// add margins
	rc.bottom = rc.top + GetSystemMetrics(SM_CYHSCROLL);	// ht = scrollbar ht
	rc += CPoint(x,0);												// shift right

	// create trapezoid region
	CPoint pts[4];
	GetTrapezoid(rc, pts);
	m_rgn.CreatePolygonRgn(pts, 4, WINDING);

	return rc.Width();
}

//////////////////
// Given the boundint rect, compute trapezoid region.
// Note that the right and bottom edges not included in rect or
// trapezoid; these are normal rules of geometry. 
//
void CFolderTab::GetTrapezoid(const CRect& rc, CPoint* pts) const
{
	pts[0] = rc.TopLeft();
	pts[1] = CPoint(rc.left + CXOFFSET,   rc.bottom);
	pts[2] = CPoint(rc.right- CXOFFSET-1, rc.bottom);
	pts[3] = CPoint(rc.right-1, rc.top);
}


//////////////////
// Draw tab in normal or highlighted state
//
int CFolderTab::Draw(CDC& dc, CFont& font, BOOL bSelected)
{
	COLORREF bgColor = GetSysColor(bSelected ? COLOR_WINDOW     : COLOR_3DFACE);

	CBrush brush(bgColor);					 // background brush
	dc.SetBkColor(bgColor);					 // text background
	dc.SetTextColor(m_colour);				 // text color = fg color

	CPen blackPen(PS_SOLID, 1, RGB(0,0,0));	// black
	CPen shadowPen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));

	// Fill trapezoid
	CPoint pts[4];
	CRect rc = m_rect;
	GetTrapezoid(rc, pts);
	CPen* pOldPen = dc.SelectObject(&blackPen);
	dc.FillRgn(&m_rgn, &brush);

	// Draw edges. This is requires two corrections:
	// 1) Trapezoid dimensions don't include the right and bottom edges,
	// so must use one pixel less on bottom (cybottom)
	// 2) the endpoint of LineTo is not included when drawing the line, so
	// must add one pixel (cytop)
	//
	pts[1].y--;			// correction #1: true bottom edge y-coord
	pts[2].y--;			// ...ditto
	pts[3].y--;			// correction #2:	extend final LineTo
	dc.MoveTo(pts[0]);						// upper left
	dc.LineTo(pts[1]);						// bottom left
	dc.SelectObject(&shadowPen);			// bottom line is shadow color
	dc.MoveTo(pts[1]);						// line is inside trapezoid bottom
	dc.LineTo(pts[2]);						// ...
	dc.SelectObject(&blackPen);			// upstroke is black
	dc.LineTo(pts[3]);						// y-1 to include endpoint
	if (!bSelected) {
		// if not highlighted, upstroke has a 3D shadow, one pixel inside
		pts[2].x--;		// offset left one pixel
		pts[3].x--;		// ...ditto
		dc.SelectObject(&shadowPen);
		dc.MoveTo(pts[2]);
		dc.LineTo(pts[3]);
	}
	dc.SelectObject(pOldPen);

	// draw text
	rc.DeflateRect(CXOFFSET + CXMARGIN, CYMARGIN);
	CFont* pOldFont = dc.SelectObject(&font);
	dc.DrawText(m_sText, &rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
	dc.SelectObject(pOldFont);

	return m_rect.right;
}

//////////////////////////////////////////////////////////////////
// CFolderTabCtrl

IMPLEMENT_DYNAMIC(CFolderTabCtrl, CWnd)
BEGIN_MESSAGE_MAP(CFolderTabCtrl, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_BN_CLICKED(FTBPREV,OnPrevTab)
	ON_BN_CLICKED(FTBNEXT,OnNextTab)
END_MESSAGE_MAP()

CFolderTabCtrl::CFolderTabCtrl()
{
	m_iCurItem =
	m_dwFtabStyle =
	m_cxDesired =
	m_cxButtons =
	m_iFirstTab = 0;
}

CFolderTabCtrl::~CFolderTabCtrl()
{
	while (!m_lsTabs.IsEmpty())
		delete (CFolderTab*)m_lsTabs.RemoveHead();
}

//////////////////
// Create folder tab control from static control.
// Destroys the static control. This is convenient for dialogs
//
BOOL CFolderTabCtrl::CreateFromStatic(UINT nID, CWnd* pParent)
{
	CStatic wndStatic;
	if (!wndStatic.SubclassDlgItem(nID, pParent))
		return FALSE;
	CRect rc;
	wndStatic.GetWindowRect(&rc);
	pParent->ScreenToClient(&rc);
	wndStatic.DestroyWindow();
	rc.bottom = rc.top + GetDesiredHeight();
	return Create(WS_CHILD|WS_VISIBLE, rc, pParent, nID);
}

//////////////////
// Create folder tab control.
//
BOOL CFolderTabCtrl::Create(DWORD dwStyle, const RECT& rc,
	CWnd* pParent, UINT nID, DWORD dwFtabStyle)
{
	ASSERT(pParent);
	ASSERT(dwStyle & WS_CHILD);

	m_dwFtabStyle = dwFtabStyle;

	static LPCTSTR lpClassName = _T("PDFolderTab");
	static BOOL bRegistered = FALSE; // registered?
	if (!bRegistered) {
		WNDCLASS wc;
		memset(&wc, 0, sizeof(wc));
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
		wc.lpfnWndProc = (WNDPROC)::DefWindowProc; // will get hooked by MFC
		wc.hInstance = AfxGetInstanceHandle();
      wc.hCursor = LoadCursor(NULL, IDC_ARROW);
      wc.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
      wc.lpszMenuName = NULL;
      wc.lpszClassName = lpClassName;
		if (!AfxRegisterClass(&wc)) {
			TRACE("*** CFolderTabCtrl::AfxRegisterClass failed!\n");
			return FALSE;
		}
		bRegistered = TRUE;
	}
	if (!CWnd::CreateEx(0, lpClassName, NULL, dwStyle, rc, pParent, nID))
		return FALSE;

	// initialize fonts
	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	lf.lfHeight = GetSystemMetrics(SM_CYHSCROLL)-CYMARGIN;
	lf.lfWeight = FW_NORMAL;
	lf.lfCharSet = DEFAULT_CHARSET;
	_tcscpy_s(lf.lfFaceName, _T("Arial"));
	m_fontNormal.CreateFontIndirect(&lf);
	lf.lfHeight -= 2;
	m_fontSelected.CreateFontIndirect(&lf);

	return TRUE;
}

//////////////////
// Folder tab was created: create scroll buttons if style says so.
//
int CFolderTabCtrl::OnCreate(LPCREATESTRUCT lpcs)
{
	if (CWnd::OnCreate(lpcs)!=0)
		return -1;

	if (m_dwFtabStyle & FTS_BUTTONS) {
		CRect rc;
		for (int id=FTBPREV; id<=FTBNEXT; id++) {
			VERIFY(m_wndButton[id-1].Create(WS_VISIBLE|WS_CHILD, this, rc, id));
		}
		m_cxButtons = 2*CXBUTTON;
	}
	return 0;
}

//////////////////
// Get folder tab text
//
LPCTSTR CFolderTabCtrl::GetItemText(int iItem)
{
	CFolderTab* pft = GetTab(iItem);
	return pft ? pft->GetText() : NULL;
}

//////////////////
// Set folder tab text
//
void CFolderTabCtrl::SetItemText(int iItem, LPCTSTR lpText)
{
	CFolderTab* pft = GetTab(iItem);
	if (pft) {
		pft->SetText(lpText);
	}
}

//////////////////
// copy a font
//
static void CopyFont(CFont& dst, CFont& src)
{
	dst.DeleteObject();
	LOGFONT lf;
	VERIFY(src.GetLogFont(&lf));
	dst.CreateFontIndirect(&lf);
}

//////////////////
// Set normal, selected fonts
//
void CFolderTabCtrl::SetFonts(CFont& fontNormal, CFont& fontSelected)
{
	CopyFont(m_fontNormal, fontNormal);
	CopyFont(m_fontSelected, fontSelected);
}

//////////////////
// Paint function
//
void CFolderTabCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	int xOrigin = m_cxButtons - GetTab(m_iFirstTab)->GetRect().left;
	dc.SetViewportOrg(xOrigin,0);

	CRect rc;
	GetClientRect(&rc);

	CFolderTab* pCurTab = NULL;

	// draw all the normal (non-selected) tabs
	int n = GetItemCount();
	for (int i=0; i<n; i++) {
		CFolderTab* pTab = GetTab(i);
		ASSERT(pTab);
		if (i==m_iCurItem) {
			pCurTab = pTab;
		} else {
			pTab->Draw(dc, m_fontNormal, FALSE);
		}
	}
	// draw selected tab last so it will be "on top" of the others
	if (pCurTab)
		pCurTab->Draw(dc, m_fontSelected, TRUE);

	// draw border: line along the top edge, excluding seleted tab
	CRect rcCurTab(0,0,0,0);
	if (pCurTab)
		rcCurTab = pCurTab->GetRect();

	rc.right -= xOrigin;
	CPen blackPen(PS_SOLID, 1, RGB(0,0,0));	// black
	CPen* pOldPen = dc.SelectObject(&blackPen);
	dc.MoveTo(rcCurTab.right, rcCurTab.top);
	dc.LineTo(rc.right, rc.top);
	if (m_dwFtabStyle & FTS_FULLBORDER) {
		dc.MoveTo(rc.right-1, rc.top);
		dc.LineTo(rc.right-1, rc.bottom-1);
		dc.LineTo(rc.left,  rc.bottom-1);
		dc.LineTo(rc.left,  rc.top);
	} else {
		dc.MoveTo(rc.left, rc.top);
	}
	dc.LineTo(rcCurTab.TopLeft());
	dc.SelectObject(pOldPen);
}

//////////////////
// Handle mouse click: select new tab, if any. Notify parent, of course
//
void CFolderTabCtrl::OnLButtonDown(UINT nFlags, CPoint pt)
{
	int iTab = HitTest(pt);
	if (iTab>=0 && iTab!=m_iCurItem) {
		SelectItem(iTab);
		NMFOLDERTAB nm;
		nm.hwndFrom = m_hWnd;
		nm.idFrom = GetDlgCtrlID();
		nm.code = FTN_TABCHANGED;
		nm.iItem = iTab;
		CWnd* pParent = GetParent();
		pParent->SendMessage(WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
	}
}

void CFolderTabCtrl::OnRButtonDown(UINT nFlags, CPoint pt)
{
	// Select the correct sheet
	int iTab = HitTest(pt);
	if (iTab>=0 && iTab!=m_iCurItem) {
		SelectItem(iTab);
		NMFOLDERTAB nm;
		nm.hwndFrom = m_hWnd;
		nm.idFrom = GetDlgCtrlID();
		nm.code = FTN_TABCHANGED;
		nm.iItem = iTab;
		CWnd* pParent = GetParent();
		pParent->SendMessage(WM_NOTIFY, nm.idFrom, (LPARAM)&nm);

	}

	{
		// Now send the context menu message
		NMFOLDERTAB nm;
		nm.hwndFrom = m_hWnd;
		nm.idFrom = GetDlgCtrlID();
		nm.code = FTN_CONTEXTMENU;
		nm.iItem = iTab;
		CWnd* pParent = GetParent();
		pParent->SendMessage(WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
	}
		
}


//////////////////
// Find which tab is under mouse, -1 if none
//
int CFolderTabCtrl::HitTest(CPoint pt)
{
	CRect rc;
	GetClientRect(&rc);
	rc.left += m_cxButtons;
	if (rc.PtInRect(pt)) {
		int xOrigin = m_cxButtons - GetTab(m_iFirstTab)->GetRect().left;
		pt.x -= xOrigin;
		int n = GetItemCount();
		for (int i=0; i<n; i++) {
			if (GetTab(i)->HitTest(pt))
				return i;
		}
	}
	return -1;
}

//////////////////
// Select ith tab. Returns index selected
//
int CFolderTabCtrl::SelectItem(int iTab)
{
	int count = GetItemCount();
	if (iTab<0 || iTab>=count)
		return -1;		// bad
	if (iTab == m_iCurItem)
		return iTab;	// already selected

	if (m_iCurItem < count)
		InvalidateTab(m_iCurItem);		// invalidate old tab (repaint)

	m_iCurItem = iTab;				// set new selected tab
	InvalidateTab(m_iCurItem);		// repaint new tab
	
	return m_iCurItem;
}

//////////////////
/// Invalidate a tab: invaldate its rect
//
void CFolderTabCtrl::InvalidateTab(int iTab, BOOL bErase)
{
	CRect rc = GetTab(iTab)->GetRect();
	int xOrigin = m_cxButtons - GetTab(m_iFirstTab)->GetRect().left;
	rc += CPoint(xOrigin,0);
	InvalidateRect(rc, bErase);
}

//////////////////
// Load folder tab control from resource string:
// newline-separated list of tab names.
//
BOOL CFolderTabCtrl::Load(UINT nIDRes)
{
	CString s;
	if (!s.LoadString(nIDRes))
		return FALSE;

	CString sTab;
	for (int i=0; AfxExtractSubString(sTab, s, i); i++) {
		AddItem(sTab);
	}
	return TRUE;
}

//////////////////
// Add a tab.
//
int CFolderTabCtrl::AddItem(LPCTSTR lpszText)
{
	m_lsTabs.AddTail(new CFolderTab(lpszText));
	RecomputeLayout();
	return m_lsTabs.GetCount() - 1;
}

//////////////////
// Remove tab at given index.
//
BOOL CFolderTabCtrl::RemoveItem(int iPos)
{
	POSITION pos = m_lsTabs.FindIndex(iPos);
	if (pos) {
		CFolderTab* pTab = (CFolderTab*)m_lsTabs.GetAt(pos);
		m_lsTabs.RemoveAt(pos);
		delete pTab;
	}
	if (m_lsTabs.GetCount() > 0)
	{
		RecomputeLayout();
	}
	return pos!=NULL;
}

CFolderTab* CFolderTabCtrl::GetTab(int iPos)
{
	POSITION pos = m_lsTabs.FindIndex(iPos);
	return pos ? static_cast<CFolderTab*>(m_lsTabs.GetAt(pos)) : NULL;
}

//////////////////
// Recalculate layout based on having added or removed a tab.
//
void CFolderTabCtrl::RecomputeLayout()
{
	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&m_fontNormal);
	int x = 0;
	int n = GetItemCount();
	CFolderTab* pTab=NULL;
	for (int i=0; i<n; i++) {
		pTab = GetTab(i);
		if (pTab) 
			x += pTab->ComputeRgn(dc, x) - CXOFFSET;
	}
	dc.SelectObject(pOldFont);

	m_cxDesired = m_cxButtons;
	if (pTab) {
		CRect rc = pTab->GetRect();
		m_cxDesired += rc.right;
	}
}

//////////////////
// Folder tabs changed size: reposition scroll buttons.
//
void CFolderTabCtrl::OnSize(UINT nType, int cx, int cy)
{
	if (m_wndButton[0].m_hWnd) {
		int w = cy;
		CRect rc(0,0,w,cy);
		for (int i=FTBPREV; i<=FTBNEXT; i++) {
			m_wndButton[i-1].MoveWindow(&rc);
			rc += CPoint(w,0);
		}
		UpdateButtons();
	}
}

//////////////////
// Determine enabled state of scroll buttons
//
void CFolderTabCtrl::UpdateButtons()
{
	if (m_wndButton[0].m_hWnd && !m_lsTabs.IsEmpty()) {
		// enable prev button iff more tabs to left.
		m_wndButton[0].EnableWindow(m_iFirstTab>0);

		// enable next button iff more tabs to right
		CRect rc;
		GetClientRect(&rc);
		int xOrigin = m_cxButtons - GetTab(m_iFirstTab)->GetRect().left;
		CRect rcLast = ((CFolderTab*)m_lsTabs.GetTail())->GetRect();
		m_wndButton[1].EnableWindow(xOrigin + rcLast.right>rc.right);
	}
}

//////////////////
// User clicked next button: increment starting tab and repaint
//
void CFolderTabCtrl::OnNextTab()
{
	if (m_iFirstTab < m_lsTabs.GetCount()-1) {
		m_iFirstTab++;
		Invalidate();
		UpdateButtons();
	}
}

//////////////////
// User clicked prev button: decrement starting tab and repaint
//
void CFolderTabCtrl::OnPrevTab()
{
	if (m_iFirstTab > 0) {
		m_iFirstTab--;
		Invalidate();
		UpdateButtons();
	}
}

////////////////////////////////////////////////////////////////
// CFolderButton
//
IMPLEMENT_DYNAMIC(CFolderButton, CButton)
BEGIN_MESSAGE_MAP(CFolderButton, CButton)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
END_MESSAGE_MAP()

//////////////////
// Draw sroll button: draw a black triangle.
//
void CFolderButton::DrawItem(LPDRAWITEMSTRUCT lpDis)
{
	DRAWITEMSTRUCT& dis = *lpDis;
	CDC& dc = *CDC::FromHandle(dis.hDC);
	CRect rc;
	GetClientRect(&rc);

	// fill background with 3D face color
	dc.FillSolidRect(&rc,GetSysColor(COLOR_3DFACE));

	// shift southeast if button is pressed (bDown)
	BOOL bDown = dis.itemState & ODS_SELECTED;
	if (bDown) {
		rc += CPoint(1,1);
	}

	// draw line above to match folder tabs
	CPen pen(PS_SOLID,1,
		dis.itemState & ODS_DISABLED ? GetSysColor(COLOR_3DSHADOW) : RGB(0,0,0));
	CPen* pOldPen = dc.SelectObject(&pen);
	dc.MoveTo(rc.TopLeft());
	dc.LineTo(rc.right,rc.top);

	// Draw 3D highlight rect for 3D look
	CRect rc2=rc;
	for (int i=0; i<2; i++) {
		dc.Draw3dRect(&rc2,
			GetSysColor(bDown ? COLOR_3DFACE : COLOR_3DHIGHLIGHT),
			GetSysColor(COLOR_3DSHADOW));
		rc2.right--;
		rc2.bottom--;
	}

	// Draw triangle pointing the right way. Use shadow color if disabled.
	CSize szArrow = CSize(4,7);
	int cyMargin = (rc.Height()-szArrow.cy)/2;
	int cxMargin = (rc.Width()-szArrow.cx)/2;
	int x, incr;
	if (dis.CtlID==FTBNEXT) {
		x = rc.left + cxMargin;
		incr = 1;
	} else {
		x = rc.right - cxMargin - 1;
		incr = -1;
	}
	int y = rc.top + cyMargin;
	int h = 7;
	for (int j=0; j<4; j++) {
		dc.MoveTo(x,y);
		dc.LineTo(x,y+h);
		h-=2;
		x += incr;
		y++;
	}
	dc.SelectObject(pOldPen);
}

//////////////////
// User clicked button.
//
void CFolderButton::OnLButtonDown(UINT nFlags, CPoint pt)
{
	Default();									 // will send WM_COMMAND to parent
	SetTimer(1,500,NULL);					 // set timer for continual scroll
	m_nTimerClick = 0;						 // counter for initial delay
}

//////////////////
// User let go the mouse.
//
void CFolderButton::OnLButtonUp(UINT nFlags, CPoint pt)
{
	KillTimer(1);								 // no more repeat
	Default();
}

//////////////////
// Double-click: treat as another click.
//
void CFolderButton::OnLButtonDblClk(UINT nFlags, CPoint pt)
{
	SendMessage(WM_LBUTTONDOWN,nFlags,MAKELONG(pt.x,pt.y));
}

//////////////////
// Timer click: send another WM_COMMMAND, as if button clicked.
//
void CFolderButton::OnTimer(UINT nIDEvent)
{
	if (IsWindowEnabled()) {
		if (m_nTimerClick++ == 0) {
			KillTimer(1);
			SetTimer(1,150,NULL);
		}
		GetParent()->SendMessage(WM_COMMAND, GetDlgCtrlID());
	} else {
		KillTimer(1);
	}
}

