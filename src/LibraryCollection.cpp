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
#include "LibraryCollection.h"
#include "registry.h"
#include "TinyCad.h"
#include "LibrarySQLite.h"
#include <assert.h>

typedef std::list<CLibraryStore*>::iterator		TIterator;

//*************************************************************************
//*                                                                       *
//*  Stores all libraries as pointers to CLibraryStore objects            *
//*  in one singleton.                                                    *
//*                                                                       *
//*  The pointer handling is a asynchron. They were created outside this  *
//*  class and passed to the Add method, the but removed in the           *
//*  Delete and Clear methods.                                            *
//*                                                                       *
//*************************************************************************

//=========================================================================
//== init class variables                                                ==
//=========================================================================
CLibraryCollection CLibraryCollection::m_oInstance;
std::list<CLibraryStore*> CLibraryCollection::m_colLibs;

//=========================================================================
//== mutator                                                             ==
//=========================================================================

//-------------------------------------------------------------------------
//-- Removes all library objects (and deletes all pointers)
void CLibraryCollection::Clear()
{
	for( TIterator i = m_colLibs.begin(); i != m_colLibs.end(); i++ )
	{
		delete *i;
	}

	m_colLibs.clear();

	assert( m_colLibs.empty() );
}
//-------------------------------------------------------------------------
//-- Add library
void CLibraryCollection::Add( CLibraryStore* pLib )
{
	assert( pLib != NULL );

	m_colLibs.push_back( pLib );

	assert( ! m_colLibs.empty() );
}
//-------------------------------------------------------------------------
//-- Removes one library(and deletes its pointer)
void CLibraryCollection::Remove( CLibraryStore* pLib )
{
	assert( pLib != NULL );
	assert( ! m_colLibs.empty() );

	m_colLibs.remove( pLib );
	delete pLib;
}
//-------------------------------------------------------------------------
//-- Doing some idle tasks thru calling all the idle time
//-- processing of the libraries
void CLibraryCollection::DoIdle()
{
	for( TIterator i = m_colLibs.begin(); i != m_colLibs.end(); i++ )
	{
		(*i)->OnIdle();
	}
}
//-------------------------------------------------------------------------

//=========================================================================
//== accessor                                                            ==
//=========================================================================

//-------------------------------------------------------------------------
//-- Returns true if this library exist in the collection
bool CLibraryCollection::Contains( CLibraryStore* pLib )
{
	assert( pLib != NULL );

	bool bReturn = false;

	for( TIterator i = m_colLibs.begin(); i != m_colLibs.end(); i++ )
	{
		bReturn |= (pLib->m_name == (*i)->m_name);
	}

	return bReturn;
}
//-------------------------------------------------------------------------
//-- Returns true if this symbol exist in any library
bool CLibraryCollection::ContainsSymbol( CLibraryStoreNameSet* pSymbol )
{
	assert( pSymbol != NULL );

	bool 			bReturn = false;
	CLibraryStore*	pLib;

	for( TIterator i = m_colLibs.begin(); i != m_colLibs.end(); i++ )
	{
		pLib = (*i);

		//BETTER: bReturn |= pLib->Contains( pSymbol );
		bReturn |= (pLib->DoesExist( pSymbol ) == TRUE);
	}

	return bReturn;
}
//-------------------------------------------------------------------------
//-- Returns the library with this name
CLibraryStore* CLibraryCollection::GetLibrary( CString sName )
{
	assert( ! sName.IsEmpty() );

	CLibraryStore*	pReturn = NULL;

	for( TIterator i = m_colLibs.begin(); i != m_colLibs.end(); i++ )
	{
		if(	sName == (*i)->m_name )
		{
			pReturn = *i;
			break;
		}
	}

	return pReturn;
}
//-------------------------------------------------------------------------
//-- Returns a symbol identified by its name, searching in all libraries
CLibraryStoreNameSet* CLibraryCollection::GetSymbol( CString sKey )
{
	assert( ! sKey.IsEmpty() );

	CLibraryStoreNameSet* pReturn = NULL;

	for( TIterator i = m_colLibs.begin(); i != m_colLibs.end(); i++ )
	{
		pReturn = (*i)->Extract( sKey );

		if( pReturn != NULL )
		{
			break;
		}
	}

	return pReturn;
}
//-------------------------------------------------------------------------
//-- Fills listbox entries with library names
void CLibraryCollection::FillLibraryNames( CListBox* lstLib )
{
	lstLib->ResetContent();

	int library_id = 0;
	for( TIterator i = m_colLibs.begin(); i != m_colLibs.end(); i++ )
	{
		int index = lstLib->AddString( (*i)->m_name );
		lstLib->SetItemData( index, library_id );
		++ library_id;
	}
}
//-------------------------------------------------------------------------
//-- Fills listbox entries with library names
void CLibraryCollection::FillLibraryNames( int start_id, CMenu* menuLib )
{
	int library_id = start_id;
	int pos = 0;
	
	for( TIterator i = m_colLibs.begin(); i != m_colLibs.end(); i++ )
	{
		menuLib->InsertMenu( pos, MF_BYPOSITION|MF_STRING, library_id,  (*i)->m_name );
		++ library_id;
		++ pos;
	}
}

