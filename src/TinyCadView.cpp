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
#include "TinyCad.h"
#include "SpecialPrintDlg.h"
#include "TinyCadDoc.h"
#include "TinyCadView.h"
#include "OptionsPropertySheet.h"
#include "DlgExportPng.h"
#include "DlgColours.h"
#include "MainFrm.h"
#include "TinyCadRegistry.h"
#include "diag.h"
#include "EditToolbar.h"
#include "DlgPositionBox.h"
#include ".\tinycadview.h"


extern CDlgERCListBox theERCListBox;



/////////////////////////////////////////////////////////////////////////////
// CTinyCadView

IMPLEMENT_DYNCREATE(CTinyCadView, CFolderView)

BEGIN_MESSAGE_MAP(CTinyCadView, CFolderView)
	//{{AFX_MSG_MAP(CTinyCadView)
	ON_UPDATE_COMMAND_UI(IDM_EDITEDIT, OnUpdateEditedit)
	ON_UPDATE_COMMAND_UI(IDM_BUSBACK, OnUpdateBusback)
	ON_UPDATE_COMMAND_UI(IDM_BUSSLASH, OnUpdateBusslash)
	ON_UPDATE_COMMAND_UI(IDM_TOOLARC, OnUpdateToolarc)
	ON_UPDATE_COMMAND_UI(IDM_TOOLBUS, OnUpdateToolbus)
	ON_UPDATE_COMMAND_UI(IDM_TOOLBUSNAME, OnUpdateToolbusname)
	ON_UPDATE_COMMAND_UI(IDM_TOOLCIRCLE, OnUpdateToolcircle)
	ON_UPDATE_COMMAND_UI(IDM_TOOLCONNECT, OnUpdateToolconnect)
	ON_UPDATE_COMMAND_UI(IDM_TOOLGET, OnUpdateToolget)
	ON_UPDATE_COMMAND_UI(IDM_TOOLJUNC, OnUpdateTooljunc)
	ON_UPDATE_COMMAND_UI(IDM_TOOLLABEL, OnUpdateToollabel)
	ON_UPDATE_COMMAND_UI(IDM_TOOLHIERACHICAL, OnUpdateToolHierachical)
	ON_UPDATE_COMMAND_UI(IDM_TOOLPOLYGON, OnUpdateToolpolygon)
	ON_UPDATE_COMMAND_UI(IDM_TOOLPOWER, OnUpdateToolpower)
	ON_UPDATE_COMMAND_UI(IDM_TOOLSQUARE, OnUpdateToolsquare)
	ON_UPDATE_COMMAND_UI(IDM_TOOLTEXT, OnUpdateTooltext)
	ON_UPDATE_COMMAND_UI(IDM_TOOLWIRE, OnUpdateToolwire)
	ON_UPDATE_COMMAND_UI(IDM_VIEWCENTRE, OnUpdateViewcentre)
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_UPDATE_COMMAND_UI(IDM_EDITDRAG, OnUpdateEditdrag)
	ON_UPDATE_COMMAND_UI(IDM_EDITDUP, OnUpdateEditdup)
	ON_UPDATE_COMMAND_UI(IDM_EDITROTATE, OnUpdateEditrotate)
	ON_UPDATE_COMMAND_UI(IDM_EDITMOVE, OnUpdateEditmove)
	ON_COMMAND(IDM_SNAPTOGRID, OnSnaptogrid)
	ON_UPDATE_COMMAND_UI(IDM_SNAPTOGRID, OnUpdateSnaptogrid)
	ON_UPDATE_COMMAND_UI(IDM_REPEATNAMEDOWN, OnUpdateRepeatnamedown)
	ON_UPDATE_COMMAND_UI(IDM_REPEATNAMEUP, OnUpdateRepeatnameup)
	ON_UPDATE_COMMAND_UI(IDM_REPEATPINDOWN, OnUpdateRepeatpindown)
	ON_UPDATE_COMMAND_UI(IDM_REPEATPINUP, OnUpdateRepeatpinup)
	ON_UPDATE_COMMAND_UI(IDM_EDITPASTE, OnUpdateEditpaste)
	ON_UPDATE_COMMAND_UI(IDM_EDITCUT, OnUpdateEditcut)
	ON_UPDATE_COMMAND_UI(IDM_EDITCOPY, OnUpdateEditcopy)
	ON_UPDATE_COMMAND_UI(IDM_EDITROTATELEFT, OnUpdateEditRotateLeft)
	ON_UPDATE_COMMAND_UI(IDM_EDITROTATERIGHT, OnUpdateEditRotateRight)
	ON_UPDATE_COMMAND_UI(IDM_EDITFLIP, OnUpdateEditFlip)
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_RULER_VERT, OnRulerVert)
	ON_UPDATE_COMMAND_UI(ID_RULER_VERT, OnUpdateRulerVert)
	ON_COMMAND(ID_RULER_HORIZ, OnRulerHoriz)
	ON_UPDATE_COMMAND_UI(ID_RULER_HORIZ, OnUpdateRulerHoriz)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(IDM_VIEW_OPTIONS, OnViewOptions)
	ON_COMMAND(ID_CONTEXT_ARCIN, OnContextArcin)
	ON_COMMAND(ID_CONTEXT_ARCOUT, OnContextArcout)
	ON_COMMAND(ID_CONTEXT_CANCELDRAWING, OnContextCanceldrawing)
	ON_COMMAND(ID_CONTEXT_FINISHDRAWING, OnContextFinishdrawing)
	ON_COMMAND(ID_CONTEXT_FREELINE, OnContextFreeline)
	ON_COMMAND(ID_CONTEXT_ADDHANDLE, OnContextAddhandle)
	ON_COMMAND(ID_CONTEXT_DELETEHANDLE, OnContextDeletehandle)
	ON_COMMAND(ID_CONTEXT_ZORDER_BRINGTOFRONT, OnContextZorderBringtofront)
	ON_COMMAND(ID_CONTEXT_ZORDER_SENDTOBACK, OnContextZorderSendtoback)
	ON_UPDATE_COMMAND_UI(ID_CONTEXT_ZORDER_BRINGTOFRONT, OnUpdateContextZorderBringtofront)
	ON_UPDATE_COMMAND_UI(ID_CONTEXT_ZORDER_SENDTOBACK, OnUpdateContextZorderSendtoback)
	ON_UPDATE_COMMAND_UI(IDM_EDITDUPLICATE, OnUpdateEditduplicate)
	ON_COMMAND(ID_SPECIAL_CREATESPICEFILE, OnSpecialCreatespicefile)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPYTO, OnUpdateEditCopyto)
	ON_COMMAND(ID_EDIT_COPYTO, OnEditCopyto)
	ON_COMMAND(ID_CONTEXT_MAKEHORIZONTAL, OnContextMakehorizontal)
	ON_COMMAND(ID_CONTEXT_MAKEVERTICAL, OnContextMakevertical)
	ON_COMMAND(ID_FILE_SAVEASBITMAP, OnFileSaveasbitmap)
	ON_COMMAND(ID_OPTIONS_COLOURS, OnOptionsColours)
	ON_COMMAND(ID_CONTEXT_REPLACESYMBOL, OnContextReplacesymbol)
	ON_COMMAND(ID_EDIT_INSERTPICTURE, OnEditInsertpicture)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)

	ON_WM_INITMENUPOPUP()
	ON_WM_COMPACTING()
	ON_WM_DESTROYCLIPBOARD()
	ON_WM_CREATE()
	ON_WM_DESTROY()

	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()

	ON_WM_SYSKEYDOWN()
	ON_WM_SYSKEYUP()
	
	ON_WM_CLOSE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_SIZE()

	// The File Menu
	ON_COMMAND( IDM_FILEDESIGN, OnFileDesign )
	ON_COMMAND( IDM_FILEIMPORT, OnFileImport )
	ON_COMMAND( ID_FILE_PRINT, OnFilePrint )
	ON_COMMAND( IDM_FILEPAGESET, OnFilePageSet )

	// The Edit Menu
	ON_COMMAND( IDM_EDITUNDO, OnEditUndo )
	ON_COMMAND( IDM_EDITREDO, OnEditRedo )
	ON_COMMAND( IDM_EDITEDIT, OnEditEdit )
	ON_COMMAND( IDM_EDITDELITEM, OnEditDelete )
	ON_COMMAND( IDM_EDITDRAG, OnEditDrag )
	ON_COMMAND( IDM_EDITMOVE, OnEditMove )
	ON_COMMAND( IDM_EDITDUP, OnEditDup )
	ON_COMMAND( IDM_EDITROTATE, OnEditRotate )
	ON_COMMAND( ID_FIND_FIND, OnFindFind )
	ON_COMMAND( IDM_EDITCOPY, OnEditCopy )
	ON_COMMAND( IDM_EDITCUT, OnEditCut )
	ON_COMMAND( IDM_EDITPASTE, OnEditPaste )
	ON_COMMAND( IDM_EDITDUPLICATE, OnEditDuplicate )
	ON_COMMAND( IDM_EDITROTATELEFT, OnEditRotateLeft )
	ON_COMMAND( IDM_EDITROTATERIGHT, OnEditRotateRight )
	ON_COMMAND( IDM_EDITFLIP, OnEditFlip )


	// The Keyboard options
	ON_COMMAND( IDM_VIEWZOOMIN, OnViewZoomIn )
	ON_COMMAND( IDM_VIEWZOOMOUT, OnViewZoomOut )
	
	// The Toolbar Menu
	ON_COMMAND( IDM_VIEWCENTRE, OnViewCentre )
	ON_COMMAND( IDM_TOOLBUSNAME, OnSelectBusName )
	ON_COMMAND( IDM_TOOLBUS, OnSelectBus )
	ON_COMMAND( IDM_BUSSLASH, OnSelectBusSlash )
	ON_COMMAND( IDM_BUSBACK, OnSelectBusBack )
	ON_COMMAND( IDM_TOOLCONNECT, OnSelectConnect )
	ON_COMMAND( IDM_TOOLLABEL, OnSelectLabel )
	ON_COMMAND( IDM_TOOLHIERACHICAL, OnSelectHierachical )
	ON_COMMAND( IDM_TOOLPOWER, OnSelectPower )
	ON_COMMAND( IDM_TOOLPOLYGON, OnSelectPolygon )
	ON_COMMAND( IDM_TOOLWIRE, OnSelectWire )
	ON_COMMAND( IDM_TOOLJUNC, OnSelectJunction )
	ON_COMMAND( IDM_TOOLARC, OnSelectArc )
	ON_COMMAND( IDM_TOOLSQUARE, OnSelectSquare )
	ON_COMMAND( IDM_TOOLCIRCLE, OnSelectCircle )
	ON_COMMAND( IDM_TOOLTEXT, OnSelectText )
	ON_COMMAND( IDM_TOOLGET, OnSelectGet )
	ON_COMMAND( IDC_SHOW_SYMBOL, OnSelectGet )

	// The Special Menu
	ON_COMMAND( IDM_SPECIALANOTATE, OnSpecialAnotate )
	ON_COMMAND( IDM_SPECIALBOM, OnSpecialBom )
	ON_COMMAND( IDM_SPECIALNET, OnSpecialNet )
	ON_COMMAND( IDM_SPECIALCHECK, OnSpecialCheck )

	// The Repeat Menu
	ON_COMMAND( IDM_REPEATNAMEUP, OnRepeatNameUp )
	ON_COMMAND( IDM_REPEATNAMEDOWN, OnRepeatNameDown )
	ON_COMMAND( IDM_REPEATPINUP, OnRepeatPinUp )
	ON_COMMAND( IDM_REPEATPINDOWN, OnRepeatPinDown )

	ON_COMMAND(ID_CONTEXT_OPENDESIGN, OnContextOpendesign)
	ON_COMMAND(ID_CONTEXT_RELOADSYMBOLFROMDESIGN, OnContextReloadsymbolfromdesign)
	END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTinyCadView construction/destruction

