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

// OptionsPropertySheet.h : header file
//
// This class defines custom modal property sheet 
// COptionsPropertySheet.

#ifndef __OPTIONSPROPERTYSHEET_H__
#define __OPTIONSPROPERTYSHEET_H__

#include "OptionsSheets.h"

class CTinyCadDoc;

/////////////////////////////////////////////////////////////////////////////
// COptionsPropertySheet

class COptionsPropertySheet: public CPropertySheet
{
	DECLARE_DYNAMIC( COptionsPropertySheet)

	// Construction
public:
	COptionsPropertySheet(CWnd* pWndParent = NULL);

	// Attributes
public:

	CTinyCadDoc* m_pDocument;

	COptionsGrid m_Page1;
	COptionsAutosnap m_Page2;
	COptionsAutoSave m_Page3;

	// Operations
public:

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionsPropertySheet)
	//}}AFX_VIRTUAL

	// Implementation
public:
	virtual ~COptionsPropertySheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(COptionsPropertySheet)
	// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif	// __OPTIONSPROPERTYSHEET_H__
