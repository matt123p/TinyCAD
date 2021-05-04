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
#include "EditDlgTextEdit.h"

////// The text edit dialog box //////

CEditDlgTextEdit::CEditDlgTextEdit()
{
	opens = 0;
}

void CEditDlgTextEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditDlgTextEdit)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEditDlgTextEdit, CEditDlg)
	//{{AFX_MSG_MAP(CEditDlgTextEdit)
	ON_BN_CLICKED(IDC_TEXT_COLOUR, OnTextColour)
	ON_BN_CLICKED(IDC_TEXT_FONT, OnTextFont)
	ON_EN_CHANGE(TEXTBOX_TEXT, OnChange)
	ON_BN_CLICKED(TEXTBOX_LEFT, OnChange)
	ON_BN_CLICKED(TEXTBOX_UP,OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Create this window
void CEditDlgTextEdit::Create()
{
	CDialog::Create(IDD_TEXT, &g_EditToolBar);
}

// Open the dialog window
void CEditDlgTextEdit::Open(CTinyCadDoc *pDesign, CDrawingObject *pObject)
{
	stop = TRUE;

	Show(pDesign, pObject);

	SetDlgItemText(TEXTBOX_TEXT, static_cast<CDrawText*> (getObject())->str);

	switch (getObject()->GetType())
	{
		case xBusNameEx:
			SetDlgItemText(TEXTBOX_LABEL, _T("Bus Label Name"));
			break;
		default:
			SetDlgItemText(TEXTBOX_LABEL, _T("Text"));
			break;
	}

	if (static_cast<CDrawText*> (getObject())->dir == 3) CheckRadioButton(TEXTBOX_LEFT, TEXTBOX_UP, TEXTBOX_LEFT);
	else CheckRadioButton(TEXTBOX_LEFT, TEXTBOX_UP, TEXTBOX_UP);

	ReFocus();
	stop = FALSE;
}

void CEditDlgTextEdit::OnChange()
{
	if (stop) return;

	getObject()->Display();

	GetDlgItemText(TEXTBOX_TEXT, static_cast<CDrawText*> (getObject())->str);
	if (GetCheckedRadioButton(TEXTBOX_LEFT, TEXTBOX_UP) == TEXTBOX_LEFT) static_cast<CDrawText*> (getObject())->dir = 3;
	else static_cast<CDrawText*> (getObject())->dir = 0;

	getObject()->NewOptions();
}

void CEditDlgTextEdit::ReFocus()
{
	SetDlgItemText(TEXTBOX_TEXT, static_cast<CDrawText*> (getObject())->str);
	CEdit *ctrl = (CEdit *) GetDlgItem(TEXTBOX_TEXT);
	ctrl->SetSel(0, -1);
	ctrl->SetFocus();
}

void CEditDlgTextEdit::OnTextColour()
{
	if (CTinyCadApp::ChooseColor(static_cast<CDrawText*> (getObject())->FontColour))
	{
		getObject()->NewOptions();
	}
}

void CEditDlgTextEdit::OnTextFont()
{
	// Bring up the font dialogue...
	CClientDC dc(AfxGetMainWnd());
	m_pDesign->GetOptions()->ChooseFont(getObject()->GetType(), dc, AfxGetMainWnd()->m_hWnd);
	getObject()->NewOptions();
}
