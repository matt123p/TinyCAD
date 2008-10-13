////////////////////////////////////////////////////////////////
// MSDN Magazine -- October 2002
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with Visual Studio 6.0 on Windows XP. Tab size=3.
//
#ifndef _FOLDERWIN_H
#define _FOLDERWIN_H

#include "ftab.h"
#include "WinMgr.h"

enum {
	ID_WIN_FOLDERTABS=1,
	ID_WIN_SIZERBAR,
	ID_WIN_SIZEBOX,
};


//////////////////
// Window to manage folder tab, view and scroll bars
//
class CFolderFrame : public CWnd {
	DECLARE_DYNAMIC(CFolderFrame)
protected:
	CWinMgr				m_winMgr;				 // window manager
	CFolderTabCtrl		m_wndFolderTabCtrl;	 // folder tab
	CSizerBar		m_wndSizerBar;			 // sizer bar
	CScrollBar		m_wndSBHorz;			 // horizontal scroll bar..
	CScrollBar		m_wndSBVert;			 // ... vertical
	CScrollBar		m_wndSBBox;				 // ... and box
	int				m_cxFolderTabCtrl;	 // width of folder tab
	UINT				m_nIDRes;				 // resource ID for tab names

public:
	CFolderFrame();
	virtual ~CFolderFrame();

	BOOL Create(CWnd* pParent, CRuntimeClass* pViewClass, CCreateContext* pcc,
		UINT nIDRes = 0,
		int cxFolderTabCtrl=-2,
		DWORD dwStyle = WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN);

	CScrollBar* GetScrollBar(int nBar) const;
	CFolderTabCtrl& GetFolderTabCtrl()			 { return m_wndFolderTabCtrl; }
	CWnd* GetView(UINT nID=AFX_IDW_PANE_FIRST) { return GetDlgItem(nID); }
	int	GetFolderTabWidth()						 { return m_cxFolderTabCtrl; }
	enum  { bestFit=-1, hide=-2 };
	void	ShowControls(int width);		 // show ctrls: bestFit, hide, or width

	// virtual overrides
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);

	// message handlers
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg LRESULT OnWinMgr(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
};

//////////////////
// Derive your view from this
//
class CFolderView : public CView {
public:
	CFolderView() { }
	virtual ~CFolderView() { }

	// override to use my own scrollbar controls, not built-in
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
	virtual void OnChangedFolder(int iPage);
	virtual void OnFolderContextMenu();
	CFolderFrame* GetFolderFrame() const;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnChangedTab(NMHDR* nmtab, LRESULT* pRes);
	afx_msg void OnContextMenu(NMHDR* nmtab, LRESULT* pRes);
	DECLARE_DYNAMIC(CFolderView)
};

#endif // _FOLDERWIN_H
