/*
 TinyCAD program for schematic capture
 Copyright 1994/1995/2002-2005 Matt Pyne.

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

// DlgRenameSheet.cpp : implementation file
//

#include "stdafx.h"
#include "tinycad.h"
#include "DlgRenameSheet.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgRenameSheet dialog


CDlgRenameSheet::CDlgRenameSheet(CWnd* pParent /*=NULL*/) :
	CDialog(CDlgRenameSheet::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgRenameSheet)
	m_Name = _T("");
	//}}AFX_DATA_INIT
}

void CDlgRenameSheet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgRenameSheet)
	DDX_Text(pDX, ID_RENAME_STRING, m_Name);
	DDV_MaxChars(pDX, m_Name, 32);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgRenameSheet, CDialog)
//{{AFX_MSG_MAP(CDlgRenameSheet)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgRenameSheet message handlers
