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

#if !defined(AFX_EDITTOOLBAR_H__02626473_7EE0_46E6_925C_DD858053CCD3__INCLUDED_)
#define AFX_EDITTOOLBAR_H__02626473_7EE0_46E6_925C_DD858053CCD3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditToolbar.h : header file
//

#include "diag.h"
#include "EditDlgMethodEdit.h"
#include "EditDlgLine.h"
#include "EditDlgPolygon.h"
#include "EditDlgTextEdit.h"
#include "EditDlgLabelEdit.h"
#include "EditDlgDrawLineEdit.h"
#include "EditDlgPinEdit.h"
#include "EditDlgDrawPolyEdit.h"
#include "EditDlgHierarchicalEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CEditToolbar window

class CEditToolbar: public CWnd
{
	// Construction
public:
	CEditToolbar();

	// Attributes
public:

	// Operations
public:

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditToolbar)
	//}}AFX_VIRTUAL


	// Implementation
public:
	void unsetDlg();
	void setDlg(CEditDlg *pWnd);
	void Create(CWnd *pParent);
	virtual ~CEditToolbar();

	CEditDlg* m_pCurrentTool;

	CEditDlgNoTool m_NoTool;
	CEditDlgTextEdit m_TextEdit;
	CEditDlgLabelEdit m_LabelEdit;
	CEditDlgPowerEdit m_PowerEdit;
	CEditDlgPinEdit m_PinEdit;
	CEditDlgDrawLineEdit m_DrawLineEdit;
	CEditDlgDrawPolyEdit m_DrawPolyEdit;
	CEditDlgMethodEdit m_MethodEdit;
	CEditDlgRotateBox m_RotateBox;
	CEditDlgLine m_LineEdit;
	CEditDlgPolygon m_PolygonEdit;
	CEditDlgHierarchicalEdit m_HierarchicalEdit;

	void changeSelected(CDrawingObject *previous, CDrawingObject *pObject);

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditToolbar)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CEditToolbar g_EditToolBar;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before m_ previous line.

#endif // !defined(AFX_EDITTOOLBAR_H__02626473_7EE0_46E6_925C_DD858053CCD3__INCLUDED_)
