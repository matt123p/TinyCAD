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
#include "TinyCadView.h"
#include "registry.h"
#include "TinyCad.h"
#include "TinyCadSymbolDoc.h"
#include "Symbol.h"
#include "HeaderStamp.h"
#include "TinyCadMultiSymbolDoc.h"

////// The Library Member Functions //////


// Create an empty library unattached to any file
CLibraryFile::CLibraryFile()
{
  revision = 255;
  methodsFileOpen=FALSE;

}

// The destructor
CLibraryFile::~CLibraryFile()
{
  // Ensure the last get symbol pointer is cleared before it is deleted!
  if (methodsFileOpen)
	theMethodsFile.Close();
}

// Create a library associated with an index and methods file
void CLibraryFile::Attach(const TCHAR *FileName)
{
  CFile theIndex;
  CFileException ep;
	BYTE			typ = xNULL;

	m_name = FileName;

  if (!OpenIndexFile(theIndex,CFile::modeRead))
	return;
	
  // Open the archive header
  CStreamFile theIndexStream(&theIndex,CArchive::load);

  typedef std::map<CString, int> symbolNameCollection;
  symbolNameCollection	names;


  try
  {
	// Check the header on the index file
		CHeaderStamp oHeader;
		oHeader.Read( theIndexStream );

		if( oHeader.IsChecked(true) )
	{

		// Read in the symbol table names
		for (;;) 
		{
			theIndexStream >> typ;
			// Is it the index information part?
			if (typ == xIndexInformation) 
			{
				theIndexStream >> revision;
			} 
			// Is it a symbol definition?
			else if (typ==xSymbol || typ == xSymbolEx || typ == xSymbolEx2 || typ == xSymbolMethod ) 
			{
				// Add the symbol to the linked list
				CLibraryStoreNameSet nwSymbol(this);

				// Check for old type of symbol descriptor
				switch (typ)
				{
				case xSymbol:
					nwSymbol.OldLoad2(theIndexStream);
					break;
				case xSymbolEx:
					nwSymbol.OldLoad1(theIndexStream);
					break;
				case xSymbolEx2:
					nwSymbol.OldLoad3(theIndexStream);
					break;
				case xSymbolMethod:
					nwSymbol.Load(theIndexStream);
					break;
				}
				

				// If this is an xSymbolMethod then what follows is the
				// symbol's method, so we must skip it...
				if (typ == xSymbolMethod)
				{
					// Get the location of the next symbol in the 
					// file
					DWORD size;
					theIndexStream >> size;

					theIndexStream.Flush();
					CFile* fp = theIndexStream.GetFile();

					// Update the symbol with the correct location
					// of the method
					nwSymbol.FilePos = static_cast<DWORD>(fp->GetPosition());

					// Now skip the method part ...
					fp->Seek( size, CFile::current );
				}

				// Has this symbol name been used before?
				CSymbolRecord &r = nwSymbol.GetRecord( 0 );
				symbolNameCollection::iterator it = names.find( r.name );

				if (it != names.end())
				{
					// Remove the old name
					symbolCollection::iterator itx = m_Symbols.find( (*it).second );
					m_Symbols.erase( itx );
				}

				m_Symbols[ nwSymbol.FilePos ] = nwSymbol;
				names[ r.name ] = nwSymbol.FilePos;
				

			} 
			else 
			{
				CDialog theDialog(_T("BadSymbol"));
				theDialog.DoModal();
				methodsFileOpen=FALSE;
				return;
			}
		}
	}
  }
  catch( CArchiveException *e )
  {
	// end of files are allowed, all other exceptions are not
	if (e->m_cause != CArchiveException::endOfFile) 
	{
		CString s;
		CString msg;
		e->GetErrorMessage( msg.GetBuffer(256), 256, NULL );
		msg.ReleaseBuffer();
		s.Format(_T("Cannot read library %s.\r\n%s"),
			m_name, msg );
		AfxMessageBox( s );
		e->Delete();
	}
	e->Delete();
  }
  catch( CException *e)
  {
		CString s;
		CString msg;
		e->GetErrorMessage( msg.GetBuffer(256), 256, NULL );
		msg.ReleaseBuffer();
		s.Format(_T("Cannot read library %s.\r\n%s"),
			m_name, msg );
		AfxMessageBox( s );
		e->Delete();
  }

  methodsFileOpen=FALSE;
}