CTinyCadView::CTinyCadView()
{
	vRuler = NULL;
	hRuler = NULL;

	// We have not captured the mouse
	m_captured = FALSE;
	m_panning = FALSE;

	// Turn on off-screen bitmap drawing
	m_use_offscreen_drawing = TRUE;

	m_PrintAllSheets = TRUE;
	m_Printing = FALSE;
}

CTinyCadView::~CTinyCadView()
{
}

BOOL CTinyCadView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	// Create our own Window's class for this window
	static CString theClass = AfxRegisterWndClass( 0, AfxGetApp()->LoadStandardCursor( IDC_ARROW ) ); 
	cs.lpszClass = theClass;

	return CView::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
// CTinyCadView printing

// Our own version of this function, so that we
// can set portrait/landscape mode automatically
BOOL CTinyCadView::DoPreparePrinting(CPrintInfo* pInfo)
{
	ASSERT(pInfo != NULL);
	ASSERT(pInfo->m_pPD != NULL);

	if (pInfo->m_pPD->m_pd.nMinPage > pInfo->m_pPD->m_pd.nMaxPage)
		pInfo->m_pPD->m_pd.nMaxPage = pInfo->m_pPD->m_pd.nMinPage;

	// don't prompt the user if we're doing print preview, printing directly,
	// or printing via IPrint and have been instructed not to ask

	CTinyCadApp* pApp = static_cast<CTinyCadApp*>(AfxGetApp());
	if (pInfo->m_bPreview || pInfo->m_bDirect ||
		(pInfo->m_bDocObject && !(pInfo->m_dwFlags & PRINTFLAG_PROMPTUSER)))
	{
		if (pInfo->m_pPD->m_pd.hDC == NULL)
		{
			// if no printer set then, get default printer DC and create DC without calling
			//   print dialog.
			if (!pApp->GetPrinterDeviceDefaults(&pInfo->m_pPD->m_pd))
			{
				// bring up dialog to alert the user they need to install a printer.
				if (!pInfo->m_bDocObject || (pInfo->m_dwFlags & PRINTFLAG_MAYBOTHERUSER))
					if (pApp->DoPrintDialog(pInfo->m_pPD) != IDOK)
						return FALSE;
			}

			if (pInfo->m_pPD->m_pd.hDC == NULL)
			{
				// Switch to landscape mode...
				DEVMODE *pdev_mode;
				pdev_mode = pInfo->m_pPD->GetDevMode();
				pdev_mode->dmOrientation = GetCurrentDocument()->GetDetails().IsPortrait() ? DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE;
				GlobalUnlock( pdev_mode );
				
				// call CreatePrinterDC if DC was not created by above
				if (pInfo->m_pPD->CreatePrinterDC() == NULL)
					return FALSE;
			}
		}

		// set up From and To page range from Min and Max
		pInfo->m_pPD->m_pd.nFromPage = (WORD)pInfo->GetMinPage();
		pInfo->m_pPD->m_pd.nToPage = (WORD)pInfo->GetMaxPage();
	}
	else
	{
		// Get the print scaling to determine the correct "fit to page"
		// scaling factor
		CPrintDialog pdlg( FALSE, 0, AfxGetMainWnd() );
		HDC hdc = NULL;

		if (AfxGetApp()->GetPrinterDeviceDefaults(&pdlg.m_pd))
		{
			if (pdlg.m_pd.hDC == NULL)
			{
				// call CreatePrinterDC if DC was not created by above
				hdc = pdlg.CreatePrinterDC();
			}
		}
		
		int width = 1024;
		int height = 768;
		if (hdc) 
		{
			// Change the scaling to print correct size
			width = max( ::GetDeviceCaps(hdc, HORZSIZE), ::GetDeviceCaps(hdc, VERTSIZE) ); 
			height = min( ::GetDeviceCaps(hdc, HORZSIZE), ::GetDeviceCaps(hdc, VERTSIZE) ); 
		}

		::DeleteDC( hdc );

	
		// otherwise, bring up the print dialog and allow user to change things
		// preset From-To range same as Min-Max range
		delete pInfo->m_pPD;
		CSpecialPrintDlg *dlg = new CSpecialPrintDlg( FALSE );

		if (GetCurrentDocument()->GetDetails().IsPortrait())
		{
			dlg->m_FitScale = min(
				static_cast<double>(width * 100 * PIXELSPERMM) / static_cast<double>(GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().y),
				static_cast<double>(height * 100 * PIXELSPERMM) / static_cast<double>(GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().x) );
		}
		else
		{
			dlg->m_FitScale = min(
				static_cast<double>(width * 100 * PIXELSPERMM) / static_cast<double>(GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().x),
				static_cast<double>(height * 100 * PIXELSPERMM) / static_cast<double>(GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().y) );
		}
		pInfo->m_pPD = dlg;
		pInfo->m_pPD->m_pd.nFromPage = (WORD)pInfo->GetMinPage();
		pInfo->m_pPD->m_pd.nToPage = 0xfff; // (WORD)pInfo->GetMaxPage();
		pInfo->m_pPD->m_pd.Flags &= ~PD_RETURNDC;

		if (pApp->DoPrintDialog(pInfo->m_pPD) != IDOK)
			return FALSE;       // do not print

		m_PrintAllSheets = dlg->m_print_all_sheets != 0;

		// Switch to landscape mode...
		DEVMODE *pdev_mode;
		pdev_mode = dlg->GetDevMode();
		pdev_mode->dmOrientation = GetCurrentDocument()->GetDetails().IsPortrait() ? DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE;
		pdev_mode->dmCopies = dlg->m_Copies;
		GlobalUnlock( pdev_mode );

		// call CreatePrinterDC if DC was not created by above
		if (pInfo->m_pPD->CreatePrinterDC() == NULL)
			return FALSE;

	}

	ASSERT(pInfo->m_pPD != NULL);
	ASSERT(pInfo->m_pPD->m_pd.hDC != NULL);
	if (pInfo->m_pPD->m_pd.hDC == NULL)
		return FALSE;

	pInfo->m_nNumPreviewPages = pApp->m_nNumPreviewPages;
	VERIFY(pInfo->m_strPageDesc.LoadString(AFX_IDS_PREVIEWPAGEDESC));
	return TRUE;
}



