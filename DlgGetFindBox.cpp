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


CDlgGetFindBox::CDlgGetFindBox()
: CInitDialogBar()
, m_Resize( TRUE )
, m_ResizeLib( FALSE )
{
	//{{AFX_DATA_INIT(CDlgGetFindBox)
	m_search_string = _T("");
	m_filter = 0;
	//}}AFX_DATA_INIT
	m_Symbol = NULL;
}

void CDlgGetFindBox::DoDataExchange(CDataExchange* pDX)
{
	CInitDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgGetFindBox)
	DDX_Control(pDX, IDC_SINGLE_LIB_SEL, m_Single_Lib_Sel);
	DDX_Control(pDX, IDC_LIBRARIES, m_Libraries);
	DDX_Control(pDX, IDC_SHOW_SYMBOL, m_Show_Symbol);
	DDX_Control(pDX, FINDGET_LIST, m_List);
	DDX_Text(pDX, IDC_SEARCH_STRING, m_search_string);
	DDX_Radio(pDX, IDC_RADIO1, m_filter);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgGetFindBox, CInitDialogBar)
	//{{AFX_MSG_MAP(CDlgGetFindBox)
	ON_LBN_SELCHANGE(FINDGET_LIST , OnListSelect )
	ON_EN_CHANGE(IDC_SEARCH_STRING, OnChangeSearchString)
	ON_WM_DRAWITEM()
	ON_LBN_DBLCLK(FINDGET_LIST, OnDblclkList)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_WM_SIZE()
	ON_LBN_SELCHANGE(IDC_LIBRARIES, OnSelchangeLibraries)
	ON_COMMAND(ID_HORZ_RESIZE, OnHorzResize)
	ON_BN_CLICKED(IDC_SINGLE_LIB_SEL, OnSingleLibSel)
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
END_MESSAGE_MAP()


BOOL CDlgGetFindBox::OnInitDialogBar()
{
	CInitDialogBar::OnInitDialogBar();

	BuildSearchList();
	BuildLibraryList();

	if (!m_Resize.m_hWnd)
	{
		m_Resize.Create( NULL, _T(""), WS_VISIBLE|WS_CHILD,CRect(0,0,10,10), this, ID_RESIZE );
	}
	if (!m_ResizeLib.m_hWnd)
	{
		m_ResizeLib.Create( NULL, _T(""), WS_VISIBLE|WS_CHILD,CRect(0,0,10,10), this, ID_HORZ_RESIZE );
	}
	m_sizeUndockedDefault = m_sizeDefault;

	// Set the checkbox for the libraries
	m_Single_Lib_Sel.SetCheck( CTinyCadRegistry::GetBool( _T("SelectOneLib"), FALSE ) ? 1 : 0 );


	// We remember the default size of the library box...
	CRect lib_list_rect;
	m_Libraries.GetWindowRect(lib_list_rect);
	ScreenToClient( lib_list_rect );
	int height = CTinyCadRegistry::GetInt("SymbolLibraryList", lib_list_rect.Height() );
	lib_list_rect.bottom = lib_list_rect.top + height;
	m_Libraries.MoveWindow( lib_list_rect);
	DetermineLayout();

	// Now recover the list of selected libraries
	RestoreLibraryList();

	return TRUE;
}


// End the dialog when an item is clicked on
void CDlgGetFindBox::OnListSelect()
{
	CListBox*	theListBox 	= (CListBox*) GetDlgItem( FINDGET_LIST );
	int			WhichItem 	= theListBox->GetCurSel();

 	if (WhichItem != LB_ERR)
	{
		m_Symbol = static_cast<CLibraryStoreSymbol *>(theListBox->GetItemDataPtr( WhichItem ));
		GetDlgItem( IDC_SHOW_SYMBOL )->RedrawWindow();
	}
}

void CDlgGetFindBox::OnChangeSearchString()
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CInitDialogBar::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	UpdateData( TRUE );
	BuildSearchList();
}

void CDlgGetFindBox::OnSelchangeLibraries() 
{
	// Do we enforce a single item check?
	if (m_Single_Lib_Sel.GetCheck() != 0)
	{
		int sel = m_Libraries.GetCurSel();
		for (int j = 0; j < m_Libraries.GetCount(); j ++)
		{
			if (m_Libraries.GetSel( j ) != 0 && sel != j)
			{
				m_Libraries.SetSel(j,FALSE);
			}
		}

	}

	BuildSearchList();
}


void CDlgGetFindBox::ResetAllSymbols()
{
	BuildLibraryList();
	BuildSearchList();
}


