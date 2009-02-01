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

// TextEditDoc.cpp : implementation file
//

#include "stdafx.h"
#include "TinyCad.h"
#include "TextEditDoc.h"


// CTextEditDoc

IMPLEMENT_DYNCREATE(CTextEditDoc, CDocument)

CTextEditDoc::CTextEditDoc()
{
	m_read_only = false;
}

BOOL CTextEditDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CTextEditDoc::~CTextEditDoc()
{
}


BEGIN_MESSAGE_MAP(CTextEditDoc, CDocument)
END_MESSAGE_MAP()


// CTextEditDoc diagnostics

#ifdef _DEBUG
void CTextEditDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTextEditDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CTextEditDoc serialization

void CTextEditDoc::Serialize(CArchive& ar)
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

void CTextEditDoc::SetText( const TCHAR *text, bool read_only )
{
	m_read_only = read_only;
	m_text = text;
}


BOOL CTextEditDoc::LoadFile( const TCHAR *file_name, bool read_only )
{
	CFile file;
	CFileException e;

	m_read_only = read_only;

	if (!file.Open( file_name, CFile::modeRead, &e ))
	{
		CString s;
		CString msg;
		e.GetErrorMessage( msg.GetBuffer(256), 256, NULL );
		msg.ReleaseBuffer();
		s.Format(_T("Cannot load file.\r\n%s"),
			msg );
		AfxMessageBox( s );

		return FALSE;
	}

	// Load the file
	int len = static_cast<int>(file.GetLength());
	char *q = new char[ len + 2 ]; // m_text.GetBuffer( len + 2 );
	file.Read( q, len );
	q[len] = 0;
	m_text = q;
	delete[] q;

	// Set the text document's name
	SetPathName( file_name, FALSE );

	return TRUE;
}

// CTextEditDoc commands
