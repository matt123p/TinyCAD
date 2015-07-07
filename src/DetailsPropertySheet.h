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

// DetailsPropertySheet.h : header file
//
// This class defines custom modal property sheet 
// CDetailsPropertySheet.

#ifndef __DETAILSPROPERTYSHEET_H__
#define __DETAILSPROPERTYSHEET_H__

class CMultiSheetDoc;

#include "DetailsPropertyPages.h"

/////////////////////////////////////////////////////////////////////////////
// CDetailsPropertySheet

class CDetailsPropertySheet: public CPropertySheet
{
	DECLARE_DYNAMIC( CDetailsPropertySheet)
private:
	CMultiSheetDoc* m_pDesign;

	// Construction
public:
	CDetailsPropertySheet(CMultiSheetDoc* pDesign, CWnd* pWndParent = NULL);

	// Attributes
public:
	CDetailsPropertyPage1 m_Page1;
	CDetailsPropertyPage2 m_Page2;

	// Operations
public:

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDetailsPropertySheet)
	//}}AFX_VIRTUAL

	// Implementation
public:
	virtual ~CDetailsPropertySheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDetailsPropertySheet)
	// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif	// __DETAILSPROPERTYSHEET_H__
