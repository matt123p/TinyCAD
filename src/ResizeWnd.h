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

#if !defined(AFX_RESIZEWND_H__6D1E2B77_7980_473D_8B38_DBF96BBEFB06__INCLUDED_)
#define AFX_RESIZEWND_H__6D1E2B77_7980_473D_8B38_DBF96BBEFB06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResizeWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CResizeWnd window

class CResizeWnd: public CWnd
{
	// Construction
public:
	CResizeWnd(BOOL vertical);

protected:
	HCURSOR m_cursor;
	BOOL m_tracking;
	BOOL m_vertical;
	BOOL m_mouseOver;
	// Attributes
public:
	int m_adjust_width;
	int m_adjust_height;

	// Operations
public:

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResizeWnd)
	//}}AFX_VIRTUAL

	// Implementation
public:
	virtual ~CResizeWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CResizeWnd)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMouseLeave();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESIZEWND_H__6D1E2B77_7980_473D_8B38_DBF96BBEFB06__INCLUDED_)