BOOL CTinyCadView::OnPreparePrinting(CPrintInfo* pInfo)
{

	// Get rid of any drawing tool
	GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument()));

	// Prepare using our version of this function to force
	// portrait/landscape mode
	return DoPreparePrinting(pInfo);
}

void CTinyCadView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Calculate how many cols and rows of pages to use
	double width;
	double height;

	width = max( pDC->GetDeviceCaps(HORZSIZE), pDC->GetDeviceCaps(VERTSIZE) ) * PIXELSPERMM; 
	height = min( pDC->GetDeviceCaps(HORZSIZE), pDC->GetDeviceCaps(VERTSIZE) ) * PIXELSPERMM; 


	double scale = CTinyCadRegistry::GetPrintScale();
	double scale_x, scale_y;

	if (GetCurrentDocument()->GetDetails().IsPortrait())
	{
		scale_x	= (scale * GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().x / height) / 100.0;
		scale_y	= (scale * GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().y / width) / 100.0;
	}
	else
	{
		scale_x	= (scale * GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().x / width) / 100.0;
		scale_y	= (scale * GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().y / height) / 100.0;
	}

	BOOL Fit = FALSE;

	if (Fit) 
	{
		m_cols = 1;
		m_rows = 1;
	} 
	else 
	{
		// Round to the nearest scaling factor...
		m_cols = static_cast<int>(scale_x);
		m_rows = static_cast<int>(scale_y);

		double sx = scale_x - static_cast<int>(scale_x);
		double sy = scale_y - static_cast<int>(scale_y);

		// Is this an exact fit or not?
		if (sx > 0.01)
		{
			m_cols ++;
		}

		if (sy > 0.01)
		{
			m_rows ++;
		}
	}
	
	int pages = 1;
	if (m_PrintAllSheets)
	{
		pages = m_cols * m_rows * GetDocument()->GetNumberOfSheets();
	}
	else
	{
		pages = m_cols * m_rows;
	}

	pInfo->SetMinPage( 1 );
	pInfo->SetMaxPage( pages );

	// Change the zoom so a single unit on the design corresponds to a pixel on the printer
	m_Printing = TRUE;
	current_Sheet = GetDocument()->GetActiveSheetIndex();
	double NewZoom = (pDC->GetDeviceCaps(LOGPIXELSX)*1000) / (PIXELSPERMM*254);

	if (Fit)
	{
		NewZoom = NewZoom / max(scale_x,scale_y);
	}
	else
	{
		NewZoom = (NewZoom * scale) / 100.0;
	}

	GetTransform().SetZoomFactor(NewZoom / 100.0);
}

