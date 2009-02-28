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
#include <assert.h>
#include "TinyCadView.h"
#include "registry.h"
#include "TinyCad.h"
#include "LibraryDb.h"
#include "LibrarySQLite.h"
#include "DlgLibraryBox.h"
#include "LibraryCollection.h"
#include ".\dlglibrarybox.h"

//*************************************************************************
//*                                                                       *
//* The dialog for editing which libraries are in use                     *
//*                                                                       *
//*************************************************************************

BEGIN_MESSAGE_MAP( CDlgLibraryBox, CDialog )
	//{{AFX_MSG_MAP(CDlgLibraryBox)
	ON_COMMAND( LIBRARYBOX_ADD, OnAdd )
	ON_COMMAND( LIBRARYBOX_REMOVE, OnRemove )
	ON_COMMAND( LIBRARYBOX_EDIT, OnEdit )
	ON_LBN_DBLCLK(LIBRARYBOX_AVAILABLE, OnDblclkAvailable)
	ON_LBN_SELCHANGE(LIBRARYBOX_AVAILABLE, OnSelchangeAvailable)
	ON_BN_CLICKED(LIBRARYBOX_NEW, OnNew)
	ON_BN_CLICKED(LIBRARYBOX_UPGRADE, OnUpgrade)
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
END_MESSAGE_MAP()

//=========================================================================
//== ctor/dtor/initializing                                              ==
//=========================================================================

//-------------------------------------------------------------------------
CDlgLibraryBox::CDlgLibraryBox(CWnd* pParentWnd)
: CDialog( IDD_LIBRARY, pParentWnd )
{
	//{{AFX_DATA_INIT(CDlgLibraryBox)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lstLibs = NULL;
}
//-------------------------------------------------------------------------
//-- Creating and filling listbox
BOOL CDlgLibraryBox::OnInitDialog()
{
	// Set up the list box with the current library names in it
	m_lstLibs = (CListBox*) GetDlgItem( LIBRARYBOX_AVAILABLE );

	// Get the position of the buttons
	CRect client, button, list;
	GetClientRect( client );
	GetDlgItem( LIBRARYBOX_ADD )->GetWindowRect( button );
	ScreenToClient( button );

	m_lstLibs->GetWindowRect( list );
	ScreenToClient( list );

	m_button_pos = client.right - button.left;
	m_list_pos = client.right - list.right;
	m_list_bottom_border = client.bottom - list.bottom;

	RefreshComponents( true );

	return TRUE;
}
//-------------------------------------------------------------------------

//==========================================================================
//== accessor                                                             ==
//==========================================================================

//-------------------------------------------------------------------------
//-- Returns a reference on the current selected listbox item,
//-- otherwise NULL.
CLibraryStore* CDlgLibraryBox::GetSelectedLibrary() const
{
	CLibraryStore*	oReturn		= NULL;
	int				nSelIndex	= m_lstLibs->GetCurSel();
	CString			sLibName;

	// Get the item which has been selected
	if( nSelIndex != LB_ERR )
	{
		// Now calculate which library this is
		m_lstLibs->GetText( nSelIndex, sLibName );

		oReturn = CLibraryCollection::GetLibrary( sLibName );
	}

	return oReturn;
}
//-------------------------------------------------------------------------

//=========================================================================
//== mutator                                                             ==
//=========================================================================

//-------------------------------------------------------------------------
//-- Controls En/Disabling of some buttons of the dialog
//-- Parameter bHasToRefill forces redrawing of listbox
void CDlgLibraryBox::RefreshComponents( bool bHasToRefill )
{
	CButton*	btnRemove		= static_cast<CButton*>(GetDlgItem( LIBRARYBOX_REMOVE ));
	CButton*	btnEdit			= static_cast<CButton*>(GetDlgItem( LIBRARYBOX_EDIT ));
	CButton*	btnUpgrade		= static_cast<CButton*>(GetDlgItem( LIBRARYBOX_UPGRADE ));
	BOOL		bIsLibSelected	= m_lstLibs->GetCurSel() != LB_ERR;

	if( bHasToRefill )
	{
		CLibraryCollection::FillLibraryNames( m_lstLibs );

		// refilling unselects the listbox
		bIsLibSelected = false;
	}

	// activ if any listbox entry is selected
	btnRemove->EnableWindow( bIsLibSelected );
	btnEdit->EnableWindow( bIsLibSelected );
	btnUpgrade->EnableWindow( bIsLibSelected );
}
//-------------------------------------------------------------------------


//=========================================================================
//== message handling                                                    ==
//=========================================================================

