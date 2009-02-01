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

// The dialogue to fetch a new symbol
class CDlgGetFindBox : public CInitDialogBar
{
protected:
	CLibraryStoreSymbol *m_Symbol;

	typedef std::deque<CLibraryStoreSymbol*> MRUCollection;
	MRUCollection m_most_recently_used;

	CSize m_sizeUndockedDefault;

	CResizeWnd	m_Resize;
	CResizeWnd	m_ResizeLib;

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
	CButton	m_Single_Lib_Sel;
	CListBox	m_Libraries;
	CButton	m_Show_Symbol;
	CListBox	m_List;
	CString	m_search_string;
	int		m_filter;
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
	afx_msg void OnListSelect();
	afx_msg void OnChangeSearchString();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnDblclkList();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeLibraries();
	afx_msg void OnHorzResize();
	afx_msg void OnSingleLibSel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void DrawLibraries( CDC &dc, LPDRAWITEMSTRUCT lpDrawItemStruct );
	void DrawSymbol( CDC &dc, CRect rect );
	void BuildLibraryList();
	void BuildSearchList();

public:
	void StoreLibraryList(void);
	void RestoreLibraryList(void);
	afx_msg void OnDestroy();
};


#endif
