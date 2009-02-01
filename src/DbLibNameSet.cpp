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

// DbLibNameSet.cpp : implementation file
//

#include "stdafx.h"
#include "tinycad.h"
#include "DbLibNameSet.h"



/////////////////////////////////////////////////////////////////////////////
// CDbLibNameSet

IMPLEMENT_DYNAMIC(CDbLibNameSet, CDaoRecordset)

CDbLibNameSet::CDbLibNameSet(CDaoDatabase* pdb)
	: CDaoRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CDbLibNameSet)
	m_NameID = 0;
	m_Name = _T("");
	m_SymbolID = 0;
	m_Type = 0;
	m_Reference = _T("");
	m_ppp = 0;
	m_Description = _T("");
	m_ShowName = 0;
	m_ShowRef = 0;
	m_nFields = 9;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dbOpenDynaset;
}


CString CDbLibNameSet::GetDefaultDBName()
{
	return _T("C:\\cygwin\\home\\Administrator\\tinycad\\db1.mdb");
}

CString CDbLibNameSet::GetDefaultSQL()
{
	return _T("[Name]");
}

void CDbLibNameSet::DoFieldExchange(CDaoFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CDbLibNameSet)
	pFX->SetFieldType(CDaoFieldExchange::outputColumn);
	DFX_Long(pFX, _T("[NameID]"), m_NameID);
	DFX_Text(pFX, _T("[Name]"), m_Name);
	DFX_Long(pFX, _T("[SymbolID]"), m_SymbolID);
	DFX_Long(pFX, _T("[Type]"), m_Type);
	DFX_Text(pFX, _T("[Reference]"), m_Reference);
	DFX_Long(pFX, _T("[ppp]"), m_ppp);
	DFX_Text(pFX, _T("[Description]"), m_Description);
	DFX_Long(pFX, _T("[ShowName]"), m_ShowName);
	DFX_Long(pFX, _T("[ShowRef]"), m_ShowRef);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDbLibNameSet diagnostics

#ifdef _DEBUG
void CDbLibNameSet::AssertValid() const
{
	CDaoRecordset::AssertValid();
}

void CDbLibNameSet::Dump(CDumpContext& dc) const
{
	CDaoRecordset::Dump(dc);
}
#endif //_DEBUG
