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

#if !defined(AFX_TINYCADMULTIDOC_H__7E25C39B_649E_4421_A207_635409612FB6__INCLUDED_)
#define AFX_TINYCADMULTIDOC_H__7E25C39B_649E_4421_A207_635409612FB6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TinyCadDoc.h"
#include "MultiSheetDoc.h"

// TinyCadMultiDoc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTinyCadMultiDoc document

class CTinyCadMultiDoc : public CMultiSheetDoc
{
protected:
	DECLARE_DYNCREATE(CTinyCadMultiDoc)

	void UnTag();

	// Save this collection out...
	void SaveXML(CXMLWriter&);

	// Load the document
	BOOL ReadFile( CStreamFile& file );

	void SetTabsFromDocument();

	void Clear();
	void InsertSheet(int i, CTinyCadDoc *pDoc = NULL);
	void DeleteSheet(int i);

// Attributes
public:

	// Construction
	CTinyCadMultiDoc();

	// Force an autosave of the document
	virtual void AutoSave();

	// Is this document editing a library?
	virtual bool IsLibInUse( CLibraryStore *lib );


	// get the number of documents in this multi-doc
	virtual int	 GetNumberOfSheets();
	virtual void SetActiveSheetIndex( int i );
	virtual int GetActiveSheetIndex() { return m_active_doc; }
	virtual CString GetSheetName( int i );

	// Get the currently active sheet to work with
	virtual CTinyCadDoc*	GetSheet(int i);

	virtual void OnFolderContextMenu();

	// Get the file path name during loading or saving
	virtual CString GetXMLPathName();

protected:
	typedef std::vector<CTinyCadDoc*>		sheetCollection;
	sheetCollection		m_sheets;
	
	int					m_active_doc;

	// The filename during loading/saving
	CString				m_xml_filename;


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTinyCadMultiDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTinyCadMultiDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CTinyCadMultiDoc)
	afx_msg void OnContextAddsheet();
	afx_msg void OnContextDeletesheet();
	afx_msg void OnUpdateContextDeletesheet(CCmdUI* pCmdUI);
	afx_msg void OnContextRenamesheet();
	afx_msg void OnLibraryAddpin();
	afx_msg void OnUpdateLibraryAddpin(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnContextAddhierachicalsymbol();
	afx_msg void OnUpdateContextAddhierachicalsymbol(CCmdUI *pCmdUI);
	afx_msg void OnUpdateContextRenamesheet(CCmdUI *pCmdUI);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TINYCADMULTIDOC_H__7E25C39B_649E_4421_A207_635409612FB6__INCLUDED_)
