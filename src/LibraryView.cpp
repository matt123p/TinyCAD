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
#include "tinycad.h"
#include "TinyCadView.h"
#include "diag.h"
#include "colour.h"
#include "option.h"
#include "LibraryView.h"
#include "LibraryDoc.h"
#include "revision.h"
#include "LibraryCollection.h"


const	int	CLibraryView::m_symbols_per_print_page = 10;
const	int CLibraryView::m_symbols_per_screen_page = 8;


/////////////////////////////////////////////////////////////////////////////
// CLibraryView

IMPLEMENT_DYNCREATE(CLibraryView, CScrollView)

CLibraryView::CLibraryView()
{
}

CLibraryView::~CLibraryView()
{
}


BEGIN_MESSAGE_MAP(CLibraryView, CScrollView)
	//{{AFX_MSG_MAP(CLibraryView)
	ON_COMMAND(ID_SYMBOL_DELETESYMBOL, OnSymbolDeletesymbol)
	ON_COMMAND(ID_SYMBOL_EDITSYMBOL, OnSymbolEditsymbol)
	ON_UPDATE_COMMAND_UI(ID_SYMBOL_DELETESYMBOL, OnUpdateSymbolDeletesymbol)
	ON_UPDATE_COMMAND_UI(ID_SYMBOL_EDITSYMBOL, OnUpdateSymbolEditsymbol)
	ON_COMMAND(ID_SYMBOL_NEWSYMBOL, OnSymbolNewsymbol)
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_CONTEXT_DUPLICATESYMBOL, OnContextDuplicatesymbol)
	ON_COMMAND(ID_SYMBOL_SYMBOLPROPERTIES, OnSymbolSymbolproperties)
	ON_UPDATE_COMMAND_UI(ID_SYMBOL_SYMBOLPROPERTIES, OnUpdateSymbolSymbolproperties)
	ON_UPDATE_COMMAND_UI(ID_CONTEXT_DUPLICATESYMBOL, OnUpdateContextDuplicatesymbol)
	//}}AFX_MSG_MAP

	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)

	// Range of commands
	ON_COMMAND_RANGE( IDM_LIBCOPY, IDM_LIBCOPY+100, OnSendTo )

END_MESSAGE_MAP()


// The OnCreate function, called when the window is created
int CLibraryView::OnCreate( LPCREATESTRUCT q )
{
	int r = CView::OnCreate( q );

	if (r == 0)
	{
		  ClipboardFormat = RegisterClipboardFormat(CLIPBOARD_SY_FORMAT);
	}

	return r;
}

BOOL CLibraryView::IsClipboardAvailable()
{
	return ::IsClipboardFormatAvailable(ClipboardFormat);
}


/////////////////////////////////////////////////////////////////////////////
// CLibraryView drawing

