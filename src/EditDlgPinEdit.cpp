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
#include "option.h"
#include "revision.h"
#include "registry.h"
#include "TinyCadView.h"
#include "TinyCad.h"
#include "MainFrm.h"
#include "EditDlgPinEdit.h"


CEditDlgPinEdit::CEditDlgPinEdit()
{
  opens=0;
  stop=TRUE;
	//{{AFX_DATA_INIT(CEditDlgPinEdit)
	//}}AFX_DATA_INIT
}

void CEditDlgPinEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditDlgPinEdit)
	DDX_Control(pDX, IDC_PIN_TYPE, m_Which);
	DDX_Control(pDX, IDC_ELEC, m_Elec);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEditDlgPinEdit, CEditDlg)
	//{{AFX_MSG_MAP(CEditDlgPinEdit)
	ON_CBN_SELCHANGE(IDC_ELEC, OnSelchangeElec)
	ON_CBN_SELCHANGE(IDC_PIN_TYPE, OnSelchangePinType)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CENTRE_NAME, OnChange)
	ON_EN_CHANGE(PINBOX_NAME, OnChange)
	ON_EN_CHANGE(PINBOX_NUMBER, OnChange)
	ON_EN_CHANGE(PINBOX_LENGTH, OnChange)
	ON_EN_CHANGE(PINBOX_PIN_POS, OnChange)
	ON_BN_CLICKED(PINBOX_NAMEON, OnChange)
	ON_BN_CLICKED(PINBOX_NUMBERON, OnChange)
	ON_BN_CLICKED(PINBOX_UP, OnChange)
	ON_BN_CLICKED(PINBOX_DOWN, OnChange)
	ON_BN_CLICKED(PINBOX_LEFT, OnChange)
	ON_BN_CLICKED(PINBOX_RIGHT, OnChange)
	ON_BN_CLICKED(PINBOX_NORMAL, OnChange)
	ON_BN_CLICKED(PINBOX_DOT, OnChange)
	ON_BN_CLICKED(PINBOX_CLOCK, OnChange)
	ON_BN_CLICKED(PINBOX_DCLOCK, OnChange)
	ON_BN_CLICKED(PINBOX_POWER, OnChange)
	ON_BN_CLICKED(PINBOX_HIDDEN, OnChange)
	ON_BN_CLICKED(PINBOX_INPUT, OnChange)
	ON_BN_CLICKED(PINBOX_OUTPUT, OnChange)
	ON_BN_CLICKED(PINBOX_BIDIR, OnChange)
	ON_BN_CLICKED(PINBOX_PASSIVE, OnChange)
	ON_BN_CLICKED(PINBOX_TRISTATE, OnChange)
END_MESSAGE_MAP()



// Create this window
void CEditDlgPinEdit::Create()
{
	CDialog::Create( IDD_PIN, &g_EditToolBar );

	m_Which.ResetContent();
	m_Which.AddString( _T("Normal") );
	m_Which.AddString( _T("Dot") );
	m_Which.AddString( _T("Clock") );
	m_Which.AddString( _T("Dot Clock") );
	m_Which.AddString( _T("Power") );
	m_Which.AddString( _T("Hidden") );
	m_Which.AddString( _T("Cross") );

	m_Elec.ResetContent();
	m_Elec.AddString( _T("Input") );
	m_Elec.AddString( _T("Output") );
	m_Elec.AddString( _T("Tristate") );
	m_Elec.AddString( _T("Open Collector") );
	m_Elec.AddString( _T("Passive") );
	m_Elec.AddString( _T("Input/Output") );
	m_Elec.AddString( _T("Not Connected") );

}

