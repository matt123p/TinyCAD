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
#include "option.h"
#include "revision.h"
#include "registry.h"
#include "TinyCadView.h"
#include "TinyCad.h"
#include "MainFrm.h"
#include "diag.h"

CDlgERCListBox theERCListBox;

////// The CEditDlg dialog //////

CEditDlg::CEditDlg()
{
	m_pDesign = NULL;
	m_pObject = NULL;
	opens = 0;

	//{{AFX_DATA_INIT(CEditDlg)
	//}}AFX_DATA_INIT
}

void CEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditDlg)
	//}}AFX_DATA_MAP
}

void CEditDlg::OnCancel()
{	
	// This member is called when edit dialog is in focus and user presses VK_ESCAPE.
	// Process this situation in the same way as what would happen if the main application window would
	// have been in focus and user pressed VK_ESCAPE.

	extern CTinyCadView *g_currentview;

	if (g_currentview) {
		// CTinyCadView maps the VK_ESCAPE to this command
		g_currentview->SendMessage(WM_COMMAND, IDM_EDITEDIT, 0);
	}
}

BEGIN_MESSAGE_MAP(CEditDlg, CDialog)
//{{AFX_MSG_MAP(CEditDlg)
// No message handlers
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

