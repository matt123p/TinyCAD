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

// DetailsPropertySheet.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "DetailsPropertySheet.h"


/////////////////////////////////////////////////////////////////////////////
// CDetailsPropertySheet

IMPLEMENT_DYNAMIC(CDetailsPropertySheet, CPropertySheet)

CDetailsPropertySheet::CDetailsPropertySheet(CMultiSheetDoc* pDesign, CWnd* pWndParent)
	 : CPropertySheet(IDS_PROPSHT_CAPTION1, pWndParent)
	 , m_pDesign( pDesign )
	 , m_Page1( pDesign )
	 , m_Page2( pDesign )
{
	// Add all of the property pages here.  Note that
	// the order that they appear in here will be
	// the order they appear in on screen.  By default,
	// the first page of the set is the active one.
	// One way to make a different property page the 
	// active one is to call SetActivePage().

	AddPage(&m_Page1);
	AddPage(&m_Page2);
}

CDetailsPropertySheet::~CDetailsPropertySheet()
{
}


BEGIN_MESSAGE_MAP(CDetailsPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CDetailsPropertySheet)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDetailsPropertySheet message handlers


