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
#include "TinyCAD.h"
#include "LibraryStore.h"
#include "TinyCadDoc.h"
#include "TinyCadMultiSymbolDoc.h"



// The constructor
CLibraryStore::CLibraryStore()
{
  // Default the name
  m_name="Library";
}

// The destructor
CLibraryStore::~CLibraryStore()
{
}

// Extract a symbol from this library
CLibraryStoreNameSet *CLibraryStore::Extract(const TCHAR *key)
{
    CLibraryStoreNameSet *temp=NULL;
  
	symbolCollection::iterator it = m_Symbols.begin();

	while (it != m_Symbols.end())
	{
		CLibraryStoreNameSet *pSymbol = &(it->second);

		// Do this for each of the names in the symbol set
		for (int i =0; i < pSymbol->GetNumRecords(); i++)
		{
			CSymbolRecord &r = pSymbol->GetRecord( i );

			CString name = r.name;

			if (name.CompareNoCase( key ) == 0)
			{
				return pSymbol;
			}
		}

		++ it;
	}


	return NULL;
}

// Find a string in the symbol names
void CLibraryStore::Find(const TCHAR *theString,CListBox *theListBox)
{
	symbolCollection::iterator it = m_Symbols.begin();

	while (it != m_Symbols.end())
	{
		CLibraryStoreNameSet *pSymbol = &(it->second);

		// Do this for each of the names in the symbol set
		for (int i =0; i < pSymbol->GetNumRecords(); i++)
		{
			CLibraryStoreSymbol &r = pSymbol->GetRecord( i );

			CString name = r.name;
			CString desc = r.description;
			name.MakeLower();
			desc.MakeLower();
			if (	name.Find(theString) != -1
				||	desc.Find(theString) != -1)
			{
				int index = theListBox->AddString( r.name + " - " + r.description );
				theListBox->SetItemDataPtr(index, &r );
			}
		}

		++ it;
	}
}


// Check to see if the item exists in this library
BOOL CLibraryStore::DoesExist( CLibraryStoreNameSet *pSymbol )
{
	symbolCollection::iterator it = m_Symbols.begin();

	while (it != m_Symbols.end())
	{
		if (pSymbol == &(it->second))
		{
			return TRUE;
		}
		++ it;
	}

	return FALSE;
}



// Re-read the library
void CLibraryStore::ReRead()
{
  // Get rid of the hash table
  m_Symbols.erase( m_Symbols.begin(), m_Symbols.end() );

  // Re-read the symbol table in
  Attach(m_name);

  // Inform the document views
  static_cast<CTinyCadApp*>(AfxGetApp())->ResetAllSymbols();
}


// Write this library to an XML file
void CLibraryStore::SaveXML( const TCHAR *filename, int id )
{
	// First open the stream to save to
  	CFile theFile;

  	// Open the file for saving as a CFile for a CArchive
  	BOOL r =  theFile.Open(filename, CFile::modeCreate | CFile::modeWrite);

	if (r) 
	{
		// Create the XML stream writer
		CStreamFile stream( &theFile, CArchive::store );
		CXMLWriter xml( &stream );

		CString comment;
		comment.Format( _T("This file was written by TinyCAD %s %s\n")
						_T("If you wish to view this file go to http://tinycad.sourceforge.net to\n")
						_T("download a copy."),
						CTinyCadApp::GetVersion(),
						CTinyCadApp::GetReleaseType());

		xml.addComment( comment );

		xml.addTag(_T("Library"));

		// Now write the library out..
		symbolCollection::iterator it = m_Symbols.begin();

		while (it != m_Symbols.end())
		{
			if (id == -1 || id == it->first)
			{
				CLibraryStoreNameSet &symbol = it->second;
				CTinyCadMultiSymbolDoc temp_doc( this, symbol );

			
				xml.addTag(_T("SYMBOL"));
				
				// Save out the name-set details
				symbol.SaveXML( xml );

				// Now the symbol details
				temp_doc.SaveXML( xml );

				xml.closeTag();
			}

			++ it;
		}

		xml.closeTag();

	}
}

// Import library symbols from an XML file
void CLibraryStore::LoadXML( const TCHAR *filename )
{
	// First open the stream to save to
  	CFile theFile;

  	// Open the file for saving as a CFile for a CArchive
  	BOOL r =  theFile.Open(filename, CFile::modeRead);

	if (r) 
	{
		CString name;

		// Create the XML stream writer
		CStreamFile stream( &theFile, CArchive::load );
		CXMLReader xml( &stream );

		// Get the library tag
		xml.nextTag( name );
		if (name != "Library")
		{
			Message(IDS_ABORTVERSION,MB_ICONEXCLAMATION);
			return;
		}
		xml.intoTag();

		CTinyCadApp::SetLockOutSymbolRedraw( true );
		while (	xml.nextTag( name ) )
		{
			// Is this a symbol?
			if (name == "SYMBOL")
			{
				// Load in the details
				xml.intoTag();
				CTinyCadMultiSymbolDoc temp_doc;
				drawingCollection drawing;
				CLibraryStoreNameSet s;
				s.LoadXML( &temp_doc, xml );
				xml.outofTag();

				// ... and store the symbol
				Store( &s, temp_doc );
			}
		}
		xml.outofTag();

		CTinyCadApp::SetLockOutSymbolRedraw( false );

	}
}
