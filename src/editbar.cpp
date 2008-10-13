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
#include "diag.h"





void CEditDlg::Show( CTinyCadDoc *pDesign, CDrawingObject *pObject )
{
	m_pDesign = pDesign;
	m_pObject = pObject;

  if (++opens==1) 
  {
	  g_EditToolBar.setDlg( this );
  }

}

void CEditDlg::Close()
{
  if (--opens==0)
  {
	  g_EditToolBar.unsetDlg();
	  ShowWindow( SW_HIDE );
  }
}


CDrawingObject *CEditDlg::getObject()
{
	
	return m_pObject;
}


void CEditDlg::changeSelected( CDrawingObject *previous, CDrawingObject *pObject )
{
	if (previous == m_pObject)
	{
		m_pObject = pObject;
	}
}

////// The no tool dialog box //////

void CEditDlgNoTool::Create()
{
	CDialog::Create( IDD_NO_TOOL, &g_EditToolBar );
}



////// The Power editing Dialog //////

BEGIN_MESSAGE_MAP( CEditDlgPowerEdit, CEditDlg )
	ON_EN_CHANGE(POWERBOX_TEXT, OnChange)
	ON_BN_CLICKED(POWERBOX_TOP, OnChange)
	ON_BN_CLICKED(POWERBOX_BOTTOM, OnChange)
	ON_BN_CLICKED(POWERBOX_LEFT, OnChange)
	ON_BN_CLICKED(POWERBOX_RIGHT, OnChange)
	ON_BN_CLICKED(POWERBOX_WAVE, OnChange)
	ON_BN_CLICKED(POWERBOX_CIRCLE, OnChange)
	ON_BN_CLICKED(POWERBOX_ARROW, OnChange)
	ON_BN_CLICKED(POWERBOX_BAR, OnChange)
	ON_BN_CLICKED(POWERBOX_EARTH, OnChange)
END_MESSAGE_MAP()

// Create this window
void CEditDlgPowerEdit::Create()
{
	  CDialog::Create( IDD_POWER, &g_EditToolBar );
}

void CEditDlgPowerEdit::Open(CTinyCadDoc *pDesign, CDrawingObject *pObject)
{
  stop=TRUE;
  
	Show( pDesign, pObject );
  SetDlgItemText(POWERBOX_TEXT,static_cast<CDrawPower*>(getObject())->str);
  CheckRadioButton(POWERBOX_TOP,POWERBOX_RIGHT,(static_cast<CDrawPower*>(getObject())->dir)+POWERBOX_TOP);
  CheckRadioButton(POWERBOX_BAR,POWERBOX_EARTH,(static_cast<CDrawPower*>(getObject())->which)+POWERBOX_BAR);

  stop=FALSE;
}


void CEditDlgPowerEdit::OnChange()
{
  TCHAR str[SIZESTRING];
  if (stop)
	return;

  getObject()->Display();

  GetDlgItemText(POWERBOX_TEXT, str, sizeof(str) );
  static_cast<CDrawPower*>(getObject())->str = str;

  static_cast<CDrawPower*>(getObject())->dir=GetCheckedRadioButton(POWERBOX_TOP,POWERBOX_RIGHT)-POWERBOX_TOP;
  static_cast<CDrawPower*>(getObject())->which=GetCheckedRadioButton(POWERBOX_BAR,POWERBOX_EARTH)-POWERBOX_BAR;

  getObject()->Display();
}






// The Rotate Dialog

BEGIN_MESSAGE_MAP( CEditDlgRotateBox, CEditDlg )
	ON_BN_CLICKED(ROTATEBLOCK_LEFT, OnLeft)
	ON_BN_CLICKED(ROTATEBLOCK_RIGHT, OnRight)
	ON_BN_CLICKED(ROTATEBLOCK_MIRROR, OnMirror)	
END_MESSAGE_MAP()


// Create this window
void CEditDlgRotateBox::Create()
{
	  CDialog::Create( IDD_ROTATE, &g_EditToolBar );
}

void CEditDlgRotateBox::Open(CTinyCadDoc *pDesign, CDrawingObject *pObject)
{

	Show( pDesign, pObject );
}



void CEditDlgRotateBox::OnLeft()
{
  getObject()->Display();
  static_cast<CDrawBlockRotate*>(getObject())->ChangeDir(3);
  getObject()->Display();
}

void CEditDlgRotateBox::OnRight()
{
  getObject()->Display();
  static_cast<CDrawBlockRotate*>(getObject())->ChangeDir(2);
  getObject()->Display();
}

void CEditDlgRotateBox::OnMirror()
{
  getObject()->Display();
  static_cast<CDrawBlockRotate*>(getObject())->ChangeDir(4);
  getObject()->Display();
}




