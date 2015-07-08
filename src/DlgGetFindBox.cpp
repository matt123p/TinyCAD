/*
 TinyCAD program for schematic capture
 Copyright 1994/1995/2002,2003 Matt Pyne.

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

#include "stdafx.h"
#include <math.h>

#include "option.h"
#include "revision.h"
#include "registry.h"
#include "TinyCadView.h"
#include "TinyCad.h"
#include "TinyCadRegistry.h"
#include "MainFrm.h"
#include "DlgGetFindBox.h"
#include "LibraryCollection.h"
#include ".\dlggetfindbox.h"

CDlgGetFindBox::CDlgGetFindBox() :
	CInitDialogBar(), /*m_Resize(TRUE),*/ m_ResizeLib(FALSE)
{
	//{{AFX_DATA_INIT(CDlgGetFindBox)
	m_search_string = _T("");
	//}}AFX_DATA_INIT
	m_Symbol = NULL;

	SetMinSize(CSize(220, 420));
}

void CDlgGetFindBox::DoDataExchange(CDataExchange* pDX)
{
	CInitDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgGetFindBox)
	DDX_Control(pDX, IDC_SHOW_SYMBOL, m_Show_Symbol);
	DDX_Control(pDX, IDC_SYMBOL_TREE, m_Tree);
	DDX_Text(pDX, IDC_SEARCH_STRING, m_search_string);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgGetFindBox, CInitDialogBar)
	//{{AFX_MSG_MAP(CDlgGetFindBox)
	ON_EN_CHANGE(IDC_SEARCH_STRING, 
	OnChangeSearchString)ON_WM_DRAWITEM()
	ON_WM_SIZE()
	ON_COMMAND(ID_HORZ_RESIZE, OnHorzResize)
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_NOTIFY(TVN_SELCHANGED, IDC_SYMBOL_TREE, &CDlgGetFindBox::OnTreeSelect)
	ON_NOTIFY(NM_DBLCLK, IDC_SYMBOL_TREE, &CDlgGetFindBox::OnDblclkTree)
	ON_WM_SHOWWINDOW()
	//ON_MESSAGE(WM_INITDIALOG, &CDlgGetFindBox::HandleInitDialog)
END_MESSAGE_MAP()

BOOL CDlgGetFindBox::OnInitDialog()
{
	CInitDialogBar::OnInitDialog();

	BuildTree();

	m_Tree.SetItemHeight(m_Tree.GetItemHeight() - 2); // original height looks weird on Windows

	if (!m_ResizeLib.m_hWnd)
	{
		m_ResizeLib.Create(NULL, _T(""), WS_VISIBLE | WS_CHILD, CRect(0, 0, 10, 10), this, ID_HORZ_RESIZE);
	}
	m_sizeUndockedDefault = m_sizeDefault;

	// We remember the default size of the library box...
	CRect lib_list_rect;
	m_Tree.GetWindowRect(lib_list_rect);
	ScreenToClient(lib_list_rect);
	int height = CTinyCadRegistry::GetInt("SymbolTreeHeightPx", lib_list_rect.Height());
	lib_list_rect.bottom = lib_list_rect.top + height;
	m_Tree.MoveWindow(lib_list_rect);
	DetermineLayout();

	return TRUE;
}

void CDlgGetFindBox::OnDblclkTree(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	HTREEITEM hItem = m_Tree.GetSelectedItem();
	if (!m_Tree.ItemHasChildren(hItem)) AfxGetMainWnd()->PostMessage(WM_COMMAND, IDM_TOOLGET);
	*pResult = 0;
}

void CDlgGetFindBox::OnTreeSelect(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	//LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hItem = m_Tree.GetSelectedItem();
	if (!m_Tree.ItemHasChildren(hItem))
	{
		CLibraryStoreSymbol* pSymbol = reinterpret_cast<CLibraryStoreSymbol*> (m_Tree.GetItemData(hItem));
		m_Symbol = pSymbol;
		GetDlgItem(IDC_SHOW_SYMBOL)->RedrawWindow();
	}

	*pResult = 0;
}

void CDlgGetFindBox::OnChangeSearchString()
{
	UpdateData(TRUE);
	BuildTree();
}

void CDlgGetFindBox::ResetAllSymbols()
{
	m_Symbol = NULL;
	BuildTree();
}

/// For iterating through all items in a tree, depth-first. Use TVI_ROOT for the first item.
HTREEITEM GetNextTreeItem(const CTreeCtrl & Tree, HTREEITEM cur)
{
	HTREEITEM res;
	res = Tree.GetChildItem(cur);
	if (res != NULL) return res;
	res = Tree.GetNextSiblingItem(cur);
	if (res != NULL) return res;

	HTREEITEM hParent = Tree.GetParentItem(cur);
	if (hParent == NULL) return NULL; // no more items in the tree
	res = Tree.GetNextSiblingItem(hParent);
	return res;
}

