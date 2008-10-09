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

#if !defined(AFX_LIBRARYVIEW_H__F6B8803F_2828_4C70_B5F5_F8767DF2AA70__INCLUDED_)
#define AFX_LIBRARYVIEW_H__F6B8803F_2828_4C70_B5F5_F8767DF2AA70__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LibraryView.h : header file
//

#include "LibraryDoc.h"


/////////////////////////////////////////////////////////////////////////////
// CLibraryView view

class CLibraryView : public CScrollView
{
protected:
	CLibraryView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CLibraryView)

	static const int	m_symbols_per_print_page;
	static const int	m_symbols_per_screen_page;

// Attributes
public:
	CLibraryDoc* GetDocument();


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLibraryView)
	public:
	virtual void OnInitialUpdate();
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
	UINT		ClipboardFormat;
	BOOL		IsClipboardAvailable();

	int			m_printing_page;


	CSize GetDocSize();

	virtual ~CLibraryView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CLibraryView)
	afx_msg void OnSymbolDeletesymbol();
	afx_msg void OnSymbolEditsymbol();
	afx_msg void OnUpdateSymbolDeletesymbol(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSymbolEditsymbol(CCmdUI* pCmdUI);
	afx_msg void OnSymbolNewsymbol();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate( LPCREATESTRUCT q );
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnContextDuplicatesymbol();
	afx_msg void OnSymbolSymbolproperties();
	afx_msg void OnUpdateSymbolSymbolproperties(CCmdUI* pCmdUI);
	afx_msg void OnUpdateContextDuplicatesymbol(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnSendTo(UINT nCmd);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in LibraryView.cpp
inline CLibraryDoc* CLibraryView::GetDocument()
   { return (CLibraryDoc*)m_pDocument; }
#endif


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIBRARYVIEW_H__F6B8803F_2828_4C70_B5F5_F8767DF2AA70__INCLUDED_)
