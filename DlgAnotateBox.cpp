/*
	TinyCAD program for schematic capture
	Copyright 1994/1995/2002,2003 Matt Pyne.

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


#include "stdafx.h"
#include "TinyCadView.h"
#include "special.h"
#include "library.h"
#include "TinyCad.h"


// The anotate box dialog


BEGIN_MESSAGE_MAP( CDlgAnotateBox, CDialog )
	ON_EN_CHANGE(ANNOTATEBOX_STARTVAL, OnChangeStart)
	ON_BN_CLICKED( IDC_REF_PAINTER, OnRefPainter )
END_MESSAGE_MAP()


BOOL CDlgAnotateBox::OnInitDialog()
{
  SetDlgItemText(ANNOTATEBOX_MATCHVAL,v.matchval);
  SetDlgItemInt(ANNOTATEBOX_STARTVAL,v.startval);
  CheckRadioButton(ANNOTATEBOX_ADD,ANNOTATEBOX_REMOVE,ANNOTATEBOX_ADD+v.action);
  CheckRadioButton(ANNOTATEBOX_ALL,ANNOTATEBOX_MATCH,ANNOTATEBOX_ALL+v.reference);
  CheckRadioButton(ANNOTATEBOX_DEF,ANNOTATEBOX_START,ANNOTATEBOX_DEF+v.value);
  static_cast<CButton*>(GetDlgItem( IDC_ALL_SHEETS ))->SetCheck( v.all_sheets ? 1 : 0 );
  return TRUE;
}


void CDlgAnotateBox::ReadData()
{
  v.action = GetCheckedRadioButton(ANNOTATEBOX_ADD,ANNOTATEBOX_REMOVE)-ANNOTATEBOX_ADD;
  v.reference = GetCheckedRadioButton(ANNOTATEBOX_ALL,ANNOTATEBOX_MATCH)-ANNOTATEBOX_ALL;
  v.value = GetCheckedRadioButton(ANNOTATEBOX_DEF,ANNOTATEBOX_START)-ANNOTATEBOX_DEF;
  v.all_sheets = static_cast<CButton*>(GetDlgItem( IDC_ALL_SHEETS ))->GetCheck() != 0;
  TCHAR hold[256];
  GetDlgItemText(ANNOTATEBOX_MATCHVAL,hold,sizeof(hold));
  v.matchval = hold;
  v.startval = GetDlgItemInt(ANNOTATEBOX_STARTVAL);
}


void CDlgAnotateBox::OnOK()
{
  ReadData();
  EndDialog( IDOK );
}

void CDlgAnotateBox::OnRefPainter()
{
	ReadData();
	EndDialog( IDC_REF_PAINTER );
}

void CDlgAnotateBox::OnChangeStart()
{
  CheckRadioButton(ANNOTATEBOX_DEF,ANNOTATEBOX_START,ANNOTATEBOX_START);
}


