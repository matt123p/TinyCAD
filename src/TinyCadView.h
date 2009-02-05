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

#if !defined(AFX_TINYCADVIEW_H__A2D9E56B_4BF2_4F22_98DB_450C36940721__INCLUDED_)
#define AFX_TINYCADVIEW_H__A2D9E56B_4BF2_4F22_98DB_450C36940721__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "resource.h"
#include "option.h"
#include "context.h"
#include "ruler.h"
#include "object.h"
#include "library.h"
#include "MultiSheetDoc.h"
#include "diag.h"
#include "EditToolBar.h"
#include "FolderFrame.h"
#include "drawhierarchicalsymbol.h"

#define STRLEN	256

class Centre;
class CDlgPositionBox;

extern CEditToolbar g_EditToolBar;


class CTinyCadView : public CFolderView
{
protected: // create from serialization only
	CTinyCadView();
	DECLARE_DYNCREATE(CTinyCadView)

	// Have we captured the mouse control?
	BOOL	m_captured;

	// Are we panning with the middle mouse button
	BOOL	m_panning;
	
	// The previous zoom
	double	m_old_zoom_factor;


	UINT	ClipboardFormat;		// The registered clipboard format

	/////////////////////////////////////////////////////////////////////////////
	//

	CStatusBar m_wndStatusBar;	// The status bar associated with this window

	CDPoint 	MousePosition;		// The last known mouse position

	// Change the current offset co-ords
	void SetScroll(double,double,bool first = false);
	// Set a new zoom value
	void ChangeZoomFactor(double);
	void SetZoomFactor(double);

	// The rulers
	Ruler *vRuler;
	Ruler *hRuler;

	BOOL	  m_Printing;
	Transform m_Printing_Transform;	// The cache of the transform whilst printing...
	int		  current_Sheet;		// The cache of the current sheet whilst printing...

	Transform &GetTransform();

	// The printing information
	int	m_rows;
	int m_cols;
	BOOL m_PrintAllSheets;

	HCURSOR m_mouse_pointers[13];

	void SetTabsFromDocument();


// Attributes
public:
	CMultiSheetDoc* GetDocument();
	CTinyCadDoc* GetCurrentDocument();

protected:
	// Our own version of this function, so that we
	// can set portrait/landscape mode automatically
	BOOL DoPreparePrinting(CPrintInfo* pInfo);

	// The user has changed the current folder
	virtual void OnChangedFolder(int iPage);
	virtual void OnFolderContextMenu();


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTinyCadView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTinyCadView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	void SetScrollCentre(CDPoint);



	// Read and write the	// Create a bitmap for off-screen drawing...
	bool CreateBitmap( CDC &dc, int width, int height );

	// The off-screen drawing bitmap...
	BOOL	m_use_offscreen_drawing;
	static CBitmap		m_bitmap;
	static int			m_bitmap_width;
	static int			m_bitmap_height;
	static int			m_max_bitmap_size;
 // registry defaults
	void RegistryInit();
	void RegistryRead();

 	// Get a symbol from the libraries
	void SetLibEdit(CLibraryStore *NewLibEdit);
	