void CTinyCadView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{

	if (pInfo)
	{
		// Change the offset co-ordinates to print the correct portion of the design
		int width = pDC->GetDeviceCaps(HORZSIZE) * PIXELSPERMM;
		int height = pDC->GetDeviceCaps(VERTSIZE) * PIXELSPERMM;

		int col = 0;
		int row = 0;

		if (m_PrintAllSheets)
		{
			int sheets = GetDocument()->GetNumberOfSheets();
			int page = (pInfo->m_nCurPage-1) / sheets;
			int sheet = (pInfo->m_nCurPage-1) % sheets;
			GetDocument()->SetActiveSheetIndex( sheet );
			col = page / m_rows;
			row = page % m_rows;
		}
		else
		{
			col = (pInfo->m_nCurPage-1) / m_rows;
			row = (pInfo->m_nCurPage-1) % m_rows;
		}

		double scale = CTinyCadRegistry::GetPrintScale();
		int x=static_cast<int>((col * width / scale) * 100);
		int y=static_cast<int>((row * height / scale) * 100);
		GetTransform().SetOriginX(x);
		GetTransform().SetOriginY(y);
	}
	
	CView::OnPrepareDC(pDC, pInfo);
}


void CTinyCadView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	GetDocument()->SetActiveSheetIndex( current_Sheet );
	m_Printing = FALSE;
	m_PrintAllSheets = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CTinyCadView diagnostics

#ifdef _DEBUG
void CTinyCadView::AssertValid() const
{
	CView::AssertValid();
}

void CTinyCadView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG


CTinyCadDoc* CTinyCadView::GetCurrentDocument() // non-debug version is inline
{
	return GetDocument()->GetCurrentSheet();
}




/////////////////////////////////////////////////////////////////////////////
// CTinyCadView message handlers

class CDlgPositionBox;
// extern CDlgPositionBox thePositionBox;
#define thisDesign (&((static_cast<CTinyCadView *>(m_pMainWnd))->theDesign))


/////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////



// The status bar indicators
static UINT BASED_CODE indicators[] =
{
	ID_SEPARATOR,			// status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
};



// The OnCreate function, called when the window is created
int CTinyCadView::OnCreate( LPCREATESTRUCT q )
{
	CView::OnCreate( q );

	// Now create the new rulers
  	CRect nSize;
  	GetClientRect(nSize);
  	vRuler = new Ruler(GetDocument(), 0, nSize, this);
  	hRuler = new Ruler(GetDocument(), 2, nSize, this);

  ClipboardFormat = RegisterClipboardFormat(CLIPBOARD_FORMAT);


	// initialize the cursor array
	HINSTANCE hInst = AfxGetResourceHandle();
	m_mouse_pointers[0] = ::LoadCursor(hInst, MAKEINTRESOURCE(AFX_IDC_TRACKNWSE));
	m_mouse_pointers[1] = ::LoadCursor(hInst, MAKEINTRESOURCE(AFX_IDC_TRACKNESW));
	m_mouse_pointers[2] = m_mouse_pointers[0];
	m_mouse_pointers[3] = m_mouse_pointers[1];
	m_mouse_pointers[4] = ::LoadCursor(hInst, MAKEINTRESOURCE(AFX_IDC_TRACKNS));
	m_mouse_pointers[5] = ::LoadCursor(hInst, MAKEINTRESOURCE(AFX_IDC_TRACKWE));
	m_mouse_pointers[6] = m_mouse_pointers[4];
	m_mouse_pointers[7] = m_mouse_pointers[5];
	m_mouse_pointers[8] = ::LoadCursor(hInst, MAKEINTRESOURCE(AFX_IDC_TRACK_MOVE));
	m_mouse_pointers[9] = ::LoadCursor(hInst, MAKEINTRESOURCE(IDC_ZOOMCURSOR));
	m_mouse_pointers[10] = ::LoadCursor(hInst, MAKEINTRESOURCE(IDC_REFCURSOR));
	m_mouse_pointers[11] = ::LoadCursor(hInst, MAKEINTRESOURCE(AFX_IDC_TRACK4WAY));
	m_mouse_pointers[12] = ::LoadCursor(hInst, MAKEINTRESOURCE(AFX_IDC_TRACK_BLOCK));


	return 0;
}


