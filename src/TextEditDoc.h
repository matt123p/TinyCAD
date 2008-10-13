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

#pragma once


// CTextEditDoc document

class CTextEditDoc : public CDocument
{
	DECLARE_DYNCREATE(CTextEditDoc)

	CString		m_text;
	bool		m_read_only;

public:
	CTextEditDoc();
	virtual ~CTextEditDoc();
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CString& GetText() { return m_text; }
	bool GetReadOnly() { return m_read_only; }

	BOOL LoadFile( const TCHAR *file_name, bool read_only );
	void SetText( const TCHAR *text, bool read_only );

protected:
	virtual BOOL OnNewDocument();
	DECLARE_MESSAGE_MAP()
};
