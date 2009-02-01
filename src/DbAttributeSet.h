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

#if !defined(AFX_DBATTRIBUTESET_H__78974206_8086_4A87_B7EC_23171114100C__INCLUDED_)
#define AFX_DBATTRIBUTESET_H__78974206_8086_4A87_B7EC_23171114100C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DbAttributeSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDbAttributeSet DAO recordset

class CDbAttributeSet : public CDaoRecordset
{
public:
	CDbAttributeSet(CDaoDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CDbAttributeSet)

// Field/Param Data
	//{{AFX_FIELD(CDbAttributeSet, CDaoRecordset)
	long	m_AttributeID;
	long	m_NameID;
	CString	m_AttName;
	CString	m_AttValue;
	long	m_DisplayFlags;
	//}}AFX_FIELD

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDbAttributeSet)
	public:
	virtual CString GetDefaultDBName();		// Default database name
	virtual CString GetDefaultSQL();		// Default SQL for Recordset
	virtual void DoFieldExchange(CDaoFieldExchange* pFX);  // RFX support
	//}}AFX_VIRTUAL

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DBATTRIBUTESET_H__78974206_8086_4A87_B7EC_23171114100C__INCLUDED_)