// CTinyCadView constructor:
// Create the window with the appropriate style, size, menu, etc.
//


// Called when the window is about to be destroyed
void CTinyCadView::OnDestroy()
{
	// Close the ERC list box
	theERCListBox.Close();

	vRuler->DestroyWindow();
	delete vRuler;

	hRuler->DestroyWindow();
	delete hRuler;
}




// Display a standard message using one of the resource strings
int Message(int Resource,int Type,const TCHAR *NameString)
{
  TCHAR String[STRLEN], buffer[STRLEN];
  int r;

  if (LoadString(AfxGetInstanceHandle(),Resource,String,1024)==0)
	r = AfxMessageBox(_T("Could not find specified resource!  There is a fault in the file TinyCAD.EXE, please re-install it."),MB_ICONEXCLAMATION | MB_OK);
  else {
	// Play the sound associated with this message
	MessageBeep(Type & 0x70);
	_stprintf_s(buffer,String,NameString);
	r = AfxMessageBox(buffer,Type);
  }

  return r;
}



////// The mouse movement operators //////

void CTinyCadView::OnMouseMove(UINT nFlags, CPoint p)
{

	CContext theContext(this,GetTransform());

	if (vRuler != NULL)
		vRuler->ShowPosition(p);
	if (hRuler != NULL)
		hRuler->ShowPosition(p);


	if (!m_pDocument)
	  return;

	CDPoint snap_p=GetTransform().DeScale(GetCurrentDocument()->m_snap,p);
    CDPoint no_snap_p = GetTransform().DeScale(p);


	// If we are panning, then don't track this movement...
	if (m_panning)
	{
		// Pan the screen with the mouse
		double x = GetTransform().GetOrigin().x;
		double y = GetTransform().GetOrigin().y;
		CDPoint old_mouse = MousePosition;
		SetScroll(x - snap_p.x + old_mouse.x, y - snap_p.y + old_mouse.y);

		MousePosition=GetTransform().DeScale(GetCurrentDocument()->m_snap,p);
	}
	else
	{

		MousePosition=snap_p;
	
		// Now display the position
		CString pos = GetCurrentDocument()->GetOptions()->PointToUnit(snap_p);
		static_cast<CMainFrame*>(AfxGetMainWnd())->setPositionText( pos );

		// Send this to the editable device
		if (GetCurrentDocument()->GetEdit())
		{
			GetCurrentDocument()->GetEdit()->Move(snap_p, no_snap_p);
		}
	}
}

void CTinyCadView::OnLButtonDown(UINT nFlags, CPoint p)
{
  CContext theContext(this,GetTransform());

  CDPoint snapped_p = GetTransform().DeScale(GetCurrentDocument()->m_snap,p);
  CDPoint no_snap_p = GetTransform().DeScale(p);

  MousePosition=snapped_p;

  GetCurrentDocument()->GetEdit()->LButtonDown(snapped_p,no_snap_p);

  SetCapture();
  m_captured = TRUE;
}

void CTinyCadView::OnLButtonDblClk(UINT nFlags, CPoint p) 
{
  CContext theContext(this,GetTransform());

  CDPoint snapped_p = GetTransform().DeScale(GetCurrentDocument()->m_snap,p);
  CDPoint no_snap_p = GetTransform().DeScale(p);

  MousePosition=snapped_p;

  GetCurrentDocument()->GetEdit()->DblLButtonDown(snapped_p,no_snap_p);
	
  CView::OnLButtonDblClk(nFlags, p);
}

void CTinyCadView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_captured)
	{
		ReleaseCapture();

		CContext theContext(this,GetTransform());

		CDPoint p=GetTransform().DeScale(GetCurrentDocument()->m_snap,point);

		MousePosition=p;

		GetCurrentDocument()->GetEdit()->LButtonUp(p);
	}
	
	CView::OnLButtonUp(nFlags, point);
}



void CTinyCadView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// Change cursor to 'block select' cursor
	OnSetCursor(this, HTCLIENT, WM_SYSKEYDOWN);
}


void CTinyCadView::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (!OnSetCursor(this, HTCLIENT, WM_SYSKEYUP))
	{
		// Change cursor back to normal
		SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );
	}
}


BOOL CTinyCadView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// trackers should only be in client area
	if (nHitTest == HTCLIENT)
	{
		int cursor = -1;
		CPoint point;
		GetCursorPos(&point);
		pWnd->ScreenToClient(&point);
		CDPoint no_snap_p = GetTransform().DeScale(point);
		if (GetCurrentDocument()->GetEdit())
		{
			cursor = GetCurrentDocument()->GetEdit()->SetCursor( no_snap_p );
		}

		if (cursor != -1)
		{
			::SetCursor(m_mouse_pointers[cursor]);
			return TRUE;
		}
	}

	return CView::OnSetCursor(pWnd, nHitTest, message);
}





void CTinyCadView::OnRButtonDown(UINT nFlags, CPoint p)
{
  // Create a Context for this Click
  CContext theContext(this,GetTransform());

  CDPoint snapped_p = GetTransform().DeScale(GetCurrentDocument()->m_snap,p);
  CDPoint no_snap_p = GetTransform().DeScale(p);

  MousePosition=snapped_p;

  // Call the end function, if it returns false then delete
  // this object
  if (!GetCurrentDocument()->GetEdit()->RButtonDown(snapped_p,no_snap_p))
  {
	GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument()));
  }
}

void CTinyCadView::OnRButtonUp(UINT nFlags, CPoint p)
{
  CContext theContext(this,GetTransform());
  CDPoint snapped_p = GetTransform().DeScale(GetCurrentDocument()->m_snap,p);
  CDPoint no_snap_p = GetTransform().DeScale(p);

  MousePosition=snapped_p;

  GetCurrentDocument()->GetEdit()->RButtonUp(snapped_p, no_snap_p );
}