// Generate the methods file name from library name and revision number
CString CLibraryFile::MethodsFileName()
{
	CString hold;

	// If this is a new style library, then the
	// methods file and index file are one and the
	// same ...
	if (revision == 255)
	{
		hold = m_name + ".idx";
	}
	else
	{
		hold = m_name + ".m";
		TCHAR buffer[10];

		_itot(revision,buffer,10);

		if (_tcslen(buffer)<2)
		hold = hold + "0";
		hold = hold + buffer;
	}

	return hold;
}

// Get the methods file associated with this library
CStream *CLibraryFile::GetMethodArchive( CLibraryStoreNameSet *s )
{
  CFileException ep;

  if (!methodsFileOpen) 
  {
		if (!theMethodsFile.Open(MethodsFileName(),CFile::modeRead,&ep)) 
		{
			Message(IDS_MISSLIB,MB_ICONEXCLAMATION | MB_OK,m_name);
			methodsFileOpen = FALSE;
			return NULL;
		}
		methodsFileOpen=TRUE;
  }

  try
  {
	  theMethodsFile.Seek(s->FilePos,CFile::begin);
	  return new CStreamFile(&theMethodsFile, CArchive::load );
  }
  catch (...)
  {
	  // Failed to load the symbol
	  return FALSE;
  }


  return FALSE;
}



// Close the shared methods file, if it is currently open
void CLibraryFile::CloseMethodFile()
{
  if (methodsFileOpen) 
  {
	theMethodsFile.Close();
	methodsFileOpen=FALSE;
  }
}




// Open the index file
BOOL CLibraryFile::OpenIndexFile(CFile &theIndexFile,int mode)
{
  CFileException ep;

  if (!theIndexFile.Open(m_name+".idx",mode,&ep)) 
  {
	Message(IDS_MISSLIB,MB_ICONEXCLAMATION | MB_OK,m_name);
	return FALSE;
  }

  return TRUE;
}


// Delete a symbol from this library
void CLibraryFile::DeleteSymbol( CLibraryStoreNameSet &symbol )
{
	// Delete the symbol from this library...
	AfxMessageBox( IDS_NO_SYM_DELETE );
}




#if 0
void CLibraryFile::SaveSymbolCollection( symbolCollection &temp_symbols )
{
  // Now write back the information contained in the index file, and
  // use it to delete unecessary information in the methods file

  // Make sure our files have been closed
  CloseMethodFile();
  
  // Now rename the index file out of the way of
  // the new index file
  try
  {
	CFile::Remove( m_name + ".idx.old" );
  }
  catch (...)
  {
  }

  CFile::Rename( m_name + ".idx", m_name + ".idx.old" );

  // Open the old methods file for reading
  CFile theIndex;
  CFile OldMethods;
  CString OldMethodsName;
  
  if( revision == 255 )
  {
	  OldMethodsName = m_name + ".idx.old";
  }
  else
  {
	  OldMethodsName = MethodsFileName();
  }

  int ok = OldMethods.Open(OldMethodsName,CFile::modeRead);

  // change the revision number
  revision = 255;

  // Open a new methods file for writing to
  ok &= theIndex.Open(MethodsFileName(),CFile::modeCreate | CFile::modeWrite);

  // Now re-open the index file for writing
  CStreamFile theIndexStream(&theIndex,CArchive::store);

	CHeaderStamp().Write( theIndexStream );

  // Now write the library revision information
  theIndexStream << (BYTE)xIndexInformation;
  theIndexStream << revision;

  // Now we can write the new index file back
	DWORD		OldPos=-1;
  CTinyCadDoc tmp_design;

  for( symbolCollection::iterator i = temp_symbols.begin(); i != temp_symbols.end(); i++ ) 
  {
	CLibraryStoreNameSet thisSymbol = i->second;

	// Move the old method from the old file to the new file
	OldPos = thisSymbol.FilePos;
	
	// Read the methods file into this design
	OldMethods.Seek( OldPos, CFile::begin);
	CStreamFile stream( &OldMethods, CArchive::load );
	tmp_design.Import(stream);

	// Now we can write this symbol into the new index file
	theIndexStream << (BYTE)xSymbolMethod;
	thisSymbol.Save(theIndexStream);

	// Write the method to the index file
	WriteMethod(theIndexStream, tmp_design);

	// Now remove this design from memory
	tmp_design.SelectDelete();
  }  


  // Close all the files to ensure the data is written correctly
  theIndex.Close();
  OldMethods.Close();

  // Now re-read this library
  ReRead();
  
  // Now inform the user it has been done
  SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );
}
#endif


