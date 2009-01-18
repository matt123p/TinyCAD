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

// TinyCadMultiDoc.cpp : implementation file
//

#include "stdafx.h"
#include "tinycad.h"
#include "TinyCadMultiDoc.h"
#include "HeaderStamp.h"
#include "DlgRenameSheet.h"
#include ".\tinycadmultidoc.h"
#include "TinyCadHierarchicalDoc.h"


/////////////////////////////////////////////////////////////////////////////
// CTinyCadMultiDoc

IMPLEMENT_DYNCREATE(CTinyCadMultiDoc, CMultiSheetDoc)

CTinyCadMultiDoc::CTinyCadMultiDoc()
{
	m_active_doc = 0;
}

BOOL CTinyCadMultiDoc::OnNewDocument()
{
	if (!CMultiSheetDoc::OnNewDocument())
		return FALSE;


	Clear();
	
	m_sheets.push_back( new CTinyCadDoc( this ) );


	return TRUE;
}

CTinyCadMultiDoc::~CTinyCadMultiDoc()
{
	m_active_doc = 0;
	Clear();
}


BEGIN_MESSAGE_MAP(CTinyCadMultiDoc, CMultiSheetDoc)
	//{{AFX_MSG_MAP(CTinyCadMultiDoc)
	ON_COMMAND(ID_CONTEXT_ADDSHEET, OnContextAddsheet)
	ON_COMMAND(ID_CONTEXT_DELETESHEET, OnContextDeletesheet)
	ON_UPDATE_COMMAND_UI(ID_CONTEXT_DELETESHEET, OnUpdateContextDeletesheet)
	ON_COMMAND(ID_CONTEXT_RENAMESHEET, OnContextRenamesheet)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_CONTEXT_ADDHIERACHICALSYMBOL, OnContextAddhierachicalsymbol)
	ON_UPDATE_COMMAND_UI(ID_CONTEXT_ADDHIERACHICALSYMBOL, OnUpdateContextAddhierachicalsymbol)
	ON_COMMAND(ID_LIBRARY_ADDPIN, OnLibraryAddpin)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_ADDPIN, OnUpdateLibraryAddpin)
	ON_UPDATE_COMMAND_UI(ID_CONTEXT_RENAMESHEET, OnUpdateContextRenamesheet)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTinyCadMultiDoc diagnostics

#ifdef _DEBUG
void CTinyCadMultiDoc::AssertValid() const
{
	CMultiSheetDoc::AssertValid();
}

void CTinyCadMultiDoc::Dump(CDumpContext& dc) const
{
	CMultiSheetDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTinyCadMultiDoc serialization



void CTinyCadMultiDoc::Serialize(CArchive& ar)
{
	m_xml_filename = ar.m_strFileName;

	if (ar.IsStoring())
	{
		UnTag();
		CStreamFile stream( &ar );
	    CXMLWriter xml( &stream );
		SaveXML( xml );
	}
	else
	{
		CStreamFile stream( &ar );
		if (ReadFile(stream) )
		{
			SetModifiedFlag( FALSE );
		}
		else
		{
			OnNewDocument();
		}

	}
}

// Get the file path name during loading or saving
CString CTinyCadMultiDoc::GetXMLPathName()
{
	return m_xml_filename;
}


void CTinyCadMultiDoc::Clear()
{
	sheetCollection::iterator i = m_sheets.begin();
	while (i != m_sheets.end())
	{
		delete (*i);
		++ i;
	}

	m_sheets.clear();
}

void CTinyCadMultiDoc::UnTag()
{
	sheetCollection::iterator i = m_sheets.begin();
	while (i != m_sheets.end())
	{
		(*i)->GetOptions()->UnTag();
		++ i;
	}

}

/////////////////////////////////////////////////////////////////////////////
// CTinyCadMultiDoc commands

CTinyCadDoc* CTinyCadMultiDoc::GetSheet( int i )
{
	if (m_sheets.size() == 0)
	{
		return NULL;
	}

	if (i < (int) m_sheets.size())
	{
		return m_sheets[ i ];
	}
	else
	{
		return m_sheets.front();
	}
}


//-------------------------------------------------------------------------
void CTinyCadMultiDoc::InsertSheet(int i , CTinyCadDoc *pDoc )
{
	sheetCollection::iterator it = m_sheets.begin();

	if (!pDoc)
	{
		pDoc = new CTinyCadDoc( this );
		if (GetCurrentSheet())
		{
			pDoc->GetDetails() = GetCurrentSheet()->GetDetails();
		}
		pDoc->GetDetails().m_sSheets.Format( _T("%d"), m_sheets.size() + 1 );
	}

	if (i == -1) {
		m_sheets.insert(it, pDoc );
	}
	else if (i != m_sheets.size())
	{
		m_sheets.insert(it+i+1, pDoc );
	}
	else
	{
		m_sheets.push_back( pDoc );
	}
	m_active_doc = i + 1;
}

//-------------------------------------------------------------------------
void CTinyCadMultiDoc::DeleteSheet(int i )
{
	sheetCollection::iterator it = m_sheets.begin();
	it += i;

	// First erase the document...
	delete *it;

	// Now remove it from the list
	m_sheets.erase(it);

	if (m_active_doc >= m_sheets.size())
	{
		m_active_doc --;
	}
}

//-------------------------------------------------------------------------
void CTinyCadMultiDoc::AutoSave()
{
	// We only backup files with a file name
	if (GetPathName().IsEmpty())
	{
		return;
	}

	// Show the busy icon
	SetCursor( AfxGetApp()->LoadStandardCursor( IDC_WAIT ) );

	// Get the filename
	CString theFileName = GetPathName() + ".autosave";

	// 
  	CFile theFile;

  	// Open the file for saving as a CFile for a CArchive
  	BOOL r =  theFile.Open(theFileName, CFile::modeCreate | CFile::modeWrite);

	if (r) 
	{
  		// Now save the file
		CStreamFile stream( &theFile, CArchive::store );
	    CXMLWriter xml( &stream );
		SaveXML(xml);
  	}

  // Turn the cursor back to normal
  SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );

  // Was there an error opening the file?
  if (!r)
  {
	// Could not open file to start saving
	Message(IDS_ABORTAUTOSAVE,MB_ICONEXCLAMATION);
  }

}