void CDlgGetFindBox::BuildSearchList()
{
	// Build the list box
	CListBox* theListBox = (CListBox*) GetDlgItem( FINDGET_LIST );
	theListBox->ResetContent();

	// Ensure the string is in Lower case for searching
	m_search_string.MakeLower();

	switch (m_filter)
	{
		case 0:
			CLibraryCollection::FillMatchingSymbols( theListBox, m_search_string, &m_Libraries );
			break;

		case 1:
			for( MRUCollection::iterator i = m_most_recently_used.begin(); i != m_most_recently_used.end(); i++ )
			{
				CLibraryStoreSymbol* pSymbol = *i;

				if( pSymbol && CLibraryCollection::ContainsSymbol(pSymbol->m_pParent) )
				{
					CString test = pSymbol->name + pSymbol->description;
					test.MakeLower();
					if (test.Find( m_search_string ) != -1)
					{
						int index = theListBox->AddString( pSymbol->name + " - " + pSymbol->description);
						theListBox->SetItemDataPtr( index, pSymbol );
					}
				}
			}
		break;
	}

	// Try and find the selected symbol in the list
	for (int i = 0; i < theListBox->GetCount(); i++)
	{
		if (theListBox->GetItemDataPtr( i ) == m_Symbol)
		{
			theListBox->SetCurSel( i );
			break;
		}
	}
	if (i == theListBox->GetCount())
	{
		m_Symbol = NULL;
		GetDlgItem( IDC_SHOW_SYMBOL )->RedrawWindow();
	}
}

void CDlgGetFindBox::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC dc;
	dc.Attach( lpDrawItemStruct->hDC );

	switch (nIDCtl)
	{
	case IDC_LIBRARIES:
		DrawLibraries( dc, lpDrawItemStruct);
		break;
	case IDC_SHOW_SYMBOL:
		DrawSymbol( dc, lpDrawItemStruct->rcItem  );
		break;
	}

	dc.Detach();

	// CInitDialogBar::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CDlgGetFindBox::DrawSymbol(CDC &dc, CRect rect)
{
	dc.SelectStockObject( WHITE_BRUSH );
	dc.Rectangle( rect );

	if (m_Symbol != NULL)
	{
		Transform transform;
		CContext q( &dc, transform );

		// Now access the symbol and draw it next to the name
		CDPoint p;
		CTinyCadDoc doc;
		CDesignFileSymbol *pSymbol = m_Symbol->GetDesignSymbol( &doc );

		// Determine the rotation
		int orientation = m_Symbol->m_pParent->orientation;


		drawingCollection method;
		if (pSymbol)
		{
			pSymbol->GetMethod( 0, false, method );
			p = pSymbol->GetTr( 0, false );
		}
		else
		{
			p = CDPoint(-75,-15);
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
		q.SetZoomFactor( zoom/100.0 );

		height = static_cast<int>((height * 100) / zoom);
		width = static_cast<int>((width * 100) / zoom);

		// Draw the name
		q.SetTextAlign(TA_LEFT | TA_BOTTOM | TA_NOUPDATECP);
		q.SetBkMode( TRANSPARENT );

		// Now display the symbol
		CDPoint old;

		switch (orientation)
		{
		case 2:
			old = q.SetTRM( CDPoint((width-p.x)/2, (height - p.y)/2) , CDPoint(0,0), orientation );
			break;
		case 3:
			old = q.SetTRM( CDPoint(width / 2 + p.x/2, (height-p.y)/2) , CDPoint(0,0), orientation );
			break;
		default:
			old = q.SetTRM( CDPoint((width-p.x)/2, (height-p.y)/2) , CDPoint(0,0), orientation );
			break;
		}
		if (pSymbol)
		{
			drawingIterator paint_it = method.begin();
			while (paint_it != method.end())
			{
				(*paint_it)->Paint( q, draw_normal );
				++ paint_it;
			}

			delete pSymbol;
		}

		q.EndTRM( old );
	}
}

void CDlgGetFindBox::DrawLibraries(CDC &dc, LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	CPen *old_pen;
	CBrush *old_brush;
	CPen select_pen;
	CBrush select_brush;
	COLORREF old_colour;
	HICON icon;

	if ((lpDrawItemStruct->itemState & ODS_SELECTED) != 0)
	{
		icon = AfxGetApp()->LoadIcon( IDI_SELECTED_BOX );
	}
	else
	{
		icon = AfxGetApp()->LoadIcon( IDI_UNSELECTED_BOX );
	}

	old_pen = (CPen *)dc.SelectStockObject( WHITE_PEN );
	old_brush = (CBrush *)dc.SelectStockObject( WHITE_BRUSH );
	old_colour = dc.SetTextColor( RGB(0,0,0) );

	dc.Rectangle( &lpDrawItemStruct->rcItem );

	CRect r = lpDrawItemStruct->rcItem;
	
	CLibraryStore *lib = CLibraryCollection::GetLibraryByIndex( lpDrawItemStruct->itemData );
	CString s;
	if (lib)
	{
		s = lib->m_name;
	}
	dc.SetBkMode( TRANSPARENT );
	DrawIconEx(dc.m_hDC, r.left + 1, r.top+1, icon, r.Height() - 5, r.Height() - 5, 0, NULL, DI_NORMAL );
	r.left += 13;

	int brk = s.ReverseFind('\\');
	if (brk != -1)
	{
		s = s.Mid(brk+1);
	}
	dc.DrawText( s , &r, DT_LEFT | DT_VCENTER );
	

	dc.SetTextColor( old_colour );
	dc.SelectObject( old_pen );
	dc.SelectObject( old_brush );
}



void CDlgGetFindBox::OnDblclkList()
{
	AfxGetMainWnd()->PostMessage(WM_COMMAND, IDM_TOOLGET );
}

void CDlgGetFindBox::OnRadio1()
{
	UpdateData( TRUE );
	BuildSearchList();
}

void CDlgGetFindBox::OnRadio2()
{
	UpdateData( TRUE );
	BuildSearchList();
}



void CDlgGetFindBox::AddToMRU()
{
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
		++ it;
	}

	m_most_recently_used.push_front( m_Symbol );

	// We keep only up to 20 symbols in this list
	if (m_most_recently_used.size() > 20)
	{
		m_most_recently_used.pop_back();
	}
}