void CLibraryView::OnDraw(CDC* pDC)
{
	CLibraryDoc * pDoc = GetDocument();

	CLibraryDoc::idCollection::iterator it = pDoc->m_SymbolMap.begin();
	Transform	view_transform;

	if (pDC->IsPrinting())
	{
		view_transform.SetZoomFactor( 5.0 );
	}

	// The font for naming with
	CFont theFont;
	theFont.CreateFont(static_cast<int>(view_transform.doubleScale( 16 )),0,0,0,400,FALSE,FALSE,FALSE,
		ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS | CLIP_LH_ANGLES,
		DEFAULT_QUALITY,DEFAULT_PITCH | FF_MODERN,_T("Courier New"));

	CFont theFont2;
	theFont2.CreateFont(static_cast<int>(view_transform.doubleScale( 14 )),0,0,0,400,FALSE,FALSE,FALSE,
		ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS | CLIP_LH_ANGLES,
		DEFAULT_QUALITY,DEFAULT_PITCH | FF_MODERN,_T("Courier New"));

	CFont theFont3;
	theFont3.CreateFont(static_cast<int>(view_transform.doubleScale( 16 )),0,0,0,400,FALSE,FALSE,FALSE,
		ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS | CLIP_LH_ANGLES,
		DEFAULT_QUALITY,DEFAULT_PITCH |  FF_SWISS, _T("Arial"));

	CFont *oldFont = pDC->SelectObject( &theFont );

	TransformSnap s;
	s.SetGridSnap( FALSE );
	s.SetAccurateGrid(FineGrid);
	CTinyCadDoc doc;
	
	const int inter_spacing = 20;
	const int text_spacing = 25;
	int current_index = 0;
	int current_y = 0;
	int start_y = 0;
	
	double height = view_transform.doubleDeScale( pDC->GetDeviceCaps( VERTRES ) );
	int advance = 0;

	double max_height;
	double max_width;

	if (!pDC->IsPrinting())
	{
		CRect r;
		GetClientRect( r );
		max_width = r.Width();
		max_height = height / m_symbols_per_screen_page;

		// Establish where to start drawing from...
		advance = static_cast<int>(GetScrollPosition().y / max_height);
		current_y = static_cast<int>(advance * max_height);
		start_y = current_y;
	}
	else
	{
		max_width = view_transform.doubleDeScale( pDC->GetDeviceCaps( HORZRES ) );
		max_height = (height - text_spacing*4) / m_symbols_per_print_page;

		CString s = GetDocument()->m_pLibrary->m_name;
		CContext q( pDC, view_transform );
		pDC->SelectObject( &theFont3 );
		q.TextOut( 5, 0, s );
		s.Format(_T("Page %d of %d"), m_printing_page + 1, (GetDocument()->m_SymbolMap.size() / m_symbols_per_print_page) + 1 );
		q.TextOut( 5, height - text_spacing, s );

		// Establish where to start drawing from...
		current_y = text_spacing * 2;
		advance = (m_printing_page * m_symbols_per_print_page);
	}

	while ( advance > 0 && it != pDoc->m_SymbolMap.end() )
	{
		++ it;
		++ current_index;
		-- advance;
	}

	for (;it != pDoc->m_SymbolMap.end(); ++it )
	{
		CContext q( pDC, view_transform );
		
		// Display the name of this symbol
		pDC->SelectObject( &theFont );
			
		// Now access the symbol and draw it next to the name
		CDPoint p;
		CLibraryStoreNameSet &s = GetDocument()->getNameSet(it);
		CDesignFileSymbol *pSymbol = s.GetDesignSymbol( &doc, 0 );

		// Determine the rotation
		int orientation = s.orientation;


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

		if (orientation >= 2)
		{
			CDPoint q = p;
			p.x = q.y;
			p.y = q.x;
		}

		int block_height = 0;
		double zoom = min( (max_width*75) / fabs(p.x), (max_height*75)/fabs(p.y) );
		zoom = min( 100, zoom );
		zoom = max( 2, zoom );
		block_height = static_cast<int>(max_height);

		// Fill in the background rectangle
		q.SetOrigin( CDPoint( 0, -current_y) );
		if (!pDC->IsPrinting())
		{
			if (current_index == pDoc->getSelected())
			{
				q.SelectBrush( cOFFPAGE );
			}
			else
			{
				q.SelectBrush( cWHITE );
			}
			q.SelectPen( cBLACK,0,0);
			q.Rectangle( CDRect(0,0,max_width, block_height ));
		}
		else
		{
			q.SelectPen( cOFFPAGE,0,0 );
			q.MoveTo( CDPoint(0,0) );
			q.LineTo( CDPoint(max_width, 0) );
		}

		// Draw the name
		q.SetTextAlign(TA_LEFT | TA_BOTTOM | TA_NOUPDATECP);
		q.SetBkMode( TRANSPARENT );
		q.SetTextColor( RGB(0,0,0) );

		// Do this for each of the names in the symbol set
		int text_pos_y = 20;
		int text_pos_x = 5;
		int text_col_width = 0;
		for (int i =0; i < s.GetNumRecords(); i++)
		{
			CSymbolRecord &r = s.GetRecord( i );

			pDC->SelectObject( &theFont );
			q.TextOut( text_pos_x, text_pos_y, r.name );

			pDC->SelectObject( &theFont2 );
			q.TextOut( text_pos_x, text_pos_y + 16, r.description );
			text_pos_y += 35;

			text_col_width = max( text_col_width, static_cast<int>(view_transform.DeScale( pDC->GetTextExtent( r.description ).cx )) );
			text_col_width = max( text_col_width, static_cast<int>(view_transform.DeScale( pDC->GetTextExtent( r.name ).cx ) ) );

			if (text_pos_y > max_height - 40)
			{
				// Move across to the other side...
				text_pos_x += text_col_width + 30;
				text_col_width = 0;
				text_pos_y = 20;
			}
		}

		int width_offset = max( 250, text_col_width + text_pos_x + 30 );
		int width_to_fill = static_cast<int>(max_width) - width_offset;

		// Now display the symbol
		CDPoint old;

		q.SetZoomFactor( zoom/100.0 *  q.GetTransform().GetZoomFactor() );
		int centering_height = static_cast<int>((max_height * 100) / zoom);
		int centering_width = static_cast<int>((width_to_fill * 100) / zoom);
		q.SetOrigin( CDPoint( -(width_offset*100)/zoom, ((-current_y)*100)/zoom) );
		switch (orientation)
		{
		case 2:
			old = q.SetTRM( CDPoint((centering_width-p.x)/2, (centering_height - p.y)/2) , CDPoint(0,0), orientation );
			break;
		case 3:
			old = q.SetTRM( CDPoint(centering_width / 2 + p.x/2, (centering_height-p.y)/2) , CDPoint(0,0), orientation );
			break;
		default:
			old = q.SetTRM( CDPoint((centering_width-p.x)/2, (centering_height-p.y)/2) , CDPoint(0,0), orientation );
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

		current_index ++;
		current_y += block_height;

		if (current_y - start_y + block_height > height)
		{
			break;
		}
	}

	pDC->SelectObject( oldFont );

	SetScrollSizes( MM_TEXT, GetDocSize() );
}



/////////////////////////////////////////////////////////////////////////////
// CLibraryView diagnostics

#ifdef _DEBUG
void CLibraryView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CLibraryView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CLibraryDoc* CLibraryView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CLibraryDoc)));
	return (CLibraryDoc*)m_pDocument;
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLibraryView message handlers

