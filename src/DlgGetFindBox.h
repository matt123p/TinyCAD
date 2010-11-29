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



#ifndef __DLGGETFINDBOX__
#define __DLGGETFINDBOX__

#include "TinyCadDoc.h"
#include "InitDialogBar.h"
#include "ResizeWnd.h"

/// The dialog to instantiate a new symbol from libraries
class CDlgGetFindBox : public CInitDialogBar
{
protected:
	CLibraryStoreSymbol *m_Symbol;  ///< The currently selected symbol (to be eventually instantiated)

	typedef std::deque<CLibraryStoreSymbol*> MRUCollection;
	MRUCollection m_most_recently_used;

	CSize m_sizeUndockedDefault;

	CResizeWnd	m_Resize;  ///< for resizing *this dialog - an stripe on the right edge
	CResizeWnd	m_ResizeLib;  ///< for resizing space between library list and symbol preview area

public:
 	CDlgGetFindBox();

	CLibraryStoreSymbol* GetSelectSymbol() { return m_Symbol; }

	virtual CSize CalcDynamicLayout( int nLength, DWORD dwMode );
	virtual CSize CalcFixedLayout( BOOL bStretch, BOOL bHorz );

	void ResetAllSymbols();
	void AddToMRU();



// Dialog Data
	//{{AFX_DATA(CDlgGetFindBox)
	enum { IDD = IDD_GETFIND };
	CButton	m_Show_Symbol;
    CTreeCtrl   m_Tree;
	CString	m_search_string;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgGetFindBox)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:
	void DetermineLayout();
	virtual BOOL OnInitDialogBar();


	//{{AFX_MSG(CDlgGetFindBox)
	afx_msg void OnChangeSearchString();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHorzResize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void DrawSymbol( CDC &dc, CRect rect );
	void BuildSearchList();
    void BuildTree();

public:
	afx_msg void OnDestroy();
    afx_msg void OnTreeSelect(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDblclkTree(NMHDR *pNMHDR, LRESULT *pResult);
};


#endif