void CEditDlgPinEdit::Open(CTinyCadDoc *pDesign, CDrawingObject *pObject)
{
	stop=TRUE;

	Show( pDesign, pObject );

	BOOL IsHierarchicalPin = static_cast<CDrawPin*>(getObject())->IsHierarchicalPin();

	GetDlgItem( PINBOX_NUMBER )->EnableWindow( !IsHierarchicalPin );
	GetDlgItem( PINBOX_NUMBERON )->EnableWindow( !IsHierarchicalPin );

	SetDlgItemText(PINBOX_NAME,static_cast<CDrawPin*>(getObject())->m_str);
	SetDlgItemText(PINBOX_NUMBER,static_cast<CDrawPin*>(getObject())->m_number);
	SetDlgItemInt(PINBOX_LENGTH,static_cast<CDrawPin*>(getObject())->m_length);
	SetDlgItemInt(PINBOX_PIN_POS,static_cast<CDrawPin*>(getObject())->m_number_pos);

	CheckRadioButton(PINBOX_UP,PINBOX_RIGHT,static_cast<CDrawPin*>(getObject())->m_dir+PINBOX_UP);

	m_Which.SetCurSel( static_cast<CDrawPin*>(getObject())->m_which );
	m_Elec.SetCurSel( static_cast<CDrawPin*>(getObject())->m_elec );

	CheckDlgButton(PINBOX_NAMEON,  ((static_cast<CDrawPin*>(getObject())->m_show)&1)!=0);
	CheckDlgButton(PINBOX_NUMBERON,((static_cast<CDrawPin*>(getObject())->m_show)&2)!=0 && !IsHierarchicalPin);
	CheckDlgButton(IDC_CENTRE_NAME, static_cast<CDrawPin*>(getObject())->m_centre_name );

	ReFocus();

	stop=FALSE;
}



void CEditDlgPinEdit::OnChange()
{
  TCHAR str[SIZESTRING];

  if (stop)
	return;

  getObject()->Display();

  // Read in the name
  GetDlgItemText(PINBOX_NAME, str, sizeof(str) );
  static_cast<CDrawPin*>(getObject())->m_str=str;

  GetDlgItemText(PINBOX_NUMBER, str, sizeof(str) );
  static_cast<CDrawPin*>(getObject())->m_number=str;

  static_cast<CDrawPin*>(getObject())->m_show  = IsDlgButtonChecked(PINBOX_NAMEON)!=0 ? 1 : 0;
  static_cast<CDrawPin*>(getObject())->m_show |= IsDlgButtonChecked(PINBOX_NUMBERON)!=0 ? 2 : 0;
  static_cast<CDrawPin*>(getObject())->m_centre_name = IsDlgButtonChecked(IDC_CENTRE_NAME)!=0;

  static_cast<CDrawPin*>(getObject())->m_dir   = (BYTE) (GetCheckedRadioButton(PINBOX_UP,PINBOX_RIGHT)-PINBOX_UP);

  int length = GetDlgItemInt(PINBOX_LENGTH);
  if (length != 0) {
      length = max(5,length);
  }
  //  int length = max(5,GetDlgItemInt(PINBOX_LENGTH));
  int number_pos = min(length, (int) GetDlgItemInt(PINBOX_PIN_POS ) );

  static_cast<CDrawPin*>(getObject())->m_length = (WORD) length;
  static_cast<CDrawPin*>(getObject())->m_number_pos = number_pos;

  getObject()->m_pDesign->GetOptions()->SetPinLength( length );
  getObject()->m_pDesign->GetOptions()->SetPinNumberPos( number_pos );

  getObject()->Display();
}



void CEditDlgPinEdit::ReFocus()
{
  stop = TRUE;

  SetDlgItemText(PINBOX_NAME,static_cast<CDrawPin*>(getObject())->m_str);
  SetDlgItemText(PINBOX_NUMBER,static_cast<CDrawPin*>(getObject())->m_number);

  CEdit *ctrl = (CEdit *)GetDlgItem( PINBOX_NAME );
  ctrl->SetSel(0, -1);
  ctrl->SetFocus();

  stop = FALSE;
}



void CEditDlgPinEdit::OnSelchangeElec() 
{
	static_cast<CDrawPin*>(getObject())->m_elec = (BYTE) m_Elec.GetCurSel();	
    getObject()->Display();
}

void CEditDlgPinEdit::OnSelchangePinType() 
{
	static_cast<CDrawPin*>(getObject())->m_which = (BYTE) m_Which.GetCurSel();
    getObject()->Display();
}