void CLibraryView::OnInitialUpdate() 
{
	CScrollView::OnInitialUpdate();
	
	SetScrollSizes( MM_TEXT, GetDocSize() );
}

void CLibraryView::OnSymbolNewsymbol() 
{
	GetDocument()->EditSymbol( -1 );		
}

void CLibraryView::OnSymbolDeletesymbol() 
{
	if (AfxMessageBox( IDS_CONFIRM_DELETE, MB_YESNO ) == IDYES)
	{
		GetDocument()->DeleteSymbol( GetDocument()->getSelected() );
		Invalidate();
	}
}

void CLibraryView::OnSymbolEditsymbol() 
{
	GetDocument()->EditSymbol( GetDocument()->getSelected() );
	
}


void CLibraryView::OnUpdateSymbolDeletesymbol(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetDocument()->getSelected() != -1 );
}

void CLibraryView::OnUpdateSymbolEditsymbol(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetDocument()->getSelected() != -1 );
	
}




/////////////////////////////////////////////////////////////////////////////
// CLibraryView printing

BOOL CLibraryView::OnPreparePrinting(CPrintInfo* pInfo) 
{

	// default preparation
	return DoPreparePrinting(pInfo);
}

void CLibraryView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	// Count the symbols and determine the page count and splits
	CLibraryDoc * pDoc = GetDocument();
	CLibraryDoc::idCollection::iterator it = pDoc->m_SymbolMap.begin();


	TransformSnap s;
	s.SetGridSnap( FALSE );
	CTinyCadDoc doc;

	const int inter_spacing = 20;
	const int text_spacing = 25;
	double current_y = 0;

	int pages = (pDoc->m_SymbolMap.size() / m_symbols_per_print_page) + 1;

	pInfo->SetMinPage( 1 );
	pInfo->SetMaxPage( pages );
	
	CScrollView::OnBeginPrinting(pDC, pInfo);
}