//-------------------------------------------------------------------------
BOOL CTinyCadMultiDoc::ReadFile( CStreamFile& file )
{
	// Is this an old style document?
	CDrawingObject*	obj		= NULL;
	BYTE			tp		= xNULL;
	CHeaderStamp	oHeader;


	Clear();

	LONG pos = file.GetPos();

	oHeader.Read( file );

	// Return the file position back the beginning
	file.Seek(pos);


	if( oHeader.IsChecked(false) )
	{
		// Use the old loader...
		CTinyCadDoc *pNewDoc = new CTinyCadDoc(this);
		if (pNewDoc->ReadFile( file ))
		{
			m_sheets.push_back( pNewDoc );
			return TRUE;
		}
		else
		{
			delete pNewDoc;
		}

	}
	else
	{
		// Use the XML loader
		CString name;
		CXMLReader xml( &file );

		xml.nextTag( name );

		if (name == "TinyCAD")
		{
			// Single sheet loader...
			CTinyCadDoc *pNewDoc = new CTinyCadDoc(this);
			if (pNewDoc->ReadFileXML( xml, TRUE ))
			{
				m_sheets.push_back( pNewDoc );
				return TRUE;
			}
			else
			{
				delete pNewDoc;
				return FALSE;
			}
		}


		if (name != "TinyCADSheets")
		{
			Message(IDS_ABORTVERSION,MB_ICONEXCLAMATION);
			return FALSE;
		}


		xml.intoTag();

		while (	xml.nextTag( name ) )
		{
			// Save the old layer setting
			CDrawingObject *obj = NULL;

			if (name == "DETAILS" )
			{
			}
			else if (name == "TinyCAD")
			{
				// Single sheet loader...
				CTinyCadDoc *pNewDoc = new CTinyCadDoc(this);
				pNewDoc->ReadFileXML( xml, TRUE );
				m_sheets.push_back( pNewDoc );
			}
			else if (name == "HierachicalSymbol")
			{
				// Hierachical symbol loader...
				CTinyCadDoc *pNewDoc = new CTinyCadHierarchicalDoc(this);
				pNewDoc->ReadFileXML( xml, TRUE );
				m_sheets.push_back( pNewDoc );
			}
		}

		xml.outofTag();

		return TRUE;
	}


	return FALSE;
}

//-------------------------------------------------------------------------
void CTinyCadMultiDoc::SaveXML( CXMLWriter &xml )
{
  // Write the objects to the file
  try
  {
	CString comment;

	comment.Format( _T("This file was written by TinyCAD %s\n")
					_T("If you wish to view this file go to http://tinycad.sourceforge.net to\n")
					_T("download a copy."),
					CTinyCadApp::GetVersion() );

	xml.addComment( comment );

    xml.addTag(_T("TinyCADSheets"));

	sheetCollection::iterator i = m_sheets.begin();
	while (i != m_sheets.end())
	{
		(*i)->SaveXML( xml, TRUE, FALSE );
		++ i;
	}


	xml.closeTag();
  }
  catch ( CException *e) 
  {
	// Could not save the file properly
    e->ReportError();
    e->Delete();
  }
}


