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

#pragma once

//*************************************************************************
//*                                                                       *
//* The dialog for editing which libraries are in use.                    *
//*                                                                       *
//* Together with the CLibraryCollection class this serves as a           *
//* primitive MVC(model-view-controller) design pattern.                  *
//*                                                                       *
//* Model:       the instance of CLibrayCollection::instance              *
//* View:        this class                                               *
//* Controller:  this class                                               *
//*                                                                       *
//*************************************************************************

class CDlgLibraryBox: public CDialog
{
private:
	//=====================================================================
	//== instance variables                                              ==
	//=====================================================================
	//-- Listbox showing all the names of libraries stored in
	//-- CLibraryCollection singleton
	CListBox* m_lstLibs;

	// The position of the buttons relative to the right hand
	// border
	int m_button_pos;

	// The position of the list box's right hand side relative to the
	// right hand side of the border
	int m_list_pos;
	int m_list_bottom_border;

	//{{AFX_DATA(CDlgLibraryBox)
	enum
	{
		IDD = IDD_LIBRARY
	};
	//}}AFX_DATA

public:
	//=====================================================================
	//== ctor/dtor/initializing                                          ==
	//=====================================================================
	CDlgLibraryBox(CWnd* pParentWnd);

private:
	//=====================================================================
	//== accessor                                                        ==
	//=====================================================================
	//-- Returns a reference on the current selected listbox item,
	//-- otherwise NULL.
	CLibraryStore* GetSelectedLibrary() const;

	//=====================================================================
	//== mutator                                                         ==
	//=====================================================================
	//-- Controls En/Disabling of some buttons of the dialog
	//-- Parameter bHasToRefill forces redrawing of listbox
	void RefreshComponents(bool bHasToRefill);

protected:
	//=====================================================================
	//== message handling                                                ==
	//=====================================================================
	//{{AFX_MSG(CDlgLibraryBox)

	//-- Closes dialog and saves library pathes to registry
	afx_msg void		OnOK();

	//-- Creating and filling listbox
	afx_msg BOOL 		OnInitDialog();

	//-- Proofs vality of a library
	//-- Adds it to collection and listbox
	afx_msg void 		OnAdd();

	//-- Removes a library from collection and listbox
	afx_msg void 		OnRemove();

	//-- Opens a thumbnail view of the selected library
	afx_msg void 		OnEdit();

	//--
	afx_msg void 		OnNew();

	//-- Converts a library from the old format to
	//-- the new Microsoft Access database format.
	afx_msg void 		OnUpgrade();

	//-- single click on listbox refreshes it
	afx_msg void 		OnSelchangeAvailable();

	//-- double click opens a thumbnail view of the selected library
	afx_msg void 		OnDblclkAvailable();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
//=========================================================================