void CDlgGetFindBox::BuildTree()
{
	m_Tree.DeleteAllItems();

	// http://www.functionx.com/visualc/treeview/tvdlg1.htm
	// use TVIS_EXPANDED or Expand(), perhaps TVIS_BOLD

	// Add/filter MRU symbols
	{
		// Cleanup MRU
		size_t nMRUCount = m_most_recently_used.size();
		for (size_t i = 0; i < nMRUCount; i++)
		{
			CLibraryStoreSymbol* pSymbol = m_most_recently_used[0];
			m_most_recently_used.pop_front();
			// Is symbol still alive?
			if (pSymbol && pSymbol->m_pParent && CLibraryCollection::ContainsSymbol(pSymbol->m_pParent))
			{
				// Place it back in MRU list
				m_most_recently_used.push_back(pSymbol);
			}
		}

		CString sCaption;
		HTREEITEM hLib = m_Tree.InsertItem(_T("(recent) (x)"), TVI_ROOT);
		int nMatches = 0;
		for (unsigned i = 0; i < m_most_recently_used.size(); i++)
		{
			CLibraryStoreSymbol* pSymbol = m_most_recently_used[i];
			TRACE("pSymbol->Description=[%S], pSymbol->Name=[%S]\n", pSymbol->description, pSymbol->name);

			if (pSymbol->IsMatching(m_search_string))
			{
				HTREEITEM hItem = m_Tree.InsertItem(pSymbol->name + " - " + pSymbol->description, hLib);
				m_Tree.SetItemData(hItem, (DWORD_PTR) pSymbol);
				nMatches++;
			}
		}

		if (nMatches != 0)
		{
			sCaption.Format(_T("(recent) (%d)"), nMatches);
			m_Tree.SetItemText(hLib, sCaption);
		}
		else
		{
			m_Tree.DeleteItem(hLib);
		}
	}

	// Fill libraries
	CLibraryCollection::FillMatchingSymbols(&m_Tree, m_search_string);

	CRect r;
	m_Tree.GetClientRect(&r);
	UINT nMaxVisbile = r.Height() / m_Tree.GetItemHeight();
	if (m_Tree.GetCount() < nMaxVisbile)
	{
		// expand matching libraries
		HTREEITEM cur = m_Tree.GetRootItem();
		for (; cur != NULL; cur = m_Tree.GetNextSiblingItem(cur))
		{
			m_Tree.Expand(cur, TVE_EXPAND);
		}
	}

	// Try and find the selected symbol in the list
	HTREEITEM cur = m_Tree.GetRootItem();
	bool found = false;
	while (cur != NULL)
	{
		if (!m_Tree.ItemHasChildren(cur))
		{
			DWORD_PTR n = m_Tree.GetItemData(cur);
			CLibraryStoreSymbol * p = reinterpret_cast<CLibraryStoreSymbol*> (n);
			if (p == m_Symbol)
			{
				m_Tree.SelectItem(cur);
				m_Tree.EnsureVisible(cur);
				found = true;
				break;
			}
		}
		cur = GetNextTreeItem(m_Tree, cur);
	}
	if (found == false)
	{
		m_Symbol = NULL;
		GetDlgItem(IDC_SHOW_SYMBOL)->RedrawWindow();
	}
}
void CDlgGetFindBox::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);

	switch (nIDCtl)
	{
		case IDC_SHOW_SYMBOL:
			DrawSymbol(dc, lpDrawItemStruct->rcItem);
			break;
	}

	dc.Detach();

	// CInitDialogBar::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CDlgGetFindBox::DrawSymbol(CDC &dc, CRect rect)
{
	dc.SelectStockObject(WHITE_BRUSH);
	dc.Rectangle(rect);

	if (m_Symbol != NULL)
	{
		Transform transform;
		CContext q(&dc, transform);

		// Now access the symbol and draw it next to the name
		CDPoint p;
		CTinyCadDoc doc;
		CDesignFileSymbol *pSymbol = m_Symbol->GetDesignSymbol(&doc);

		// Determine the rotation
		int orientation = m_Symbol->m_pParent->orientation;

		drawingCollection method;
		if (pSymbol)
		{
			pSymbol->GetMethod(0, false, method);
			p = pSymbol->GetTr(0, false);
		}
		else
		{
			p = CDPoint(-75, -15);
		}

		switch (orientation)
		{
			case 2:
			{
				CDPoint q = p;
				p.x = q.y;
				p.y = q.x;
			}
				break;
			case 3:
			{
				CDPoint q = p;
				p.x = q.y;
				p.y = q.x;
			}
		}
		int height = (rect.bottom - rect.top);
		int width = (rect.right - rect.left);

		// Determine the zoom factor
		double zoom = min( (width * 75) / fabs(p.x), (height*75)/fabs(p.y) );
		zoom = min( zoom, 100 );
		q.SetZoomFactor(zoom / 100.0);

		height = static_cast<int> ( (height * 100) / zoom);
		width = static_cast<int> ( (width * 100) / zoom);

		// Draw the name
		q.SetTextAlign(TA_LEFT | TA_BOTTOM | TA_NOUPDATECP);
		q.SetBkMode(TRANSPARENT);

		// Now display the symbol
		CDPoint old;

		switch (orientation)
		{
			case 2:
				old = q.SetTRM(CDPoint( (width - p.x) / 2, (height - p.y) / 2), CDPoint(0, 0), orientation);
				break;
			case 3:
				old = q.SetTRM(CDPoint(width / 2 + p.x / 2, (height - p.y) / 2), CDPoint(0, 0), orientation);
				break;
			default:
				old = q.SetTRM(CDPoint( (width - p.x) / 2, (height - p.y) / 2), CDPoint(0, 0), orientation);
				break;
		}
		if (pSymbol)
		{
			drawingIterator paint_it = method.begin();
			while (paint_it != method.end())
			{
				(*paint_it)->Paint(q, draw_normal);
				++paint_it;
			}

			delete pSymbol;
		}

		q.EndTRM(old);
	}
}

