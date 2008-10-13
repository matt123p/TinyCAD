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

#if !defined(AFX_MULTISHEETDOC_H__EDAF3F16_D67C_494C_9A09_0F3BA98D2FD5__INCLUDED_)
#define AFX_MULTISHEETDOC_H__EDAF3F16_D67C_494C_9A09_0F3BA98D2FD5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Details.h"

// MultiSheetDoc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMultiSheetDoc document


class CMultiSheetDoc : public CDocument
{
protected:
	CMultiSheetDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CMultiSheetDoc)


public:

	// Force an autosave of the document
	virtual void AutoSave();

	// Is this document editing a library?
	virtual bool IsLibInUse( CLibraryStore *lib );

	// get the number of documents in this multi-doc
	virtual int	 GetNumberOfSheets();
	virtual void SelectSheetView( int i );
	virtual void SetActiveSheetIndex( int i );
	virtual int GetActiveSheetIndex();
	virtual CString GetSheetName( int i );
	virtual CTinyCadDoc*	GetSheet( int i );

	// Get the currently active sheet to work with
	virtual CTinyCadDoc*	GetCurrentSheet();

	virtual void OnFolderContextMenu();

	// Get the file path name during loading or saving
	virtual CString GetXMLPathName();


// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiSheetDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMultiSheetDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CMultiSheetDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTISHEETDOC_H__EDAF3F16_D67C_494C_9A09_0F3BA98D2FD5__INCLUDED_)
