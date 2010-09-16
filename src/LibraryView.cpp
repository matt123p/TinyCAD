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
const	int CLibraryView::m_default_symbols_per_screen_page = 8;
int			CLibraryView::m_optimal_symbols_per_screen_page = 0;	//calculated dynamically

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
	else {
		view_transform.SetZoomFactor( 1.0 );
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
	//TRACE("CLibraryView::OnDraw(): Default monitor device height = %g transformed pixels.\n", height);

	int advance = 0;

	double max_height;
	double max_width;
	int block_height = 0;
	int min_range_y=0, max_range_y=0;
	double dPercentY, dMax_range_y;
	SIZE page_size, line_size;

	if (pDC->IsPrinting())
	{
		max_width = view_transform.doubleDeScale( pDC->GetDeviceCaps( HORZRES ) );
		max_height = (height - text_spacing*4) / m_symbols_per_print_page;
		block_height = static_cast<int>(max_height);

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
	else
	{	//Normal display on screen
		CRect r;
		GetClientRect( r );
		max_width = r.Width();
		max_height = GetSymbolBlockHeight();
		height = max_height * m_optimal_symbols_per_screen_page;
		block_height = static_cast<int>(max_height);
		//TRACE("CLibraryView::OnDraw(): New height = %g, max_height = %g, block_height = %ld.\n", height, max_height, block_height);

		// Establish which symbol to start drawing from based on the vertical scroll indicator
		GetScrollRange(SB_VERT, &min_range_y, &max_range_y);
		dMax_range_y = max_range_y-min_range_y+1;
		dPercentY = GetDeviceScrollPosition().y;
		dPercentY = dPercentY/dMax_range_y;
		advance = static_cast<int> (floor((dPercentY * pDoc->m_SymbolMap.size())+0.5));	//result is in units of number of symbols to scroll
		current_y = static_cast<int>(advance * block_height);
		start_y = current_y;

		//TRACE("CLibraryView::OnDraw(): Scroll Range Y min/max = %d/%d, Number of symbols (size) = %d, dPercentY = %g, advance = %d\n", min_range_y, max_range_y, pDoc->m_SymbolMap.size(), dPercentY, advance);
		//TRACE("CLibraryView::OnDraw(): GetScrollPosition().y = %ld, current_index = %d, start_y = %d.\n", GetScrollPosition().y, current_index, start_y);
	}

	current_index = advance;
	advance = 0;

	// No operator seems to exist to increment it advance times in one operation, so do it iteratively
	for (int i=0; i < current_index; i++ )
	{
		if (it != pDoc->m_SymbolMap.end()) ++it;
		else break;
	}

	//TRACE("CLibraryView::OnDraw(): After adjustment:  current_index = %d, advance = %d\n",current_index, advance);
	int text_pos_y;
	int text_pos_x;
	int text_col_width = 0;
	CDPoint p;
	CDPoint qq;

	for (;it != pDoc->m_SymbolMap.end(); ++it )
	{
		CContext q( pDC, view_transform );
		
		// Display the name of this symbol
		pDC->SelectObject( &theFont );
			
		// Now access the symbol and draw it next to the name
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

		//TRACE("Symbol orientation = %d, raw symbol origin = %g/%g\n",orientation, p.x, p.y);

		if (orientation >= 2)
		{	//swap x and y coordinates
			qq = p;
			p.x = qq.y;
			p.y = qq.x;
		}
		//TRACE("Symbol orientation = %d, rotated symbol origin = %g/%g\n",orientation, p.x, p.y);

		double zoom = min( (max_width*75) / fabs(p.x), (block_height*75)/fabs(p.y) );	//djl - shouldn't max_height really be block_height here?!!
		zoom = min( 100, zoom );
		zoom = max( 2, zoom );

		// Fill in the background rectangle
		//TRACE("current_index = %d:  Setting origin to %d/%d",current_index, 0, -current_y);
		q.SetOrigin( CDPoint( 0, -current_y) );	//This is the origin that will be used to draw the symbol's enclosing rectangle.

		if (pDC->IsPrinting())
		{	//printing the library contents
			q.SelectPen( cOFFPAGE,0,0 );
			q.MoveTo( CDPoint(0,0) );
			q.LineTo( CDPoint(max_width, 0) );
		}
		else
		{	//normal screen display
			if (current_index == pDoc->getSelected())
			{	//Draw selected items with a different background color than non-selected items
				//TRACE(" - current_index is selected, so brush = cOFFPAGE\n");
				q.SelectBrush( cOFFPAGE );	//Selected symbols are drawn with a background color the same as the color used for offpage items
			}
			else
			{	//Non-selected items use a white background
				//TRACE(" - current_index is not selected, so brush = cWHITE\n");
				q.SelectBrush( cWHITE );
			}
			q.SelectPen( cBLACK,0,0);
			q.Rectangle( CDRect(0,0,max_width, block_height ));	//This is the decorative rectangle that delineates each symbol in the library list
		}

		// Draw the name
		q.SetTextAlign(TA_LEFT | TA_BOTTOM | TA_NOUPDATECP);
		q.SetBkMode( TRANSPARENT );
		q.SetTextColor( RGB(0,0,0) );	//Symbol set names are drawn in black

		// Draw the name for each of the names in the symbol set
		text_pos_y = 20;
		text_pos_x = 5;
		text_col_width = 0;

		if (s.GetNumRecords() > 1) {
			//TRACE("CLibraryView::OnDraw(): ********* This symbol has %d variant names.   ****************\n",s.GetNumRecords());
		}

		for (int i =0; i < s.GetNumRecords(); i++)
		{
			CSymbolRecord &r = s.GetRecord( i );

			// Draw the name
			q.SetTextAlign(TA_LEFT | TA_BOTTOM | TA_NOUPDATECP);
			q.SetBkMode( TRANSPARENT );
			q.SetTextColor( RGB(0,0,0) );	//Symbol set names are drawn in black

			pDC->SelectObject( &theFont );
			q.TextOut( text_pos_x, text_pos_y, r.name.GetString() );
			//TRACE("Drawing at location (%d,%d) length = %d, name = [%s]\n", text_pos_x, text_pos_y, r.name.GetLength(), r.name.GetString());
			pDC->SelectObject( &theFont2 );
			q.TextOut( text_pos_x, text_pos_y + 16, r.description.GetString() );
			//TRACE("                     (%d,%d) length = %d, description = [%s]\n", text_pos_x, text_pos_y+16, r.name.GetLength(), r.description.GetString());
			text_pos_y += 35;

			text_col_width = max( text_col_width, static_cast<int>(view_transform.DeScale( pDC->GetTextExtent( r.description ).cx )) );
			text_col_width = max( text_col_width, static_cast<int>(view_transform.DeScale( pDC->GetTextExtent( r.name ).cx ) ) );

			if (text_pos_y > block_height - 40)
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
		double symbol_specific_zoom_factor = (zoom/100.0) *  q.GetTransform().GetZoomFactor();

		q.SetZoomFactor(symbol_specific_zoom_factor=( zoom/100.0 *  q.GetTransform().GetZoomFactor() ));
		int centering_height = static_cast<int>((block_height * 100) / zoom);
		int centering_width = static_cast<int>((width_to_fill * 100) / zoom);
		//TRACE("Symbol specific zoom factor = %g, local zoom factor = %g",symbol_specific_zoom_factor, zoom);

		CDPoint symbol_specific_origin = CDPoint( -(width_offset*100)/zoom, ((-current_y)*100)/zoom);
		q.SetOrigin( symbol_specific_origin);
		//TRACE(", Symbol specific origin = %g/%g\n",symbol_specific_origin.x, symbol_specific_origin.y);

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

		//TRACE("CLibraryView::OnDraw(): current_y now = %d.\n\n\n",current_y);
		int units_to_draw = current_y - start_y - 2*block_height;	//subtracting 2*block_height makes it paint two more symbols than otherwise necessary.  This is needed because the visible portion of the canvas may contain two extra fractional portions of a symbol, one at the top of the screen and one at the bottom of the screen.
		if (units_to_draw > height)
		{	//if we have drawn enough symbols to completely fill the visible portion of the screen, then quit drawing
			//TRACE("Symbol drawing loop complete.  current_y = %d, start_y = %d, block_height = %d, height = %g, formula result %d is greater than height.\n\n", current_y, start_y, block_height, height, units_to_draw);
			break;
		}
	}

	pDC->SelectObject( oldFont );

	//Make an intelligent selection of the amount to scroll when scrolling in any direction by a line or a page
	//Performing this logic inside the loop permits dynamic symbol additions or deletions to be taken into consideration
	page_size.cx = text_col_width;
	page_size.cy = static_cast<LONG>(height);	//size of the visible portion of the window for scrolling purposes
	if (page_size.cy > block_height) page_size.cy -= block_height;	//set to one symbol less than a full page of symbols

	line_size.cx = text_col_width;
	line_size.cy = 2*block_height;	//when vertical scroll bar is pressed, scroll up or down 2 symbols provide that this is less than one page of symbols
	if (line_size.cy >= page_size.cy) line_size.cy = block_height;	//If less than one symbol can be displayed at a time, then scroll by one whole symbol.  Alternative could be to scroll by the fraction of a symbol that is displayed.
	SetScrollSizes( MM_TEXT, GetDocSize(), page_size, line_size );
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
{	//returns the number of pixels required to display the entire document in the vertical dimension.  
	//the document consists of "blocks" of graphical symbols.  The height of a single block in pixels is based on
	//fitting a constant number of these blocks onto the current visible and usable window height.  Truncation will
	//possibly result in a fractional block being displayed.

	int block_height = GetSymbolBlockHeight();
	int total_size = block_height * GetDocument()->m_SymbolMap.size();

	//TRACE("CLibraryView::GetDocSize():  block_height = %d, total_size = %d\n", block_height, total_size);
	return CSize(0,total_size);
}

int CLibraryView::GetSymbolBlockHeight()
{	//returns the number of pixel units required to display a single symbol in the vertical dimension.

	CClientDC dc( this );
	Transform	view_transform;

	CRect r;
	GetClientRect( r );
//	double height = view_transform.doubleDeScale( dc.GetDeviceCaps( VERTRES ) );	//djl - the number of pixels in height MUST match the assumptions made by the library editing program regarding the height of the drawing area - this statement does NOT meet that requirement.
	double height = view_transform.doubleDeScale(r.Height());

	int block_height;
	m_optimal_symbols_per_screen_page = m_default_symbols_per_screen_page;

	if (dc.IsPrinting()) {	//printing the library contents
		block_height = static_cast<int>(height)/m_symbols_per_print_page;
	}
	else {	//normal screen display
		block_height = static_cast<int>(height)/m_optimal_symbols_per_screen_page;
		//When drawing symbols on the screen, try to achieve a more optimal number of symbols per screen than the fixed constant.
		//Assumption:  symbols larger than 125 pixel units are wasting space, while symbols < 50 pixel units are hard to read.
		//             When in-between these 2 limits, select the default number of symbols per page.
		//			   Note that this only applies to on-screen display, not to printer displays.
		if (block_height > 125) {
			m_optimal_symbols_per_screen_page = static_cast<int>(height) / 125;
			block_height = static_cast<int>(height)/m_optimal_symbols_per_screen_page;
		}
		else if (block_height < 50) {
			m_optimal_symbols_per_screen_page = static_cast<int>(height) / 50;
			if (m_optimal_symbols_per_screen_page < 1) {
				m_optimal_symbols_per_screen_page = 1;
				block_height = 50;
			}
			else {
				block_height = static_cast<int>(height)/m_optimal_symbols_per_screen_page;
			}
		}
	}
	//TRACE("Optimal symbols per page = %d, block_height = %d\n",m_optimal_symbols_per_screen_page, block_height);
	return block_height;
}


/////////////////////////////////////////////////////////////////////////////
// The user is selecting a new object

void CLibraryView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// Determine the current scroll limits
//	CClientDC dc( this );
//	Transform	view_transform;
//	double height = view_transform.doubleDeScale( dc.GetDeviceCaps( VERTRES ) );
//	double max_height = height / m_default_symbols_per_screen_page;
	double max_height = GetSymbolBlockHeight();
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



