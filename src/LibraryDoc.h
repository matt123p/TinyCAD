/*
 TinyCAD program for schematic capture
 Copyright 1994/1995/2002 Matt Pyne.

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

#if !defined(AFX_LIBRARYDOC_H__642CE6F7_87BC_4A90_BB6E_CB3293813F99__INCLUDED_)
#define AFX_LIBRARYDOC_H__642CE6F7_87BC_4A90_BB6E_CB3293813F99__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <set>
#include <list>

#include "DSize.h"

/////////////////////////////////////////////////////////////////////////////
// CLibraryDoc document

class CLibraryDoc: public CDocument
{
protected:
	CLibraryDoc(); // protected constructor used by dynamic creation
	DECLARE_DYNCREATE( CLibraryDoc)

protected:
	int m_selected;

	// Attributes
public:

	// Operations
public:

	// Is this document editing a library?
	virtual bool IsLibInUse(CLibraryStore *lib);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLibraryDoc)
public:
	virtual void Serialize(CArchive& ar); // overridden for document i/o
protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL


	// Implementation
public:
	void CopySymbol(int which, CLibraryStore *target);
	void EditSymbol(int which);
	void DeleteSymbol(int which);
	void DuplicateSymbol(int which);
	void SymbolProperties(int which);

	CLibraryStore::symbolCollection::iterator getSymbol(int which);

	typedef std::multimap<CString, int> idCollection;
	typedef std::set<int> heightCollection;
	idCollection m_SymbolMap;

	int getSelected()
	{
		return m_selected;
	}
	void setSelected(int i)
	{
		m_selected = i;
	}
	CLibraryStore* m_pLibrary;
	CLibraryStoreNameSet &getNameSet(const idCollection::iterator &it);

	void ResetSymbols();

	void setLibrary(CLibraryStore *pLibrary);
	virtual ~CLibraryDoc();

#ifdef _DEBUG
	virtual void AssertValid() const OVERRIDE;
	virtual void Dump(CDumpContext& dc) const OVERRIDE;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CLibraryDoc)
	afx_msg void OnFileExportlibrary();
	afx_msg void OnFileImportlibrary();
	afx_msg void OnContextExportsymbol();
	afx_msg void OnUpdateContextExportsymbol(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIBRARYDOC_H__642CE6F7_87BC_4A90_BB6E_CB3293813F99__INCLUDED_)
