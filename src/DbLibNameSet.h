/*
 TinyCAD program for schematic capture
 Copyright 1994-2004 Matt Pyne.

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

#if !defined(AFX_DBLIBNAMESET_H__66FF493B_7051_4465_9C80_502F7759D977__INCLUDED_)
#define AFX_DBLIBNAMESET_H__66FF493B_7051_4465_9C80_502F7759D977__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DbLibNameSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDbLibNameSet DAO recordset

class CDbLibNameSet: public CDaoRecordset
{
public:
	CDbLibNameSet(CDaoDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC( CDbLibNameSet)

	// Field/Param Data
	//{{AFX_FIELD(CDbLibNameSet, CDaoRecordset)
	long m_NameID;
	CString m_Name;
	long m_SymbolID;
	long m_Type;
	CString m_Reference;
	long m_ppp;
	CString m_Description;
	long m_ShowName;
	long m_ShowRef;
	//}}AFX_FIELD

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDbLibNameSet)
public:
	virtual CString GetDefaultDBName(); // Default database name
	virtual CString GetDefaultSQL(); // Default SQL for Recordset
	virtual void DoFieldExchange(CDaoFieldExchange* pFX); // RFX support
	//}}AFX_VIRTUAL

	// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DBLIBNAMESET_H__66FF493B_7051_4465_9C80_502F7759D977__INCLUDED_)
