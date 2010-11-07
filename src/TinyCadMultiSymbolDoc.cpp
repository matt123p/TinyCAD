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

// TinyCadMultiSymbolDoc.cpp : implementation file
//

#include "stdafx.h"
#include "tinycad.h"
#include "TinyCadMultiSymbolDoc.h"
#include "DlgPartsInPackage.h"
#include ".\tinycadmultisymboldoc.h"
#include "DlgUpdateBox.h"
#include "HeaderStamp.h"


/////////////////////////////////////////////////////////////////////////////
// CTinyCadMultiSymbolDoc

IMPLEMENT_DYNCREATE(CTinyCadMultiSymbolDoc, CMultiSheetDoc)

CTinyCadMultiSymbolDoc::CTinyCadMultiSymbolDoc()
{
	m_symbols.resize( 1 );
	m_symbols[0] = new CTinyCadSymbolDoc( this );
	m_ppp = 1;
	m_heterogeneous = false;
	m_current_index = 0;
}

CTinyCadMultiSymbolDoc::CTinyCadMultiSymbolDoc(CLibraryStore* pLib, CLibraryStoreNameSet &symbol )
{
	m_symboledit = symbol;
	m_libedit = pLib;

	m_heterogeneous = false;
	m_current_index = 0;

	// Set our name according to the symbol
	CString title = "Symbol - " + symbol.GetRecord(0).name;
	SetTitle( title );

	// Get the file to load the symbol details from
	CStream* s = symbol.GetMethodArchive();
	if (!s)
	{
		// Create a blank symbol, this must a new one!
		m_symbols.resize( 1 );
		m_symbols[0] = new CTinyCadSymbolDoc( this );
		m_ppp = 1;
		m_heterogeneous = false;
		m_current_index = 0;

		return;
	}


	// We have to see if this is an XML file or an old binary format 
	// symbol
	LONG pos = s->GetPos();
	CHeaderStamp oHeader;
	oHeader.Read( *s );
	s->Seek(pos);

	if( oHeader.IsChecked(false) ) 
	{
		m_symbols.resize( 1 );
		m_symbols[0] = new CTinyCadSymbolDoc( this );
		m_symbols[0]->ReadFile( *s );
		m_symbols[0]->setSymbol();
		m_ppp = m_symbols[0]->GetPartsPerPackage();
	}
	else
	{
		CXMLReader xml( s );
		CString tag;
		xml.nextTag( tag );
		if (tag == "TinyCADSheets")
		{
			LoadXML( xml, true );
		}
		else if (tag == "TinyCAD")
		{
			LoadXML( xml, false );
		}
		else
		{
			// Not much to do about this!
		}
	}

	delete s;
}


BOOL CTinyCadMultiSymbolDoc::OnNewDocument()
{
	if (!CMultiSheetDoc::OnNewDocument())
		return FALSE;
	return TRUE;
}

CTinyCadMultiSymbolDoc::~CTinyCadMultiSymbolDoc()
{
	sheetCollection::iterator i = m_symbols.begin();
	while (i != m_symbols.end())
	{
		delete *i;
		++ i;
	}
}


BEGIN_MESSAGE_MAP(CTinyCadMultiSymbolDoc, CMultiSheetDoc)
	//{{AFX_MSG_MAP(CTinyCadMultiSymbolDoc)
	ON_COMMAND(ID_LIBRARY_ADDPIN, OnLibraryAddpin)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_LIBRARY_SETPARTSPERPACKAGE, OnLibrarySetpartsperpackage)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_LIBRARY_HETEROGENEOUS, OnLibraryHeterogeneous)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_HETEROGENEOUS, OnUpdateLibraryHeterogeneous)
	ON_COMMAND(ID_LIBRARY_HOMOGENEOUS, OnLibraryHomogeneous)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_HOMOGENEOUS, OnUpdateLibraryHomogeneous)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTinyCadMultiSymbolDoc diagnostics

#ifdef _DEBUG
void CTinyCadMultiSymbolDoc::AssertValid() const
{
	CMultiSheetDoc::AssertValid();
}

void CTinyCadMultiSymbolDoc::Dump(CDumpContext& dc) const
{
	CMultiSheetDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTinyCadMultiSymbolDoc serialization

void CTinyCadMultiSymbolDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTinyCadMultiSymbolDoc commands

// Is this document editing a library?
bool CTinyCadMultiSymbolDoc::IsLibInUse( CLibraryStore *lib )
{
	return lib == m_libedit;
}



// get the number of documents in this multi-doc
int CTinyCadMultiSymbolDoc::GetNumberOfSheets()
{
	return m_ppp;
}



void CTinyCadMultiSymbolDoc::SetActiveSheetIndex( int i )
{
	if (m_heterogeneous)
	{
		m_current_index = i;
	}
	else
	{
		m_symbols[0]->EditPartInPackage( i );
	}
}



int CTinyCadMultiSymbolDoc::GetActiveSheetIndex()
{
	if (m_heterogeneous)
	{
		return m_current_index;
	}
	else
	{
		return m_symbols[0]->GetPart();
	}
}



CString CTinyCadMultiSymbolDoc::GetSheetName( int i )
{
	CString r;
	r.Format( _T("Part %c"), 'A' + i );
	return r;
}


// Get the currently active sheet to work with
CTinyCadDoc* CTinyCadMultiSymbolDoc::GetSheet( int i )
{
	if (m_heterogeneous)
	{
		return m_symbols[ i ];
	}
	else
	{
		return m_symbols[0];
	}
}

void CTinyCadMultiSymbolDoc::OnFolderContextMenu()
{
	// Get the current location of the mouse
	CPoint pt;
	GetCursorPos( &pt );

	// Now bring up the context menu..
	CMenu menu;
	menu.LoadMenu( IDR_SYMBOL_SHEET_MENU );
	menu.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN|TPM_RIGHTBUTTON,
		pt.x,pt.y, AfxGetMainWnd(), NULL );
}