	// Clipboard operators
	afx_msg void OnDestroyClipboard();
	BOOL IsClipboardAvailable();


// Generated message map functions
protected:
	//{{AFX_MSG(CTinyCadView)
	afx_msg void OnUpdateEditedit(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBusback(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBusslash(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolarc(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolbus(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolbusname(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolcircle(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolconnect(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolget(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTooljunc(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToollabel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolHierachical(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolpolygon(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolpower(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolsquare(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTooltext(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolwire(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewcentre(CCmdUI* pCmdUI);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnUpdateEditdrag(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditdup(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditrotate(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditmove(CCmdUI* pCmdUI);
	afx_msg void OnSnaptogrid();
	afx_msg void OnUpdateSnaptogrid(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRepeatnamedown(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRepeatnameup(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRepeatpindown(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRepeatpinup(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditpaste(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditcut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditcopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditRotateLRF(CCmdUI* pCmdUI);
	afx_msg void OnDestroy();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRulerVert();
	afx_msg void OnUpdateRulerVert(CCmdUI* pCmdUI);
	afx_msg void OnRulerHoriz();
	afx_msg void OnUpdateRulerHoriz(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnViewOptions();
	afx_msg void OnContextArcin();
	afx_msg void OnContextArcout();
	afx_msg void OnContextCanceldrawing();
	afx_msg void OnContextFinishdrawing();
	afx_msg void OnContextFreeline();
	afx_msg void OnContextAddhandle();
	afx_msg void OnContextDeletehandle();
	afx_msg void OnContextZorderBringtofront();
	afx_msg void OnContextZorderSendtoback();
	afx_msg void OnUpdateContextZorderBringtofront(CCmdUI* pCmdUI);
	afx_msg void OnUpdateContextZorderSendtoback(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditduplicate(CCmdUI* pCmdUI);
	afx_msg void OnSpecialCreatespicefile();
	afx_msg void OnUpdateEditCopyto(CCmdUI* pCmdUI);
	afx_msg void OnEditCopyto();
	afx_msg void OnContextMakehorizontal();
	afx_msg void OnContextMakevertical();
	afx_msg void OnFileSaveasbitmap();
	afx_msg void OnOptionsColours();
	afx_msg void OnContextReplacesymbol();
	afx_msg void OnEditInsertpicture();
	//}}AFX_MSG

	afx_msg int	 OnCreate( LPCREATESTRUCT );

	// The Help Menu
	afx_msg void OnAbout();

	// The File Menu
	afx_msg void OnFileDesign();
	afx_msg void OnFileImport();
	afx_msg void OnFilePageSet();


	// The Edit Menu
	afx_msg void OnEditUndo() { GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument())); GetCurrentDocument()->Undo(); }
	afx_msg void OnEditRedo() { GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument())); GetCurrentDocument()->Redo(); }
	afx_msg void OnEditLayer();
	afx_msg void OnEditEdit() { GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument())); }
	afx_msg void OnEditDelete();
	afx_msg void OnEditMove() { GetCurrentDocument()->SelectObject(new CDrawBlockMove(GetCurrentDocument())); }
	afx_msg void OnEditDrag() { GetCurrentDocument()->SelectObject(new CDrawBlockDrag(GetCurrentDocument())); }
	afx_msg void OnEditDup() { GetCurrentDocument()->SelectObject(new CDrawBlockDup(GetCurrentDocument())); }
	afx_msg void OnEditRotate() { GetCurrentDocument()->SelectObject(new CDrawBlockRotate(GetCurrentDocument())); }
	afx_msg void OnFindFind();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnEditDuplicate();
	afx_msg void OnEditRotateLeft();
	afx_msg void OnEditRotateRight();
	afx_msg void OnEditFlip();


	// The Keyboard operations
	afx_msg void OnViewZoomIn();
	afx_msg void OnViewZoomOut();
	afx_msg void OnViewCentreX() { SetScrollCentre(MousePosition); }

	// The scrollbar/sizing operations
	afx_msg void OnHScroll(UINT,UINT,CScrollBar *);
	afx_msg void OnVScroll(UINT,UINT,CScrollBar *);

   	// The Toolbar Menu
	afx_msg void OnSelectWire() { GetCurrentDocument()->SelectObject(new CDrawLine(GetCurrentDocument(),xWire)); }
	afx_msg void OnSelectBus() { GetCurrentDocument()->SelectObject(new CDrawLine(GetCurrentDocument(),xBus)); }
	afx_msg void OnSelectBusSlash() { GetCurrentDocument()->SelectObject(new CDrawBusSlash(GetCurrentDocument(),0)); }
	afx_msg void OnSelectBusBack() { GetCurrentDocument()->SelectObject(new CDrawBusSlash(GetCurrentDocument(),1)); }
	afx_msg void OnSelectBusName() { GetCurrentDocument()->SelectObject(new CDrawText(GetCurrentDocument(),xBusNameEx)); }
	afx_msg void OnSelectJunction();
	afx_msg void OnSelectPower() { GetCurrentDocument()->SelectObject(new CDrawPower(GetCurrentDocument())); }
	afx_msg void OnSelectLabel() { GetCurrentDocument()->SelectObject(new CDrawLabel(GetCurrentDocument())); }
	afx_msg void OnSelectHierachical();
	afx_msg void OnSelectConnect() { GetCurrentDocument()->SelectObject(new CDrawNoConnect(GetCurrentDocument())); }
	afx_msg void OnSelectPolygon();
	afx_msg void OnSelectArc();
	afx_msg void OnSelectSquare() { GetCurrentDocument()->SelectObject(new CDrawSquare(GetCurrentDocument(),xSquareEx3)); }
	afx_msg void OnSelectCircle() { GetCurrentDocument()->SelectObject(new CDrawSquare(GetCurrentDocument(),xCircleEx3)); }
	afx_msg void OnSelectText() { GetCurrentDocument()->SelectObject(new CDrawText(GetCurrentDocument(),xTextEx2)); }
	afx_msg void OnViewCentre() { GetCurrentDocument()->SelectObject(new CDrawCentre(GetCurrentDocument())); }
	afx_msg void OnSelectGet();

	// The Special Menu
	afx_msg void OnSpecialAnotate();
	afx_msg void OnSpecialBom();
	afx_msg void OnSpecialNet();
	afx_msg void OnSpecialCheck();

	// The Repeat Menu
	afx_msg void OnRepeatNameUp();
	afx_msg void OnRepeatNameDown();
	afx_msg void OnRepeatPinUp();
	afx_msg void OnRepeatPinDown();


	// Tracking mouse movements
	afx_msg void OnMouseMove(UINT nFlags, CPoint p);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint p);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint p);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint p);
	afx_msg void OnMButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnMButtonUp( UINT nFlags, CPoint point );


	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnContextOpendesign();
	afx_msg void OnContextReloadsymbolfromdesign();
	void SelectSheet(int sheet);
	void ChangeDir(int dir);
};

inline CMultiSheetDoc* CTinyCadView::GetDocument()
   { return (CMultiSheetDoc*)m_pDocument; }

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TINYCADVIEW_H__A2D9E56B_4BF2_4F22_98DB_450C36940721__INCLUDED_)