void CDlgGetFindBox::AddToMRU()
{
	if (m_Symbol == NULL) return;
	// Is this symbol already in the MRU list?
	MRUCollection::iterator it = m_most_recently_used.begin();
	while (it != m_most_recently_used.end())
	{
		CLibraryStoreSymbol* symbol = *it;
		if (symbol == m_Symbol)
		{
			// Already in the MRU list
			return;
		}
		++it;
	}

	m_most_recently_used.push_front(m_Symbol);

	// We keep only up to 20 symbols in this list
	if (m_most_recently_used.size() > 20)
	{
		m_most_recently_used.pop_back();
	}
}

void CDlgGetFindBox::OnSlide(BOOL bSlideDirection)
{
	m_nSlideDelta += m_nSlideDelta / 3; // Speedup the slide
	CPaneDialog::OnSlide(bSlideDirection);
}

void CDlgGetFindBox::OnSize(UINT nType, int cx, int cy)
{
	CInitDialogBar::OnSize(nType, cx, cy);
	DetermineLayout();
}

void CDlgGetFindBox::DetermineLayout()
{
	CRect client;
	GetClientRect(client);
	int cx = client.Width();
	int cy = client.Height();

	// Resize the list to the correct width
	if (m_Tree.m_hWnd)
	{
		// Move the libraries list into position
		CRect lib_list_rect;
		m_Tree.GetWindowRect(lib_list_rect);
		ScreenToClient(lib_list_rect);
		int border_y = lib_list_rect.left;
		int border_x = lib_list_rect.left;

		//CRect tree_rect(lib_list_rect);
		CRect tree_rect;
		m_Tree.GetWindowRect(tree_rect);
		ScreenToClient(tree_rect);
		if (tree_rect.Height() < 12)
			tree_rect.bottom = tree_rect.top + 12;
		if (tree_rect.bottom > cy - 12)
			tree_rect.bottom = cy - 12;

		tree_rect.right = cx - border_x;
		m_Tree.MoveWindow(tree_rect);

		CRect resize_rect(tree_rect);
		resize_rect.top = tree_rect.bottom + 0;
		resize_rect.bottom = resize_rect.top + 6;
		m_ResizeLib.MoveWindow(resize_rect);

		// Move the symbol preview window into position
		CRect show_rect;
		m_Show_Symbol.GetWindowRect(show_rect);
		ScreenToClient(show_rect);
		//int height = show_rect.Height();
		show_rect.top = resize_rect.bottom + 0;
		show_rect.bottom = cy - border_y;
		show_rect.right = cx - border_x;
		m_Show_Symbol.MoveWindow(show_rect);
	}
}

void CDlgGetFindBox::OnHorzResize()
{
	// Resize due to the vertical resize handle dragging...
	int delta = m_ResizeLib.m_adjust_height;

	// Resize the tree window...
	CRect r;
	m_Tree.GetWindowRect(r);
	ScreenToClient(r);
	r.bottom += delta;
	m_Tree.MoveWindow(r);

	CTinyCadRegistry::Set("SymbolTreeHeightPx", r.Height());

	m_Show_Symbol.GetWindowRect(r);
	ScreenToClient(r);
	r.top += delta;
	m_Show_Symbol.MoveWindow(r);

	DetermineLayout();
	// less flicker: RedrawWindow();
}

void CDlgGetFindBox::OnDestroy()
{
	CInitDialogBar::OnDestroy();
}

void CDlgGetFindBox::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CInitDialogBar::OnShowWindow(bShow, nStatus);

	if (bShow)
	{
		GetDlgItem(IDC_SEARCH_STRING)->SetFocus();
	}
}
