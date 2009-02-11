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


// EditToolbar.cpp : implementation file
//

#include "stdafx.h"
#include "tinycad.h"

#include "TinyCadDoc.h"
#include "TinyCadView.h"

#include "EditToolbar.h"


/////////////////////////////////////////////////////////////////////////////
// CEditToolbar

CEditToolbar::CEditToolbar()
{
	m_pCurrentTool = NULL;
}

CEditToolbar::~CEditToolbar()
{
}


BEGIN_MESSAGE_MAP(CEditToolbar, CWnd)
	//{{AFX_MSG_MAP(CEditToolbar)
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CEditToolbar message handlers

void CEditToolbar::Create(CWnd *pParent)
{
	CString s = AfxRegisterWndClass( CS_NOCLOSE, NULL, NULL, NULL );
	CRect rect;
	pParent->GetWindowRect( rect );
	rect.top += 80;
	rect.left = rect.right - 240;
	CWnd::CreateEx( WS_EX_TOOLWINDOW, s, _T("Tool"), WS_POPUP | WS_BORDER | WS_CAPTION, rect, pParent, NULL );

	m_NoTool.Create();
	m_TextEdit.Create();
	m_LabelEdit.Create();
	m_PowerEdit.Create();
	m_PinEdit.Create();
	m_DrawLineEdit.Create();
	m_DrawPolyEdit.Create();
	m_MethodEdit.Create();
	m_RotateBox.Create();
	m_LineEdit.Create();
	m_PolygonEdit.Create();
	m_HierarchicalEdit.Create();

	setDlg( &m_NoTool );
	ShowWindow( SW_SHOW );
}

void CEditToolbar::setDlg(CEditDlg *pWnd)
{
	CRect rect(0,0,0,0);

	if (pWnd)
	{
		pWnd->GetWindowRect( rect );
	}
	else
	{
		m_NoTool.GetWindowRect( rect );
	}

	// Move ourselves
	CalcWindowRect( rect, 0 );
	SetWindowPos( NULL, 0,0, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );

	// Now make sure the dlg is show and in the correct position
	if (pWnd)
	{
		if (pWnd->getObject()) 
		{
			pWnd->GetParent()->SetWindowText(pWnd->getObject()->GetName() + " Tool Options");
		}
		else
		{
			pWnd->GetParent()->SetWindowText(_T("Tool"));
		}
		pWnd->ShowWindow( SW_SHOWNA );
		pWnd->SetWindowPos( &wndTop, 0,0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE );
	}

	if (pWnd != &m_NoTool && m_NoTool.m_hWnd != NULL)
	{
		m_NoTool.ShowWindow( SW_HIDE );
	}

	m_pCurrentTool = pWnd;
}

void CEditToolbar::unsetDlg()
{
	setDlg( &m_NoTool );
	m_NoTool.GetParentFrame()->SetFocus();
}

void CEditToolbar::OnSetFocus(CWnd* pOldWnd) 
{

	CWnd::OnSetFocus(pOldWnd);
}


void CEditToolbar::changeSelected( CDrawingObject *previous, CDrawingObject *pObject )
{
	if (m_pCurrentTool)
	{
		m_pCurrentTool->changeSelected( previous, pObject );
	}
}