void CDlgGetFindBox::OnSize(UINT nType, int cx, int cy)
{
	CInitDialogBar::OnSize(nType, cx, cy);
	DetermineLayout();
}


void CDlgGetFindBox::DetermineLayout()
{
	const int width = 6;
	CRect client;
	GetClientRect( client );
	int cx = client.Width();
	int cy = client.Height();

	// Resize the list to the correct width
	if (m_List.m_hWnd)
	{
		// Move the libraries list into position
		CRect lib_list_rect;
		m_Libraries.GetWindowRect(lib_list_rect);
		ScreenToClient( lib_list_rect );

		int border_y = lib_list_rect.left;
		int border_x = IsFloating() ? lib_list_rect.left : lib_list_rect.left + 4;
		lib_list_rect.right = cx - border_x;
		m_Libraries.MoveWindow( lib_list_rect);
		m_Libraries.RedrawWindow();

		CRect resize_rect(lib_list_rect);
		resize_rect.top = lib_list_rect.bottom + 4;
		resize_rect.bottom = lib_list_rect.bottom + width;
		m_ResizeLib.MoveWindow( resize_rect );

		
		// Move the symbol preview window into position
		CRect show_rect;
		m_Show_Symbol.GetWindowRect( show_rect );
		ScreenToClient( show_rect );
		int height = show_rect.Height();
		show_rect.top = resize_rect.bottom + width * 2;
		show_rect.bottom = show_rect.top + height;
		show_rect.right = cx - border_x;
		m_Show_Symbol.MoveWindow( show_rect );


		// Move the symbol list into position
		CRect list_rect;
		m_List.GetWindowRect(list_rect);
		ScreenToClient( list_rect );

		list_rect.top = show_rect.bottom + width * 2;
		list_rect.right = cx - border_x;
		list_rect.bottom = cy - border_y;
		m_List.MoveWindow( list_rect );


		if (IsFloating())
		{
			resize_rect.left = -5;
			resize_rect.right = -3;
			resize_rect.top = 0;
			resize_rect.bottom = 1;
		}
		else
		{
			resize_rect.left = cx-width;
			resize_rect.right = cx;
			resize_rect.top = border_y;
			resize_rect.bottom = cy-2;
		}
		m_Resize.MoveWindow( resize_rect );
	}
}


CSize CDlgGetFindBox::CalcDynamicLayout( int nLength, DWORD dwMode )
{
	if (m_Resize.m_adjust_width != 0)
	{
		m_sizeDefault.cx += m_Resize.m_adjust_width;
		m_Resize.m_adjust_width = 0;
	}

	if (   (dwMode & LM_STRETCH)!=0
		|| (dwMode & LM_MRUWIDTH)!=0
		|| (dwMode & LM_COMMIT)!=0
		|| (dwMode & LM_HORZDOCK)!=0
		|| (dwMode & LM_VERTDOCK)!=0)
	{
	}
	else
	{
		// Otherwise store this length
		if ((dwMode & LM_LENGTHY)!=0)
		{
			m_sizeUndockedDefault.cy = nLength;
		}
		else
		{
			m_sizeUndockedDefault.cx = nLength;
		}
		m_sizeDefault = m_sizeUndockedDefault;
	}

	return CalcFixedLayout(dwMode & LM_STRETCH, dwMode & LM_HORZDOCK);
}