void CTinyCadView::OnMButtonDown( UINT nFlags, CPoint point )
{
	m_panning = TRUE;
	SetCapture();
}

void CTinyCadView::OnMButtonUp( UINT nFlags, CPoint point )
{
	m_panning = FALSE;
	ReleaseCapture();
}




// This function takes file names and shortens them (if necessary)
CString NameLength(const TCHAR *s,int MaxLen)
{
  CString in = s;
  int len = in.GetLength();
  int diff = MaxLen - len;

  // Is the string short enough as it is?
  if (diff>=0)
	return in;

  // Otherwise remove sufficient characters so it is
  if ((diff&1)!=0)
	diff++;

  return (in.Left((len+diff)/2-2) + "..." + in.Mid((len-diff)/2+1));
}


void CTinyCadView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	switch (lHint)
	{
	case DOC_UPDATE_SETCURSOR:
		{
			int cursor = GetCurrentDocument()->GetEdit()->SetCursor( MousePosition );
			if (cursor != -1)
			{
				::SetCursor(m_mouse_pointers[cursor]);
			}
		}
		break;
	case DOC_UPDATE_INVALIDATE:
		Invalidate();
		break;
	case DOC_UPDATE_INVALIDRECT:
	case DOC_UPDATE_INVALIDRECTERASE:
		{
			doc_invalidrect* hint = static_cast<doc_invalidrect*>(pHint);
			CContext dc(this, GetTransform() );
			dc.InvalidateRect( hint->r, lHint == DOC_UPDATE_INVALIDRECTERASE, hint->grow );
		}
		break;
	case DOC_UPDATE_TABS:
		SetTabsFromDocument();
		break;
	}
	
}


void CTinyCadView::OnRulerVert() 
{
	GetCurrentDocument()->SelectObject(new CDrawRuler(GetCurrentDocument(),FALSE));
	
}

void CTinyCadView::OnUpdateRulerVert(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}

}

void CTinyCadView::OnRulerHoriz() 
{
	GetCurrentDocument()->SelectObject(new CDrawRuler(GetCurrentDocument(),TRUE));
}

void CTinyCadView::OnUpdateRulerHoriz(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}	
}


void CTinyCadView::OnUpdateEditedit(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}

void CTinyCadView::OnUpdateBusback(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}

void CTinyCadView::OnUpdateBusslash(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}

void CTinyCadView::OnUpdateToolarc(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}

void CTinyCadView::OnUpdateToolbus(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}

void CTinyCadView::OnUpdateToolbusname(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}

void CTinyCadView::OnUpdateToolcircle(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}

void CTinyCadView::OnUpdateToolconnect(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}

void CTinyCadView::OnUpdateToolget(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}

void CTinyCadView::OnUpdateTooljunc(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}

void CTinyCadView::OnUpdateToollabel(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}

void CTinyCadView::OnUpdateToolHierachical(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
	pCmdUI->Enable( !GetCurrentDocument()->IsHierarchicalSymbol() );
}

void CTinyCadView::OnUpdateToolpolygon(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}

void CTinyCadView::OnUpdateToolpower(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}

void CTinyCadView::OnUpdateToolsquare(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}

void CTinyCadView::OnUpdateTooltext(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}

void CTinyCadView::OnUpdateToolwire(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}

void CTinyCadView::OnUpdateViewcentre(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}


void CTinyCadView::OnUpdateEditdrag(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}	
}

void CTinyCadView::OnUpdateEditdup(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
	
}

void CTinyCadView::OnUpdateEditrotate(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}	
}

void CTinyCadView::OnUpdateEditmove(CCmdUI* pCmdUI) 
{
	CDrawingObject *q = GetCurrentDocument()->GetEdit();
	if (q)	
	{
		pCmdUI->SetCheck( q->getMenuID() == pCmdUI->m_nID );
	}
}

void CTinyCadView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	// Set the offsets
	SetScroll(0,0, true);

	SetTabsFromDocument();


 	if( CTinyCadRegistry::GetMDIMaximize() )
	{
		((CMDIChildWnd *)GetParentFrame())->MDIMaximize();
	}

	// Center Symbol inside view
	if (GetCurrentDocument()->IsEditLibrary())
	{
		// Center drawing inside view
   		drawingIterator it = GetCurrentDocument()->GetDrawingBegin();
		if (it != GetCurrentDocument()->GetDrawingEnd()) {
			CDrawingObject *obj = *it;
			CDRect ext(obj->m_point_a.x,obj->m_point_a.y,obj->m_point_b.x,obj->m_point_b.y);
			ext.NormalizeRect();
			while (it != GetCurrentDocument()->GetDrawingEnd()) 
			{
				obj = (CDrawingObject *)*it;
				CDRect box(obj->m_point_a.x,obj->m_point_a.y,obj->m_point_b.x,obj->m_point_b.y);
				box.NormalizeRect();

				if (ext.left > box.left)
					ext.left = box.left;

				if (ext.top > box.top)
					ext.top = box.top;

				if (ext.right < box.right)
					ext.right = box.right;

				if (ext.bottom < box.bottom)
					ext.bottom = box.bottom;

				++ it;
			}

			SetScrollCentre(CDPoint((ext.left + ext.right)/2, (ext.top + ext.bottom)/2));
		}
	}

}

void CTinyCadView::SetTabsFromDocument()
{
	CFolderTabCtrl& ftc = GetFolderFrame()->GetFolderTabCtrl();
	CMultiSheetDoc *pDoc = GetDocument();
	int docs = pDoc->GetNumberOfSheets();

	// Has the document been initialised yet?
	if (docs == 0)
	{
		return;
	}

	while (ftc.GetItemCount() > 0)
	{
		ftc.RemoveItem( 0 );
	}

	for (int i=0; i < docs; i++)
	{
		ftc.AddItem( pDoc->GetSheetName( i ) );
	}


	GetFolderFrame()->ShowControls( CFolderFrame::bestFit );
	ftc.SelectItem( GetDocument()->GetActiveSheetIndex() );
}


////// The Snap to Grid menu //////