//-------------------------------------------------------------------------
//-- Fills listbox entries with library names
CLibraryStore* CLibraryCollection::GetLibraryByIndex( int library_id )
{
	TIterator lib_it = m_colLibs.begin();
	while (library_id > 0 && lib_it != m_colLibs.end()) 
	{
		++ lib_it;
		-- library_id;
	}

	if (lib_it != m_colLibs.end())
	{
		return *lib_it;
	}
	
	return NULL;
}

//-------------------------------------------------------------------------
//-- Fills listbox with all symbols(of all libraries), whoose symbol name
//-- matches the search string
void CLibraryCollection::FillMatchingSymbols( CListBox* lstSymbols, CString sSearch, CListBox* mask  )
{
	CLibraryStore* pLib;

	lstSymbols->ResetContent();

	int index = 0;
	for( TIterator i = m_colLibs.begin(); i != m_colLibs.end(); i++ )
	{
		pLib = *i;
		
		bool use = true;
		if (mask)
		{
			for (int j = 0; j < mask->GetCount(); j ++)
			{
				if (mask->GetItemData( j ) == index)
				{
					use = mask->GetSel( j ) != 0;
					break;
				}
			}
		}

		if (use)
		{
			pLib->Find( sSearch, lstSymbols );
		}

		++ index;
	}
}
//-------------------------------------------------------------------------
//-- Write the all library file pathes back to the registry
void CLibraryCollection::SaveToRegistry()
{
	CString	sLibs = "";

	for( TIterator i = m_colLibs.begin(); i != m_colLibs.end(); i++ )
	{
		if( ! sLibs.IsEmpty() )
		{
			sLibs += ",";
		}

		sLibs += (*i)->m_name;
	}

	CRegistry::Set( "Libraries", sLibs );
}
//-------------------------------------------------------------------------
//-- Converts a library from the old format to
//-- the new Microsoft Access database format.
CLibraryStore* CLibraryCollection::Upgrade( CLibraryStore* pOldLib )
{
	if( CTinyCadApp::IsLibInUse(pOldLib) )
	{
		// We cannot upgrade whilst in use
		AfxMessageBox( IDS_NOUPGRADE );
	}
	else
	{
		CLibraryStore* pNewLib = new CLibrarySQLite;

		if( pOldLib->Upgrade( pNewLib ) )
		{
			// First remove the old library from this system
			Remove( pOldLib );

			// Now add the new library
			Add( pNewLib );

			// Finally update the symbol picker...
			CTinyCadApp::ResetAllSymbols();

			return pNewLib;
		}
		else
		{
			// Upgrade failed
			delete pNewLib;
		}

	}

	return NULL;
}
//-------------------------------------------------------------------------