CSize CDlgGetFindBox::CalcFixedLayout( BOOL bStretch, BOOL bHorz )
{
	if (!IsFloating())
	{
		CWnd* pWnd=GetParent();
		CRect r;
		pWnd->GetClientRect(r);
		m_sizeDefault.cy = r.Height();
	}


	return IsFloating() ? m_sizeUndockedDefault : m_sizeDefault;
}

void CDlgGetFindBox::StoreLibraryList(void)
{
	CString list_of_libraries;

	for (int i = 0;i < m_Libraries.GetCount(); ++i)
	{
		if (m_Libraries.GetSel( i ))
		{
			int index = m_Libraries.GetItemData(i);
			CLibraryStore *lib = CLibraryCollection::GetLibraryByIndex( index );
			if (lib)
			{
				if (!list_of_libraries.IsEmpty())
				{
					list_of_libraries += ",";
				}
				list_of_libraries += lib->m_name;
			}
		}
	}

	CTinyCadRegistry::Set("SymbolLibrarySelection", list_of_libraries );

}

void CDlgGetFindBox::RestoreLibraryList(void)
{
	CString list_of_libraries = CTinyCadRegistry::GetString("SymbolLibrarySelection", "" );
	std::set<CString>	selected_libs;

	// Split the list up into a a set
	while (!list_of_libraries.IsEmpty())
	{
		CString name;
		int brk = list_of_libraries.Find(_T(","));
		if (brk != -1)
		{
			name = list_of_libraries.Left( brk );
			list_of_libraries = list_of_libraries.Mid( brk + 1 );
		}
		else
		{
			name = list_of_libraries;
			list_of_libraries = "";
		}
		selected_libs.insert( name );
	}

	for (int i = 0;i < m_Libraries.GetCount(); ++i)
	{
		int index = m_Libraries.GetItemData(i);
		CLibraryStore *lib = CLibraryCollection::GetLibraryByIndex( index );
		if (lib)
		{
			if (selected_libs.find( lib->m_name ) != selected_libs.end() || selected_libs.size() == 0)
			{
				// Select this library
				m_Libraries.SetSel( i, TRUE );
			}
		}
	}

}


// Build the list of libraries for the library selection box
void CDlgGetFindBox::BuildLibraryList()
{
	BOOL r = m_Single_Lib_Sel.GetCheck() != 0;

	// Before reseting the content get a list of not-selected items...
	std::set<CString>	selected_libs;
	for (int i = 0;i < m_Libraries.GetCount(); ++i)
	{
		if (m_Libraries.GetSel( i ))
		{
			int index = m_Libraries.GetItemData(i);
			CLibraryStore *lib = CLibraryCollection::GetLibraryByIndex( index );
			if (lib)
			{
				selected_libs.insert( lib->m_name );
			}
		}
	}

	m_Libraries.ResetContent();
	CLibraryCollection::FillLibraryNames( &m_Libraries );	

	// Now select back all the libraries that were selected and any new ones...
	for (i = 0;i < m_Libraries.GetCount(); ++i)
	{
		int index = m_Libraries.GetItemData(i);
		CLibraryStore *lib = CLibraryCollection::GetLibraryByIndex( index );
		if (lib && selected_libs.find( lib->m_name) != selected_libs.end() )
		{
			m_Libraries.SetSel( i, TRUE );
			if (r)
			{
				break;
			}
		}
	}
}


void CDlgGetFindBox::OnHorzResize() 
{
	// Resize due to the library list size changing...
	int delta = m_ResizeLib.m_adjust_height;

	// Resize the library window...
	CRect r;
	m_Libraries.GetWindowRect( r );
	ScreenToClient( r );
	r.bottom += delta;
	m_Libraries.MoveWindow( r );

	CTinyCadRegistry::Set("SymbolLibraryList", r.Height() );


	DetermineLayout();
	RedrawWindow();
}


void CDlgGetFindBox::OnSingleLibSel() 
{
	BOOL r = m_Single_Lib_Sel.GetCheck() != 0;
	CTinyCadRegistry::Set( "SelectOneLib", r );

	if (r)
	{
		int sel = m_Libraries.GetCurSel();
		for (int j = 0; j < m_Libraries.GetCount(); j ++)
		{
			if (m_Libraries.GetSel( j ) != 0 && sel != j)
			{
				m_Libraries.SetSel(j,FALSE);
			}
		}

		if (sel == LB_ERR)
		{
			m_Libraries.SetSel(0,TRUE);
		}
	}

	BuildSearchList();
}



void CDlgGetFindBox::OnDestroy()
{
	StoreLibraryList();

	CInitDialogBar::OnDestroy();
}