//-------------------------------------------------------------------------
//-- Closes dialog and saves library pathes to registry
void CDlgLibraryBox::OnOK()
{
	CLibraryCollection::SaveToRegistry();
	EndDialog( IDOK );
}
//-------------------------------------------------------------------------
//-- Proofs vality of a library
//-- Adds it to collection and listbox
void CDlgLibraryBox::OnAdd()
{
  	TCHAR szFile[256];

  	_tcscpy_s(szFile, _T("*.tclib;*.mdb;*.idx\0"));

	CFileDialog dlg( TRUE, _T("*.tclib"), szFile, OFN_HIDEREADONLY,
		_T("Library file (*.tclib, *.mdb, *idx)|*.tclib;*.mdb;*.idx|All files (*.*)|*.*||"), AfxGetMainWnd() ); 


  	if( dlg.DoModal() == IDOK )
	{
		_tcscpy_s( szFile, dlg.GetPathName() );

  		// Remove the extension
		TCHAR *brk = _tcsrchr( szFile, '.');
		if (brk)
		{
			*brk = 0;
		}
		else
		{
			brk = szFile;
		}

  		// Create the library
  		CLibraryStore* NewLib;

  		if (_tcsicmp(brk+1,_T("idx")) == 0)
  		{
			NewLib = new CLibraryFile;
  		}
  		else if (_tcsicmp(brk+1,_T("mdb")) == 0)
  		{
			NewLib = new CLibraryDb;
  		}
  		else
  		{
			NewLib = new CLibrarySQLite;
  		}

  		NewLib->Attach( szFile );

		// Now add it to the library linked list
		if( ! CLibraryCollection::Contains( NewLib ) )
		{
			CLibraryCollection::Add( NewLib );
			RefreshComponents( true );

			// Finally update the symbol picker...
			CTinyCadApp::ResetAllSymbols();
		}
	}
}
//-------------------------------------------------------------------------
//-- Removes a library from collection and listbox
void CDlgLibraryBox::OnRemove()
{
	int				nSelIndex = m_lstLibs->GetCurSel();
	CString			sLibName;
	CLibraryStore*	pLib;

	// Get the item which has been selected
	if( nSelIndex != LB_ERR  )
	{
		// Now calculate which library this is
		m_lstLibs->GetText( nSelIndex, sLibName );

		pLib = CLibraryCollection::GetLibrary( sLibName );

		// listbox and collection should be synchron
		assert( pLib != NULL );

		if( CTinyCadApp::IsLibInUse(pLib) )
		{
			// We cannot delete whilst in use
			AfxMessageBox( IDS_NODELETE );
		}
		else
		{
			CLibraryCollection::Remove( pLib );
			RefreshComponents( true );

			// Finally update the symbol picker...
			CTinyCadApp::ResetAllSymbols();
		}
	}
}
//-------------------------------------------------------------------------
//-- Opens a thumbnail view of the selected library
void CDlgLibraryBox::OnEdit()
{
	CLibraryStore* pLib = GetSelectedLibrary();

	// Should be impossible to edit a non-selected library
	assert( pLib != NULL );

	// First can this library be upgraded?
	if (pLib->MustUpgrade())
	{
		if (AfxMessageBox(IDS_MUSTUPGRADE, MB_YESNO ) != IDYES)
		{
			return;
		}

		pLib = CLibraryCollection::Upgrade(pLib);
		if (!pLib)
		{
			return;
		}
	}

	CTinyCadApp::EditLibrary( pLib );
	CLibraryCollection::SaveToRegistry();
	EndDialog( IDOK );
}
//-------------------------------------------------------------------------
//--
void CDlgLibraryBox::OnNew()
{
  	TCHAR szFile[256];

  	_tcscpy_s(szFile, _T("*.tclib\0"));

	CFileDialog dlg( FALSE, _T("*.tclib"), szFile, OFN_HIDEREADONLY,
		_T("Library database (*.tclib)|*.tclib|All files (*.*)|*.*||"), AfxGetMainWnd() ); 

    if( dlg.DoModal() == IDOK )
  	{
	  	// Remove the extension
		_tcscpy_s( szFile, dlg.GetPathName() );
		TCHAR *brk = _tcsrchr( szFile, '.');
		if (brk)
		{
			*brk = 0;
		}
		else
		{
			brk = szFile;
		}

  		// Create the library object
  		CLibraryStore* NewLib;
  		if (_tcsicmp(brk+1,_T("mdb")) == 0)
  		{
			// When you explicitly ask for an MDB, you get an MDB!
  			NewLib = new CLibraryDb;
  		}
		else
		{
	  		NewLib = new CLibrarySQLite;
		}

	  	// .. and create the file
	  	if( NewLib->Create(szFile) )
	  	{
	  		// Now add it to the library linked list
	  		CLibraryCollection::Add( NewLib );

	  		// .. and to the list box
			RefreshComponents( true );
	  	}
	  	else
	  	{
		  	delete NewLib;
		  	return;
	  	}
	}
}


//-------------------------------------------------------------------------
//-- Converts a library from the old format to
//-- the new Microsoft Access database format.
void CDlgLibraryBox::OnUpgrade()
{
	CLibraryStore* pOldLib = GetSelectedLibrary();
	CLibraryCollection::Upgrade( pOldLib );
}
//-------------------------------------------------------------------------
//-- single click on listbox refreshes it
void CDlgLibraryBox::OnSelchangeAvailable()
{
	RefreshComponents( false );
}
//-------------------------------------------------------------------------
//-- double click opens a thumbnail view of the selected library
void CDlgLibraryBox::OnDblclkAvailable()
{
	OnEdit();
}
//-------------------------------------------------------------------------


void CDlgLibraryBox::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// Move the list box into position
	if (m_lstLibs)
	{
		CRect r;
		m_lstLibs->GetWindowRect( r );
		ScreenToClient( r );
		r.right = cx - m_list_pos;
		r.bottom = cy - m_list_bottom_border;
		m_lstLibs->MoveWindow( r );
	}

	const int ids[] = {
		LIBRARYBOX_ADD, LIBRARYBOX_REMOVE, LIBRARYBOX_EDIT,	
		LIBRARYBOX_NEW,	LIBRARYBOX_UPGRADE, IDOK };

	for (int i = 0; i < sizeof(ids) / sizeof(int); ++i)
	{
		CWnd *pButton = GetDlgItem( ids[i] );
		if (pButton)
		{
			CRect r;
			pButton->GetWindowRect(r);
			ScreenToClient(r);
			r += CPoint( (cx - m_button_pos) - r.left, 0 );
			pButton->MoveWindow( r );
		}
	}
}
