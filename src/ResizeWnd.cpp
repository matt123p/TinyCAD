/*
 TinyCAD program for schematic capture
 Copyright 1994/1995/2002-2005 Matt Pyne.

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

// ResizeWnd.cpp : implementation file
//

#include "stdafx.h"
#include "tinycad.h"
#include "ResizeWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CResizeWnd

CResizeWnd::CResizeWnd(BOOL vertical)
{

	m_adjust_width = 0;
	m_vertical = vertical;

	if (m_vertical)
	{
		m_cursor = AfxGetApp()->LoadCursor(AFX_IDC_TRACKWE);
	}
	else
	{
		m_cursor = AfxGetApp()->LoadCursor(AFX_IDC_TRACKNS);
	}
}

CResizeWnd::~CResizeWnd()
{

}

BEGIN_MESSAGE_MAP(CResizeWnd, CWnd)
	//{{AFX_MSG_MAP(CResizeWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResizeWnd message handlers

void CResizeWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect r;
	GetClientRect(r);

	if (m_vertical)
	{
		r.left = point.x - 1;
		r.right = point.x + 1;
		r.top = -2048;
		r.bottom = 2048;
	}
	else
	{
		r.top = point.y - 1;
		r.bottom = point.y + 1;
		r.left = -2048;
		r.right = 2048;
	}

	CRectTracker tracker(r, CRectTracker::hatchInside);

	if (m_vertical)
	{
		tracker.Track(this, point, FALSE, AfxGetMainWnd());
		m_adjust_width = tracker.m_rect.left;
		m_adjust_height = 0;
		AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_RESIZE);
	}
	else
	{
		tracker.Track(this, point, FALSE, GetParent());
		m_adjust_width = 0;
		m_adjust_height = tracker.m_rect.top - point.y;
		GetParent()->PostMessage(WM_COMMAND, ID_HORZ_RESIZE);
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CResizeWnd::OnLButtonUp(UINT nFlags, CPoint point)
{

	CWnd::OnLButtonUp(nFlags, point);
}

void CResizeWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CWnd::OnMouseMove(nFlags, point);
}

void CResizeWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect r;
	GetClientRect(r);
	CBrush brush(::GetSysColor(COLOR_3DFACE));
	CBrush *pbrush = dc.SelectObject(&brush);
	dc.Rectangle(r);

	dc.Draw3dRect(r, GetSysColor(COLOR_BTNHIGHLIGHT), GetSysColor(COLOR_BTNSHADOW));

	dc.SelectObject(pbrush);

}

BOOL CResizeWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// trackers should only be in client area
	if (nHitTest == HTCLIENT)
	{
		::SetCursor(m_cursor);
		return TRUE;
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}
