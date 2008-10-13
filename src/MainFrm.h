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

#if !defined(AFX_MAINFRM_H__1892E75F_763C_4067_80A1_B007DD788EA9__INCLUDED_)
#define AFX_MAINFRM_H__1892E75F_763C_4067_80A1_B007DD788EA9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "tinycaddoc.h"
#include "object.h"
#include "diag.h"
#include "DlgPositionBox.h"
#include "DlgGetFindBox.h"
#include "FolderFrame.h"
#include "WinXPToolbar.h"

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// control bar embedded members
	CStatusBar  m_wndStatusBar;
	CWinXPToolbar    m_wndToolBar;
	CWinXPToolbar    m_wndToolBarDraw;
	CWinXPToolbar    m_wndToolBarDrawBus;
	CWinXPToolbar    m_wndToolBarDrawGroup;
	CWinXPToolbar    m_wndToolBarAnnotate;
	CDlgPositionBox m_wndPositionBox;
	CDlgGetFindBox m_wndGetFindBox;

	CRect		m_oldRect;

public:
	void 					setPositionText( CString sPos );
	CLibraryStoreSymbol*	GetSelectSymbol();
	void ResetAllSymbols();
	virtual LRESULT OnDDEExecute(WPARAM wParam, LPARAM lParam);


// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnClose();
	afx_msg void OnViewAnnotationtoolbar();
	afx_msg void OnViewEdittoolbar();
	afx_msg void OnViewFiletoolbar();
	afx_msg void OnViewGrouptoolbar();
	afx_msg void OnViewBustoolbar();
	afx_msg void OnUpdateViewBustoolbar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewAnnotationtoolbar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewEdittoolbar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewFiletoolbar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewGrouptoolbar(CCmdUI* pCmdUI);
	afx_msg void OnViewshowpos();
	afx_msg void OnUpdateViewshowpos(CCmdUI* pCmdUI);
	afx_msg void OnResize();
	afx_msg void OnOptionsToolbarsSymbol();
	afx_msg void OnUpdateOptionsToolbarsSymbol(CCmdUI* pCmdUI);
	afx_msg void OnTimer(UINT);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__1892E75F_763C_4067_80A1_B007DD788EA9__INCLUDED_)