// Is this document editing a library?
bool CTinyCadMultiDoc::IsLibInUse( CLibraryStore *lib )
{
	sheetCollection::iterator i = m_sheets.begin();
	while (i != m_sheets.end())
	{
		if ((*i)->IsLibInUse( lib ))
		{
			return true;
		}
		++ i;
	}

	return false;
}


// get the number of documents in this multi-doc
int CTinyCadMultiDoc::GetNumberOfSheets()
{
	return static_cast<int>(m_sheets.size());
}

// get the number of documents in this multi-doc
void CTinyCadMultiDoc::SetActiveSheetIndex( int i )
{
	ASSERT( i >= 0 && i < GetNumberOfSheets() );
	m_active_doc = i;
}

CString CTinyCadMultiDoc::GetSheetName( int i )
{
	ASSERT( i >= 0 && i < GetNumberOfSheets() );

	CString r = m_sheets[i]->GetSheetName();

	if (r.IsEmpty())
	{
		r.Format(_T("Sheet %d"), i + 1 );
		m_sheets[i]->SetSheetName( r );
	}

	return r;
}

void CTinyCadMultiDoc::OnFolderContextMenu()
{
	// Get the current location of the mouse
	CPoint pt;
	GetCursorPos( &pt );

	// Now bring up the context menu..
	CMenu menu;
	menu.LoadMenu( IDR_SHEET_MENU );
	menu.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN|TPM_RIGHTBUTTON,
		pt.x,pt.y, AfxGetMainWnd(), NULL );
}

void CTinyCadMultiDoc::OnContextAddsheet() 
{
	// switch back to the Edit tool
	GetCurrentSheet()->SelectObject( new CDrawEditItem(GetCurrentSheet()) );	

	// Insert the new sheet
	InsertSheet( GetActiveSheetIndex() ); 

	CString s;
	s.Format(_T("Sheet %d"), GetNumberOfSheets() );
	GetCurrentSheet()->SetSheetName( s );
		
	// Now redo the tabs
	SetTabsFromDocument();
}

void CTinyCadMultiDoc::OnContextDeletesheet() 
{
	if (AfxMessageBox( IDS_DELETE_SHEET, MB_YESNO ) == IDYES)
	{
		// switch back to the Edit tool
		GetCurrentSheet()->SelectObject( new CDrawEditItem(GetCurrentSheet()) );	

		// Insert the new sheet
		DeleteSheet( GetActiveSheetIndex() ); 
		
		// Now redo the tabs
		SetTabsFromDocument();
	}
}

void CTinyCadMultiDoc::OnUpdateContextDeletesheet(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetNumberOfSheets() > 1 );	
}

void CTinyCadMultiDoc::OnContextRenamesheet() 
{
	CDlgRenameSheet s;
	s.m_Name = GetCurrentSheet()->GetSheetName();

	if (s.DoModal() == IDOK && !s.m_Name.IsEmpty())
	{
		GetCurrentSheet()->SetSheetName( s.m_Name );
		SetTabsFromDocument();
	}
}

void CTinyCadMultiDoc::OnUpdateContextRenamesheet(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( !GetCurrentSheet()->IsHierarchicalSymbol() );
}


void CTinyCadMultiDoc::SetTabsFromDocument()
{
	UpdateAllViews( NULL, DOC_UPDATE_TABS );
}
void CTinyCadMultiDoc::OnContextAddhierachicalsymbol()
{
	// Add in a new document that is the hierachical symbol
	GetCurrentSheet()->SelectObject( new CDrawEditItem(GetCurrentSheet()) );	

	// Insert the new sheet		
	CTinyCadDoc *pDoc = new CTinyCadHierarchicalDoc( this );
	if (GetCurrentSheet())
	{
		pDoc->GetDetails() = GetCurrentSheet()->GetDetails();
	}

	InsertSheet( -1, pDoc ); 
		
	// Now redo the tabs
	SetTabsFromDocument();
}

void CTinyCadMultiDoc::OnUpdateContextAddhierachicalsymbol(CCmdUI *pCmdUI)
{
	// Determine if we already have the hierachical symbol
	pCmdUI->Enable(!m_sheets[0]->IsHierarchicalSymbol());
}

void CTinyCadMultiDoc::OnLibraryAddpin() 
{
	// Add a new pin to this drawing
	GetCurrentSheet()->SelectObject(new CDrawPin(GetCurrentSheet()) );
}

void CTinyCadMultiDoc::OnUpdateLibraryAddpin(CCmdUI* pCmdUI) 
{
	// Only allow pin additions on the hierachical symbol sheet
	pCmdUI->Enable( GetCurrentSheet()->IsHierarchicalSymbol() );
}

