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

#if !defined(AFX_TINYCADMULTISYMBOLDOC_H__0E0AA344_4EE5_4EA5_9A11_05B0033DC1C0__INCLUDED_)
#define AFX_TINYCADMULTISYMBOLDOC_H__0E0AA344_4EE5_4EA5_9A11_05B0033DC1C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TinyCadMultiSymbolDoc.h : header file
//

#include "MultiSheetDoc.h"
#include "TinyCadSymbolDoc.h"


/////////////////////////////////////////////////////////////////////////////
// CTinyCadMultiSymbolDoc document

class CTinyCadMultiSymbolDoc : public CMultiSheetDoc
{
protected:
	DECLARE_DYNCREATE(CTinyCadMultiSymbolDoc)	
	/*
		Homogeneous parts are those in which all the parts in the package 
		have identical symbols. Heterogeneous parts have different symbols 
		within the same package.

		Historically TinyCAD has only had homogeneous parts so if the symbol
		is homogeneous then we use just a single document (which is the first
		and only document in the collection).

		If the symbol is heterogeneous, then we use the collection with one document
		per symbol outline.

	*/
	bool				m_heterogeneous;
	typedef std::vector<CTinyCadSymbolDoc*>		sheetCollection;
	sheetCollection		m_symbols;

	// The library currently being edited
	CLibraryStore*				m_libedit;				
	CLibraryStoreNameSet		m_symboledit;

	int					m_ppp;
	int					m_current_index;

	// Store this symbol back to the library
	BOOL Store();
	void SaveXML( CXMLWriter &xml );
	void LoadXML( CXMLReader &xml, bool heterogeneous );

public:

	CTinyCadMultiSymbolDoc();       
	CTinyCadMultiSymbolDoc(CLibraryStore* pLib, CLibraryStoreNameSet &symbol );

	// Is this document editing a library?
	virtual bool IsLibInUse( CLibraryStore *lib );

	// get the number of documents in this multi-doc
	virtual int	 GetNumberOfSheets();
	virtual void SetActiveSheetIndex( int i );
	virtual int GetActiveSheetIndex();
	virtual CString GetSheetName( int i );
	CTinyCadDoc* GetActiveSheet();

	CLibraryStoreNameSet *getSymbol() { return &m_symboledit; }


	// Get the currently active sheet to work with
	virtual CTinyCadDoc*	GetSheet(int i);

	virtual void OnFolderContextMenu();

	virtual BOOL CanCloseFrame(CFrameWnd* pFrameArg);


// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTinyCadMultiSymbolDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTinyCadMultiSymbolDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CTinyCadMultiSymbolDoc)
	afx_msg void OnLibraryAddpin();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnLibrarySetpartsperpackage();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLibraryHeterogeneous();
	afx_msg void OnUpdateLibraryHeterogeneous(CCmdUI *pCmdUI);
	afx_msg void OnLibraryHomogeneous();
	afx_msg void OnUpdateLibraryHomogeneous(CCmdUI *pCmdUI);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TINYCADMULTISYMBOLDOC_H__0E0AA344_4EE5_4EA5_9A11_05B0033DC1C0__INCLUDED_)
