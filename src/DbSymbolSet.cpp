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

// DbSymbolSet.cpp : implementation file
//

#include "stdafx.h"
#include "tinycad.h"
#include "DbSymbolSet.h"


/////////////////////////////////////////////////////////////////////////////
// CDbSymbolSet

IMPLEMENT_DYNAMIC(CDbSymbolSet, CDaoRecordset)

CDbSymbolSet::CDbSymbolSet(CDaoDatabase* pdb)
	: CDaoRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CDbSymbolSet)
	m_SymbolID = 0;
	m_DrawRotate = 0;
	m_DefRotate = 0;
	m_Type = 0;
	m_nFields = 5;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dbOpenDynaset;
}


CString CDbSymbolSet::GetDefaultDBName()
{
	return _T("C:\\cygwin\\home\\Administrator\\tinycad\\db1.mdb");
}

CString CDbSymbolSet::GetDefaultSQL()
{
	return _T("[Symbol]");
}

void CDbSymbolSet::DoFieldExchange(CDaoFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CDbSymbolSet)
	pFX->SetFieldType(CDaoFieldExchange::outputColumn);
	DFX_Long(pFX, _T("[SymbolID]"), m_SymbolID);
	DFX_Binary(pFX, _T("[Data]"), m_Data, 1024);
	DFX_Long(pFX, _T("[DrawRotate]"), m_DrawRotate);
	DFX_Long(pFX, _T("[DefRotate]"), m_DefRotate);
	DFX_Long(pFX, _T("[Type]"), m_Type);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDbSymbolSet diagnostics

#ifdef _DEBUG
void CDbSymbolSet::AssertValid() const
{
	CDaoRecordset::AssertValid();
}

void CDbSymbolSet::Dump(CDumpContext& dc) const
{
	CDaoRecordset::Dump(dc);
}
#endif //_DEBUG