void CTinyCadView::OnSnaptogrid() 
{
	GetCurrentDocument()->SetSnapToGrid( !GetCurrentDocument()->GetSnapToGrid() );
}

void CTinyCadView::OnUpdateSnaptogrid(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( GetCurrentDocument()->GetSnapToGrid() ? 1 : 0 );	
}






////// The REPEAT menu //////

void CTinyCadView::OnRepeatNameUp()
{
	GetCurrentDocument()->SetNameDir( 1 );
}

void CTinyCadView::OnUpdateRepeatnameup(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( GetCurrentDocument()->GetNameDir() == 1 ? 1 : 0 );
	
}

void CTinyCadView::OnRepeatNameDown()
{
	GetCurrentDocument()->SetNameDir( -1 );
}

void CTinyCadView::OnUpdateRepeatnamedown(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( GetCurrentDocument()->GetNameDir() == -1 ? 1 : 0 );
}


void CTinyCadView::OnRepeatPinUp()
{
  GetCurrentDocument()->SetPinDir( 1 );
}

void CTinyCadView::OnUpdateRepeatpinup(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( GetCurrentDocument()->GetPinDir() == 1 ? 1 : 0 );
	
}


void CTinyCadView::OnRepeatPinDown()
{
	GetCurrentDocument()->SetPinDir( -1 );
}


void CTinyCadView::OnUpdateRepeatpindown(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( GetCurrentDocument()->GetPinDir() == -1 ? 1 : 0 );
}




void CTinyCadView::OnUpdateEditpaste(CCmdUI* pCmdUI) 
{
	BOOL r = IsClipboardAvailable() 
		|| ::IsClipboardFormatAvailable( CF_ENHMETAFILE )
		|| ::IsClipboardFormatAvailable( CF_BITMAP );
	pCmdUI->Enable( r );	
}

void CTinyCadView::OnUpdateEditcut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetCurrentDocument()->IsSelected() );	
}

void CTinyCadView::OnUpdateEditcopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( TRUE );
}

//=======


void CTinyCadView::OnUpdateEditRotateLeft(CCmdUI* pCmdUI) 
{
	ObjType type = GetCurrentDocument()->GetEdit()->GetType();
	BOOL r = (GetCurrentDocument()->IsSelected() && type == xEditItem) || type == xMethodEx3 || type == xAnotation || type == xLabelEx2 || type == xPower;
 	pCmdUI->Enable( r );	
}

void CTinyCadView::OnUpdateEditRotateRight(CCmdUI* pCmdUI) 
{
	ObjType type = GetCurrentDocument()->GetEdit()->GetType();
	BOOL r = (GetCurrentDocument()->IsSelected() && type == xEditItem) || type == xMethodEx3 || type == xAnotation || type == xLabelEx2 || type == xPower;
 	pCmdUI->Enable( r );	
}

void CTinyCadView::OnUpdateEditFlip(CCmdUI* pCmdUI) 
{
	ObjType type = GetCurrentDocument()->GetEdit()->GetType();
	BOOL r = (GetCurrentDocument()->IsSelected() && type == xEditItem) || type == xMethodEx3 || type == xAnotation || type == xLabelEx2 || type == xPower;
 	pCmdUI->Enable( r );	
}



//======



void CTinyCadView::OnUpdateEditduplicate(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetCurrentDocument()->IsSelected() );	
}

void CTinyCadView::OnUpdateEditCopyto(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetCurrentDocument()->IsSelected() );	
}



BOOL CTinyCadView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if (zDelta > 0)
	{
		OnViewZoomIn();
	}
	else
	{
		OnViewZoomOut();
	}
	
	return TRUE;
}

void CTinyCadView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	if (!bActivate)
	{
		// Get rid of any drawing tool at this moment
		GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument()));
	}
	
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}



void CTinyCadView::OnViewOptions() 
{
	COptionsPropertySheet propSheet;

	propSheet.m_pDocument = GetCurrentDocument();
	propSheet.DoModal();

  if (hRuler != NULL)
  	hRuler->RedrawWindow();
  if (vRuler != NULL)
  	vRuler->RedrawWindow();

  Invalidate();
}

void CTinyCadView::OnEditDelete()
{
	GetCurrentDocument()->GetEdit()->ContextMenu( MousePosition, IDM_EDITDELITEM );

}

void CTinyCadView::OnContextMakehorizontal() 
{
	GetCurrentDocument()->GetEdit()->ContextMenu( MousePosition, ID_CONTEXT_MAKEHORIZONTAL );
	
}

void CTinyCadView::OnContextMakevertical() 
{
	GetCurrentDocument()->GetEdit()->ContextMenu( MousePosition, ID_CONTEXT_MAKEVERTICAL );
	
}


void CTinyCadView::OnContextArcin() 
{
	GetCurrentDocument()->GetEdit()->ContextMenu( MousePosition, ID_CONTEXT_ARCIN );
	
}

void CTinyCadView::OnContextArcout() 
{
	GetCurrentDocument()->GetEdit()->ContextMenu( MousePosition, ID_CONTEXT_ARCOUT );
}

void CTinyCadView::OnContextFreeline() 
{
	GetCurrentDocument()->GetEdit()->ContextMenu( MousePosition, ID_CONTEXT_FREELINE );
}

void CTinyCadView::OnContextAddhandle() 
{
	GetCurrentDocument()->GetEdit()->ContextMenu( MousePosition, ID_CONTEXT_ADDHANDLE );
}

void CTinyCadView::OnContextDeletehandle() 
{
	GetCurrentDocument()->GetEdit()->ContextMenu( MousePosition, ID_CONTEXT_DELETEHANDLE );
}

void CTinyCadView::OnContextCanceldrawing() 
{
	// switch back to the Edit tool
	GetCurrentDocument()->SelectObject( new CDrawEditItem(GetCurrentDocument()) );	
}

void CTinyCadView::OnContextFinishdrawing() 
{
	GetCurrentDocument()->GetEdit()->FinishDrawing(MousePosition);
}

void CTinyCadView::OnContextReplacesymbol() 
{
	GetCurrentDocument()->GetEdit()->ContextMenu( MousePosition, ID_CONTEXT_REPLACESYMBOL );
}