// Write a symbol to the file
void CLibraryFile::WriteMethod( CStreamFile &stream, CTinyCadDoc &design )
{
	// Write a place holder for the method length.
	// We only need this if we are storing methods and
	// symbols in the same file...
	CFile *pFile = NULL;
	DWORD place_holder_pos = 0;
	if (revision == 255)
	{
		stream.Flush();
		pFile = stream.GetFile();
		place_holder_pos = static_cast<DWORD>(pFile->GetPosition());
		stream << (DWORD)0;
	}

	// Now save this design onto the end of the new methods file
	CXMLWriter xml( &stream );
	design.SaveXML(xml,FALSE,FALSE);

	// Skip back to the place holder
	// and write in the size of the method.  We then
	// seek to the end of the file ready for the next method
	if (revision == 255)
	{
		stream.Flush();
		DWORD file_size = static_cast<DWORD>(pFile->GetPosition());
		pFile->Seek( place_holder_pos, CFile::begin );
		stream << (DWORD)( file_size - place_holder_pos - sizeof(DWORD) );
		stream.Flush();
		pFile->Seek( file_size, CFile::begin );
	}
}

// Write a symbol to this library
void CLibraryFile::Store( CLibraryStoreNameSet *nwSymbol, CTinyCadMultiSymbolDoc &document )
{
	// Writing to the old file type is not supported - must upgrade to the new library type
#if 0
	BOOL ok;

	// Set the busy cursor
	SetCursor( AfxGetApp()->LoadStandardCursor( IDC_WAIT ) );

	// Close down the current access to the file
	CloseMethodFile();

	// Now save the symbol into a file

	// Open the file, and move to the end of it
	CFile theIndex;
	CFile *theMethods;
	CFileException e;

	// Do we have one or two files here?
	if (revision == 255)
	{
		// One file
		ok = theIndex.Open( m_name+".idx",CFile::modeReadWrite,&e);
		theMethods = &theIndex;
	}
	else
	{
		// Two files
		theMethods = new CFile();
		ok = theMethods->Open(MethodsFileName(),CFile::modeReadWrite,&e)
			&& theIndex.Open( m_name+".idx",CFile::modeReadWrite,&e);
	}

	while (!ok) 
	{
		// Does this file exist?
		if (e.m_cause==CFileException::fileNotFound) 
		{
			revision = 255;

			if (Message(IDS_NOLIBRARY,MB_YESNO | MB_ICONQUESTION)!=IDYES)
			{
				return;
			}

			// Create just a single index file
			ok = OpenIndexFile(theIndex,CFile::modeCreate | CFile::modeReadWrite);
			theMethods = &theIndex;

			// Write the header to the Index file
			if (ok) 
			{
				CStreamFile theIndexStream(&theIndex,CArchive::store);

				CHeaderStamp().Write(theIndexStream);

				// Now write the library revision information
				theIndexStream << (BYTE)xIndexInformation;
				theIndexStream << revision;
			}
		}
		else
		{
			SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );
			Message(IDS_BADLIB,MB_ICONEXCLAMATION | MB_OK, m_name);
			return;
		}
	}

	// check the header stamp in the index file
	theIndex.SeekToBegin();
	
	CStreamFile theIndexStreamIn(&theIndex,CArchive::load);
	CHeaderStamp	oHeader;

	oHeader.Read( theIndexStreamIn );

	if( ! oHeader.IsChecked(true) ) 
	{
		SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );
		Message(IDS_BADLIB,MB_ICONEXCLAMATION | MB_OK, m_name);
		return;
	}
	theIndexStreamIn.Close();


	// now write the index entry to the end of the file
	theIndex.SeekToEnd();
	theMethods->SeekToEnd();
	nwSymbol->FilePos=static_cast<DWORD>(theMethods->GetPosition());

	CStreamFile theIndexStream( &theIndex,CArchive::store);
	if (revision == 255)
	{
		theIndexStream << (BYTE)xSymbolMethod;
	}
	else
	{
		theIndexStream << (BYTE)xSymbolEx2;
	}
	nwSymbol->Save(theIndexStream);
	theIndexStream.Close();


	// Write the symbol data into the methods file
	{
		CStreamFile stream( theMethods, CArchive::store );
		WriteMethod( stream, document );
	}


	// Close the files
	theIndex.Close();

	if (revision != 255)
	{
		theMethods->Close();
		delete theMethods;
	}

	// Inform the design it has been saved
	document.SetModifiedFlag(FALSE);

	// Re-load the library now it has changed
	ReRead();

	// Inform the document views
	CTinyCadApp::ResetAllSymbols();

	SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );
#endif
}

bool CLibraryFile::Create(const TCHAR *filename)
{
	m_name = filename;

	// We don't have to do anything to create a library
	return true;
}

// Is an upgrade required before editing this library?
BOOL CLibraryFile::MustUpgrade()
{
	return TRUE;
}


// Upgrade to the latest version of the library system
BOOL CLibraryFile::Upgrade(CLibraryStore *NewLib)
{
	// Create the new library
	if (!NewLib->Create( m_name ))
	{
		return FALSE;
	}

	// Now copy our symbols into the new library

	// Make sure our files have been closed
	CloseMethodFile();

	// Now rename the index file out of the way of
	// the new index file
	try
	{
		CFile::Remove( m_name + ".idx.old" );
	}
	catch (...)
	{
	}

	CFile::Rename( m_name + ".idx", m_name + ".idx.old" );

	// Open the old methods file for reading
	CFile theIndex;
	CFile OldMethods;
	CString OldMethodsName;

	if( revision == 255 )
	{
	  OldMethodsName = m_name + ".idx.old";
	}
	else
	{
	  OldMethodsName = MethodsFileName();
	}

	int ok = OldMethods.Open(OldMethodsName,CFile::modeRead);

	// Now we can write the new index file back
	for( symbolCollection::iterator i = m_Symbols.begin(); i != m_Symbols.end(); i++ ) 
	{
		CTinyCadMultiSymbolDoc tmp_design;
		CLibraryStoreNameSet thisSymbol = i->second;

		// Read the methods file into this design
		OldMethods.Seek( thisSymbol.FilePos, CFile::begin);
		CStreamFile stream( &OldMethods, CArchive::load );
		tmp_design.GetActiveSheet()->Import(stream);

		// Clear this symbols identity...
		thisSymbol.FilePos = -1;

		CSymbolRecord &r = thisSymbol.GetRecord(0);
		r.NameID = -1;

		// Write this symbol into the new library
		NewLib->Store( &thisSymbol, tmp_design );

		// Now remove this design from memory
		tmp_design.GetActiveSheet()->SelectDelete();
	}  

	return TRUE;
}

