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

// MultiSheetDoc.cpp : implementation file
//

#include "stdafx.h"
#include "tinycad.h"
#include "MultiSheetDoc.h"
#include "TinyCadView.h"


/////////////////////////////////////////////////////////////////////////////
// CMultiSheetDoc

IMPLEMENT_DYNCREATE(CMultiSheetDoc, CDocument)

CMultiSheetDoc::CMultiSheetDoc()
{
}

BOOL CMultiSheetDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CMultiSheetDoc::~CMultiSheetDoc()
{
}


BEGIN_MESSAGE_MAP(CMultiSheetDoc, CDocument)
	//{{AFX_MSG_MAP(CMultiSheetDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiSheetDoc diagnostics

#ifdef _DEBUG
void CMultiSheetDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMultiSheetDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMultiSheetDoc serialization

void CMultiSheetDoc::Serialize(CArchive& ar)
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
// CMultiSheetDoc commands


// Force an autosave of the document
void CMultiSheetDoc::AutoSave()
{
}

// Get the file path name during loading or saving
CString CMultiSheetDoc::GetXMLPathName()
{
	return GetPathName();
}


// Is this document editing a library?
bool CMultiSheetDoc::IsLibInUse( CLibraryStore *lib )
{
	return false;
}



// get the number of documents in this multi-doc
int CMultiSheetDoc::GetNumberOfSheets()
{
	return 0;
}



void CMultiSheetDoc::SetActiveSheetIndex( int i )
{
}



int CMultiSheetDoc::GetActiveSheetIndex()
{
	return 0;
}



CString CMultiSheetDoc::GetSheetName( int i )
{
	return "error";
}

CTinyCadDoc* CMultiSheetDoc::GetSheet( int i )
{
	return NULL;
}


// Get the currently active sheet to work with
CTinyCadDoc* CMultiSheetDoc::GetCurrentSheet()
{
	return GetSheet( GetActiveSheetIndex() );
}

void CMultiSheetDoc::OnFolderContextMenu()
{
}

void CMultiSheetDoc::SelectSheetView( int i )
{
	POSITION pos = GetFirstViewPosition();
    while (pos != NULL)
    {
		CView* pView = GetNextView(pos);
		if (pView->IsKindOf( RUNTIME_CLASS( CTinyCadView )))
		{
			static_cast<CTinyCadView*>(pView)->SelectSheet( i );
			pView->RedrawWindow();
		}
    }
}
