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

#if !defined(AFX_DBSYMBOLSET_H__02C4F35D_661D_491C_9153_14B6CA23D8AB__INCLUDED_)
#define AFX_DBSYMBOLSET_H__02C4F35D_661D_491C_9153_14B6CA23D8AB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DbSymbolSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDbSymbolSet DAO recordset

class CDbSymbolSet: public CDaoRecordset
{
public:
	CDbSymbolSet(CDaoDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC( CDbSymbolSet)

	// Field/Param Data
	//{{AFX_FIELD(CDbSymbolSet, CDaoRecordset)
	long m_SymbolID;
	CByteArray m_Data;
	long m_DrawRotate;
	long m_DefRotate;
	long m_Type;
	//}}AFX_FIELD

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDbSymbolSet)
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

#endif // !defined(AFX_DBSYMBOLSET_H__02C4F35D_661D_491C_9153_14B6CA23D8AB__INCLUDED_)
