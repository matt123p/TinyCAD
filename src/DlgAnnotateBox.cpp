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
#include "TinyCadRegistry.h"

// The annotate box dialog


BEGIN_MESSAGE_MAP( CDlgAnnotateBox, CDialog )
	ON_EN_CHANGE(ANNOTATEBOX_STARTVAL, OnChangeStart)
	ON_BN_CLICKED( IDC_REF_PAINTER, OnRefPainter )
END_MESSAGE_MAP()


BOOL CDlgAnnotateBox::OnInitDialog()
{
  //Some default values for this dialog are stored in the Registry
  v.reference = (enum AnnotateSetup::WhichReferences) CTinyCadRegistry::GetInt( "Annotate.type", (int) AnnotateSetup::UNNUMBERED );
  v.all_sheets = CTinyCadRegistry::GetInt( "Annotate.allsheets", (int) true) != 0;

  SetDlgItemText(ANNOTATEBOX_MATCHVAL,v.matchval);
  SetDlgItemInt(ANNOTATEBOX_STARTVAL,v.startval);
  int id = -1;
  switch (v.action)
  {
  case AnnotateSetup::ADD:
	  id = ANNOTATEBOX_ADD;
	  break;
  case AnnotateSetup::REMOVE:
	  id = ANNOTATEBOX_REMOVE;
	  break;
  }
  if (id >= 0)
  {
	CheckRadioButton(ANNOTATEBOX_ADD,ANNOTATEBOX_REMOVE,id);
  }

  id = -1;
  switch (v.reference)
  {
  case AnnotateSetup::ALL:
	  id = ANNOTATEBOX_ALL;
	  break;
  case AnnotateSetup::MATCHING:
	  id = ANNOTATEBOX_MATCH;
	  break;
  case AnnotateSetup::UNNUMBERED:
	  id = ANNOTATEBOX_UNNUM;
	  break;
  }
  if (id >= 0)
  {
	CheckRadioButton(ANNOTATEBOX_ALL,ANNOTATEBOX_MATCH,id);
  }

  id = -1;
  switch (v.value)
  {
  case AnnotateSetup::DEFAULT:
	  id = ANNOTATEBOX_DEF;
	  break;
  case AnnotateSetup::SPECIFIED:
	  id = ANNOTATEBOX_START;
	  break;
  }
  if (id >= 0)
  {
	CheckRadioButton(ANNOTATEBOX_DEF,ANNOTATEBOX_START,id);
  }
  static_cast<CButton*>(GetDlgItem( IDC_ALL_SHEETS ))->SetCheck( v.all_sheets ? 1 : 0 );
  return TRUE;
}


void CDlgAnnotateBox::ReadData()
{
	switch (GetCheckedRadioButton(ANNOTATEBOX_ADD,ANNOTATEBOX_REMOVE))
	{
	case ANNOTATEBOX_ADD:
		v.action = AnnotateSetup::ADD;
		break;
	case ANNOTATEBOX_REMOVE:
		v.action = AnnotateSetup::REMOVE;
		break;
	default:
		// what do we do here?
		;
	}

	switch (GetCheckedRadioButton(ANNOTATEBOX_ALL,ANNOTATEBOX_MATCH))
	{
	case ANNOTATEBOX_ALL:
		v.reference = AnnotateSetup::ALL;
		break;
	case ANNOTATEBOX_UNNUM:
		v.reference = AnnotateSetup::UNNUMBERED;
		break;
	case ANNOTATEBOX_MATCH:
		v.reference = AnnotateSetup::MATCHING;
		break;
	default:
		// what do we do here?
		;
	}

	switch (GetCheckedRadioButton(ANNOTATEBOX_DEF,ANNOTATEBOX_START))
	{
	case ANNOTATEBOX_DEF:
		v.value = AnnotateSetup::DEFAULT;
		break;
	case ANNOTATEBOX_START:
		v.value = AnnotateSetup::SPECIFIED;
		break;
	default:
		// what do we do here?
		;
	}
  v.all_sheets = static_cast<CButton*>(GetDlgItem( IDC_ALL_SHEETS ))->GetCheck() != 0;
  TCHAR hold[256];
  GetDlgItemText(ANNOTATEBOX_MATCHVAL,hold,sizeof(hold));
  v.matchval = hold;
  v.startval = GetDlgItemInt(ANNOTATEBOX_STARTVAL);
}


void CDlgAnnotateBox::OnOK()
{
  ReadData();

  CTinyCadRegistry::Set( "Annotate.type", v.reference);
  CTinyCadRegistry::Set( "Annotate.allsheets", v.all_sheets);
  EndDialog( IDOK );
}

void CDlgAnnotateBox::OnRefPainter()
{
	ReadData();
	EndDialog( IDC_REF_PAINTER );
}

void CDlgAnnotateBox::OnChangeStart()
{
  CheckRadioButton(ANNOTATEBOX_DEF,ANNOTATEBOX_START,ANNOTATEBOX_START);
}


