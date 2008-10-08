/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "TinyCad.h"
#include "WinXPToolbar.h"
#include ".\winxptoolbar.h"


// CWinXPToolbar

IMPLEMENT_DYNAMIC(CWinXPToolbar, CToolBar)
CWinXPToolbar::CWinXPToolbar()
{
}

CWinXPToolbar::~CWinXPToolbar()
{
}


BEGIN_MESSAGE_MAP(CWinXPToolbar, CToolBar)
	ON_WM_NCPAINT()
END_MESSAGE_MAP()



// CWinXPToolbar message handlers


void CWinXPToolbar::OnNcPaint()
{
	EraseNonClient();
}

void CWinXPToolbar::EraseNonClient(void)
{
	//Get DC that is clipped to the non-client area
	CWindowDC dc(this);
	CRect rectClient;
	GetClientRect(rectClient);
	CRect rectWindow;
	GetWindowRect(rectWindow);
	ScreenToClient(rectWindow);
	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	dc.ExcludeClipRect(rectClient);

	//Draw borders in the nclient area
	rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);
	DrawBorders(&dc, rectWindow);

	//Erase parts that don't get drawn
	dc.IntersectClipRect(rectWindow);
	SendMessage(WM_ERASEBKGND, (WPARAM)dc.m_hDC);

	//Draw the gripper
	DrawGripper(&dc, rectWindow);
}

void CWinXPToolbar::DoPaint(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	//Paint the client area
	CRect rect;
	GetClientRect(rect);
	DrawBorders(pDC, rect);
	DrawGripper(pDC, rect);
}

void CWinXPToolbar::DrawGripper(CDC * pDC, const CRect& rect)
{
	pDC->FillSolidRect(&rect, ::GetSysColor(COLOR_BTNFACE));
	CToolBar::DrawGripper(pDC, rect);
}