void CLibraryView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	if (pInfo)
	{
		m_printing_page = pInfo->m_nCurPage - 1;
	}
	else
	{
		m_printing_page = 0;
	}


	CScrollView::OnPrepareDC(pDC, pInfo);
}

void CLibraryView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	// TODO: Add your specialized code here and/or call the base class	
	CScrollView::OnEndPrinting(pDC, pInfo);
}


CSize CLibraryView::GetDocSize()
{
	CClientDC dc( this );
	Transform	view_transform;
	double height = view_transform.doubleDeScale( dc.GetDeviceCaps( VERTRES ) );
	int total_size = static_cast<int>(GetDocument()->m_SymbolMap.size() * (height / m_symbols_per_screen_page));

	return CSize(0,total_size);
}

/////////////////////////////////////////////////////////////////////////////
// The user is selecting a new object

void CLibraryView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// Determine the current scroll limits
	CClientDC dc( this );
	Transform	view_transform;
	double height = view_transform.doubleDeScale( dc.GetDeviceCaps( VERTRES ) );
	double max_height = height / m_symbols_per_screen_page;

	int y_hit = GetScrollPosition().y + point.y;
	GetDocument()->setSelected( static_cast<int>(y_hit / max_height) );
	Invalidate();
	
	CScrollView::OnLButtonDown(nFlags, point);
}


void CLibraryView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	OnSymbolEditsymbol();
	
	CScrollView::OnLButtonDblClk(nFlags, point);
}


void CLibraryView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// First select the clicked on symbol...
	OnLButtonDown( nFlags, point );

	ClientToScreen( &point );

	// Now bring up the context menu
	CMenu menu;
	menu.LoadMenu( IDR_LIBVIEW );

	CMenu lib_menu;
	lib_menu.CreatePopupMenu();

	CLibraryCollection::FillLibraryNames( IDM_LIBCOPY, &lib_menu );
	menu.ModifyMenu( ID_CONTEXT_COPY_TO, MF_POPUP, (unsigned int)lib_menu.m_hMenu, _T("Copy to") );
	lib_menu.Detach();
	menu.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN|TPM_RIGHTBUTTON,
		point.x,point.y, AfxGetMainWnd(), NULL );
	
	CScrollView::OnRButtonDown(nFlags, point);
}

void CLibraryView::OnSendTo(UINT nCmd)
{
	// Did we find the library?
	CLibraryStore *lib = CLibraryCollection::GetLibraryByIndex( nCmd - IDM_LIBCOPY );

	if (lib)
	{
		// First can this library be upgraded?
		if (lib->MustUpgrade())
		{
			if (AfxMessageBox(IDS_MUSTUPGRADE, MB_YESNO ) != IDYES)
			{
				return;
			}

			lib = CLibraryCollection::Upgrade(lib);
			if (!lib)
			{
				return;
			}
		}


		// Now copy the symbol to the specified library...
		GetDocument()->CopySymbol( GetDocument()->getSelected(), lib );
	}
}


void CLibraryView::OnContextDuplicatesymbol() 
{
	// Now copy the symbol to the specified library...
	GetDocument()->DuplicateSymbol( GetDocument()->getSelected() );
}

void CLibraryView::OnUpdateContextDuplicatesymbol(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetDocument()->getSelected() != -1 );
}


void CLibraryView::OnSymbolSymbolproperties() 
{
	// Symbol properties
	GetDocument()->SymbolProperties( GetDocument()->getSelected() );

}

void CLibraryView::OnUpdateSymbolSymbolproperties(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetDocument()->getSelected() != -1 );	
}