CTinyCadDoc* CTinyCadMultiSymbolDoc::GetActiveSheet()
{
	return GetSheet( GetActiveSheetIndex() );
}

void CTinyCadMultiSymbolDoc::OnLibraryAddpin() 
{
	GetActiveSheet()->SelectObject(new CDrawPin(GetActiveSheet()));
}


void CTinyCadMultiSymbolDoc::OnFileSave() 
{
	Store();
}

void CTinyCadMultiSymbolDoc::OnFileSaveAs() 
{
	Store();
}


BOOL CTinyCadMultiSymbolDoc::CanCloseFrame(CFrameWnd* pFrameArg)
{
	if (IsModified())
	{
		CString prompt;
		AfxFormatString1(prompt, AFX_IDP_ASK_TO_SAVE, m_symboledit.GetRecord(0).name);
		switch (AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE))
		{
		case IDCANCEL:
			return FALSE;
		case IDYES:
			return Store();
		case IDNO:
			break;
		}
	}

	return TRUE;
}


void CTinyCadMultiSymbolDoc::OnLibrarySetpartsperpackage() 
{
	CDlgPartsInPackage dialog;
	dialog.m_Parts = m_ppp;

	if (dialog.DoModal() == IDOK)
	{
		if (dialog.m_Parts > 0 && dialog.m_Parts <= 26)
		{
			m_ppp = dialog.m_Parts;
			

			if (m_heterogeneous)
			{
				for (unsigned int i = m_ppp; i < m_symbols.size(); ++i )
				{
					delete m_symbols[i];
				}

				m_symbols.resize( m_ppp );

				for (unsigned int i = 0; i < m_symbols.size(); ++i )
				{
					if (!m_symbols[i])
					{	
						m_symbols[i] = new CTinyCadSymbolDoc( this );
					}
				}

				m_symbols[ 0 ]->SetPartsPerPackage( 1 );
			}
			else
			{
				m_symbols[ 0 ]->SetPartsPerPackage( m_ppp );
			}

			UpdateAllViews( NULL, DOC_UPDATE_TABS );
		}
	}
	
}

void CTinyCadMultiSymbolDoc::OnLibraryHeterogeneous()
{
	// If we are already hetrogeneous then ignore this click
	if (m_heterogeneous)
	{
		return;
	}

	// We can only change if there is only a single part per-package
	if (m_ppp != 1)
	{
		AfxMessageBox( IDS_BAD_PPP );
	}
	else
	{
		m_heterogeneous = true;
	}
}

void CTinyCadMultiSymbolDoc::OnUpdateLibraryHeterogeneous(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( m_heterogeneous ? 1 : 0 );
}

void CTinyCadMultiSymbolDoc::OnLibraryHomogeneous()
{
	// If we are already homogeneous then ignore this click
	if (!m_heterogeneous)
	{
		return;
	}

	// We can only change if there is only a single part per-package
	if (m_ppp != 1)
	{
		AfxMessageBox( IDS_BAD_PPP );
	}
	else
	{
		m_heterogeneous = false;
	}
}

void CTinyCadMultiSymbolDoc::OnUpdateLibraryHomogeneous(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck( m_heterogeneous ? 0 : 1 );
}

// Write this symbol back to the library
BOOL CTinyCadMultiSymbolDoc::Store()
{
	m_symboledit.ppp = (BYTE) m_ppp;

	CDlgUpdateBox dlg( AfxGetMainWnd() );
	dlg.SetSymbol( &m_symboledit );

	if (dlg.DoModal() == IDOK)
	{
		(m_symbols[0])->GetOptions()->UnTag();
		m_libedit->Store( getSymbol(), *this );
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}

void CTinyCadMultiSymbolDoc::SaveXML( CXMLWriter &xml )
{
	if (m_heterogeneous)
	{
		xml.addTag(_T("TinyCADSheets"));
		sheetCollection::iterator i = m_symbols.begin();
		while (i != m_symbols.end())
		{
			(*i)->SaveXML(xml,FALSE,FALSE);
			(*i)->MarkChangeForUndo(NULL);
			++ i;
		}
		xml.closeTag();		
	}
	else
	{
		m_symbols[0]->SaveXML(xml,FALSE,FALSE);
		m_symbols[0]->MarkChangeForUndo(NULL);
	}
}

void CTinyCadMultiSymbolDoc::LoadXML( CXMLReader &xml, bool heterogeneous )
{
	m_heterogeneous = heterogeneous;

	// Clear out the old design
	sheetCollection::iterator i = m_symbols.begin();
	while (i != m_symbols.end())
	{
		delete *i;
		++ i;
	}
	m_symbols.resize(0);		

	// Load in the new design
	if (m_heterogeneous)
	{
		m_ppp = 0;
		xml.intoTag();
		CString tag;
		while (xml.nextTag( tag ))
		{
			if (tag == "TinyCAD")
			{
				CTinyCadSymbolDoc *pDesign = new CTinyCadSymbolDoc( this );
				pDesign->ReadFileXML(xml,TRUE);
				pDesign->setSymbol();
				m_symbols.push_back( pDesign );
				++ m_ppp;
			}
		}
		xml.outofTag();
	}
	else
	{
		CTinyCadSymbolDoc *pDesign = new CTinyCadSymbolDoc( this );
		pDesign->ReadFileXML(xml,TRUE);
		pDesign->setSymbol();
		m_ppp = pDesign->GetPartsPerPackage();
		m_symbols.push_back( pDesign );
	}
}


