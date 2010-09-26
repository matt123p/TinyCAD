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
#include "TinyCadView.h"
#include "TinyCad.h"
#include "EditDlgLabelEdit.h"


////// The text edit dialog box //////

CEditDlgLabelEdit::CEditDlgLabelEdit() 
{ 
	opens=0; 
}



void CEditDlgLabelEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditDlgLabelEdit)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEditDlgLabelEdit, CEditDlg)
	//{{AFX_MSG_MAP(CEditDlgLabelEdit)
	ON_BN_CLICKED(IDC_TEXT_COLOUR, OnTextColour)
	ON_BN_CLICKED(IDC_TEXT_FONT, OnTextFont)
	ON_BN_CLICKED(IDC_STYLE1, OnStyle1)
	ON_BN_CLICKED(IDC_STYLE2, OnStyle2)
	ON_BN_CLICKED(IDC_STYLE3, OnStyle3)
	ON_BN_CLICKED(IDC_STYLE4, OnStyle4)
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(TEXTBOX_TEXT, OnChange)
	ON_BN_CLICKED(IDC_LABEL_DIR1, OnChange)
	ON_BN_CLICKED(IDC_LABEL_DIR2, OnChange)
	ON_BN_CLICKED(IDC_LABEL_DIR3, OnChange)
	ON_BN_CLICKED(IDC_LABEL_DIR4, OnChange)
END_MESSAGE_MAP()


// Create this window
void CEditDlgLabelEdit::Create()
{
   CDialog::Create( IDD_LABEL, &g_EditToolBar );
}


// Open the dialog window
void CEditDlgLabelEdit::Open(CTinyCadDoc *pDesign, CDrawingObject *pObject)
{
  stop=TRUE;

	Show( pDesign, pObject );

  SetDlgItemText(TEXTBOX_TEXT,static_cast<CDrawLabel*>(getObject())->str);
  SetDlgItemText(TEXTBOX_LABEL,_T("Label Name"));

  CheckRadioButton(IDC_LABEL_DIR1,IDC_LABEL_DIR4,IDC_LABEL_DIR1 + static_cast<CDrawLabel*>(getObject())->dir);
  CheckRadioButton(IDC_STYLE1,IDC_STYLE4,IDC_STYLE1 + static_cast<CDrawLabel*>(getObject())->m_Style);	

  ReFocus();

  stop=FALSE;  
}


void CEditDlgLabelEdit::OnChange()
{
  TCHAR str[SIZESTRING];

  if (stop)
	return;

  if (!getObject())
  {
	  return;
  }
  getObject()->Display();

  GetDlgItemText(TEXTBOX_TEXT, str, sizeof(str) );
  static_cast<CDrawLabel*>(getObject())->str = str;
  static_cast<CDrawLabel*>(getObject())->dir = (BYTE) (GetCheckedRadioButton(IDC_LABEL_DIR1,IDC_LABEL_DIR4) - IDC_LABEL_DIR1);

  int style = GetCheckedRadioButton(IDC_STYLE1,IDC_STYLE4) - IDC_STYLE1;
  static_cast<CDrawLabel*>(getObject())->m_Style = static_cast<CDrawLabel::label_style>(style);

  getObject()->NewOptions();
}

void CEditDlgLabelEdit::ReFocus()
{
	SetDlgItemText(TEXTBOX_TEXT, static_cast<CDrawLabel*>(getObject())->str);
	CEdit *ctrl = (CEdit *)GetDlgItem( TEXTBOX_TEXT );
	ctrl->SetSel(0, -1);
	ctrl->SetFocus();
}


void CEditDlgLabelEdit::OnTextColour() 
{
	if (CTinyCadApp::ChooseColor( static_cast<CDrawLabel*>(getObject())->FontColour ))
	{
		getObject()->NewOptions();	
	}
}

void CEditDlgLabelEdit::OnTextFont() 
{
	// Bring up the font dialogue...
	CClientDC dc(AfxGetMainWnd());
	m_pDesign->GetOptions()->ChooseFont(getObject()->GetType(), dc,AfxGetMainWnd()->m_hWnd);
	getObject()->NewOptions();	
}

void CEditDlgLabelEdit::OnStyle1() 
{
	OnChange();
}

void CEditDlgLabelEdit::OnStyle2() 
{
	OnChange();
}

void CEditDlgLabelEdit::OnStyle3() 
{
	OnChange();
}

void CEditDlgLabelEdit::OnStyle4() 
{
	OnChange();
}