void CTinyCadView::OnContextOpendesign()
{
	GetCurrentDocument()->GetEdit()->ContextMenu( MousePosition, ID_CONTEXT_OPENDESIGN );
}

void CTinyCadView::OnContextReloadsymbolfromdesign()
{
	GetCurrentDocument()->GetEdit()->ContextMenu( MousePosition, ID_CONTEXT_RELOADSYMBOLFROMDESIGN );
}



void CTinyCadView::OnContextZorderBringtofront() 
{
	GetCurrentDocument()->BringToFront();
}

void CTinyCadView::OnContextZorderSendtoback() 
{
	GetCurrentDocument()->SendToBack();
}

void CTinyCadView::OnUpdateContextZorderBringtofront(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetCurrentDocument()->IsSelected() );		
}

void CTinyCadView::OnUpdateContextZorderSendtoback(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetCurrentDocument()->IsSelected() );		
}


void CTinyCadView::OnFileSaveasbitmap() 
{

	// Get the file in which to save the network
	TCHAR szFile[256];

	_tcscpy_s(szFile,GetDocument()->GetPathName());
	TCHAR* ext = _tcsrchr(szFile,'.');
	if (!ext)
	{
		_tcscpy_s(szFile,_T("output.png"));
	}
	else
	{
		#ifdef USE_VS2003
			_tcscpy(ext, _T(".png"));
		#else
			size_t remaining_space = &szFile[255] - ext + 1;
			_tcscpy_s(ext, remaining_space,_T(".png"));
		#endif
	}

	CDlgExportPNG dlg;
	dlg.m_Filename = szFile;

	if (dlg.DoModal() != IDOK)
		return;

	CClientDC dc( this );
	switch (dlg.m_type)
	{
	case 0: // Colour PNG
		GetCurrentDocument()->SavePNG( dlg.m_Filename, dc, dlg.m_Scaling, false, dlg.m_Rotate );
		break;
	case 1: // B&W PNG
		GetCurrentDocument()->SavePNG( dlg.m_Filename, dc, dlg.m_Scaling, true, dlg.m_Rotate );
		break;
	case 2: // Colour EMF
		GetCurrentDocument()->CreateMetafile( dc, dlg.m_Filename, false );
		break;
	case 3: // B&W EMF
		GetCurrentDocument()->CreateMetafile( dc, dlg.m_Filename, true );
		break;
	}
}
//-------------------------------------------------------------------------
void CTinyCadView::OnOptionsColours() 
{
	CDlgColours( GetCurrentDocument()->GetOptions()->GetUserColor() ).DoModal();

		RedrawWindow();
}
//-------------------------------------------------------------------------
// The user has changed the current folder
void CTinyCadView::OnChangedFolder(int iPage)
{
	// switch back to the Edit tool
	GetCurrentDocument()->SelectObject( new CDrawEditItem(GetCurrentDocument()) );	

	// Now change the active sheet
	GetDocument()->SetActiveSheetIndex( iPage );
	SetScroll(GetTransform().GetOrigin().x,GetTransform().GetOrigin().y, true);
	RedrawWindow();
}
//-------------------------------------------------------------------------

void CTinyCadView::OnFolderContextMenu()
{
	GetDocument()->OnFolderContextMenu();
}



#define ALL_IMAGE_FILES _T("*.png;*.emf;*.bmp;*.jpeg;*.jpe;*.jpg")
void CTinyCadView::OnEditInsertpicture() 
{
	// switch back to the Edit tool
	GetCurrentDocument()->SelectObject( new CDrawEditItem(GetCurrentDocument()) );	

	CFileDialog dlg( TRUE, ALL_IMAGE_FILES, ALL_IMAGE_FILES, OFN_HIDEREADONLY,
		_T("Image files|") ALL_IMAGE_FILES _T("|")
		_T("Portable network graphic (*.png)|*.png|")
		_T("JPEG (*.jpeg)|*.jpeg;*.jpg;*.jpe|")
		_T("Windows bitmaps(*.bmp,*.dib)|*.bmp;*.dib|")
		_T("Enhanced metafile (*.emf)|*.emf|")
		_T("All files (*.*)|*.*||"), AfxGetMainWnd() ); 


    if (dlg.DoModal() != IDOK)
  		return;	

	CDrawMetaFile *pObject = new CDrawMetaFile( GetCurrentDocument() );
	if (pObject->setImageFile( dlg.GetPathName() ))
	{
		CClientDC dc(this);
		pObject->determineSize( dc );
		GetCurrentDocument()->AddImage( pObject );
	}
	else
	{
		delete pObject;
	}
}



void CTinyCadView::SelectSheet(int sheet)
{
	CFolderTabCtrl& ftc = GetFolderFrame()->GetFolderTabCtrl();
	GetDocument()->SetActiveSheetIndex( sheet );
	ftc.SelectItem( GetDocument()->GetActiveSheetIndex() );
}


void CTinyCadView::ChangeDir(int dir) 
{
	ObjType type = GetCurrentDocument()->GetEdit()->GetType();
	// Rotate selection
	if (type == xEditItem)
	{
		static_cast<CDrawEditItem*>(GetCurrentDocument()->GetEdit())->ChangeDir(dir);

		if (GetCurrentDocument()->IsSingleItemSelected() 
		&& (GetCurrentDocument()->GetSingleSelectedItem())->CanEdit()) 
		{
			// reflect updates in tool window
			(GetCurrentDocument()->GetSingleSelectedItem())->BeginEdit(TRUE);
		}
	}

	// Rotate annotation (E.g. import symbol into other symbol)
	else if (type == xAnotation)
	{
		// Use the ChangeDir function in CDrawEditItem
		CDrawEditItem* edit = new CDrawEditItem(GetCurrentDocument());
		edit->ChangeDir(dir);
		delete edit;
	}

	// Rotate object while placing it
	else if (type == xMethodEx3 || type == xPower || type == xLabelEx2)
	{
		CDrawMethod* edit = static_cast<CDrawMethod*>(GetCurrentDocument()->GetEdit());
		// Update screen
		edit->Display(TRUE);

		edit->Rotate(CDPoint(0,0), dir);
		edit->Display(TRUE);

		// reflect updates in tool window
		edit->BeginEdit(TRUE);
	}
}

