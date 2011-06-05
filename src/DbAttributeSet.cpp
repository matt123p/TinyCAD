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
// DbAttributeSet.cpp : implementation file
//
#include "stdafx.h"
#include "tinycad.h"
#include "DbAttributeSet.h"
/////////////////////////////////////////////////////////////////////////////
// CDbAttributeSet
IMPLEMENT_DYNAMIC(CDbAttributeSet, CDaoRecordset)

CDbAttributeSet::CDbAttributeSet(CDaoDatabase* pdb) :
	CDaoRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CDbAttributeSet)
	m_AttributeID = 0;
	m_NameID = 0;
	m_AttName = _T("");
	m_AttValue = _T("");
	m_DisplayFlags = 0;
	m_nFields = 5;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dbOpenDynaset;
}

CString CDbAttributeSet::GetDefaultDBName()
{
	return _T("C:\\cygwin\\home\\Administrator\\tinycad\\db1.mdb");
}

CString CDbAttributeSet::GetDefaultSQL()
{
	return _T("[Attribute]");
}

void CDbAttributeSet::DoFieldExchange(CDaoFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CDbAttributeSet)
	pFX->SetFieldType(CDaoFieldExchange::outputColumn);
	DFX_Long(pFX, _T("[AttributeID]"), m_AttributeID);
	DFX_Long(pFX, _T("[NameID]"), m_NameID);
	DFX_Text(pFX, _T("[AttName]"), m_AttName);
	DFX_Text(pFX, _T("[AttValue]"), m_AttValue);
	DFX_Long(pFX, _T("[ShowAtt]"), m_DisplayFlags);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDbAttributeSet diagnostics

#ifdef _DEBUG
void CDbAttributeSet::AssertValid() const
{
	CDaoRecordset::AssertValid();
}

void CDbAttributeSet::Dump(CDumpContext& dc) const
{
	CDaoRecordset::Dump(dc);
}
#endif //_DEBUG
