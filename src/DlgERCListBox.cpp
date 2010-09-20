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
#include "TinyCadMultiDoc.h"


////// The ERC Box Dialog //////


BEGIN_MESSAGE_MAP( CDlgERCListBox, CDialog )
	ON_LBN_SELCHANGE(ERCLIST_LIST , OnClick )
	ON_BN_CLICKED(IDOK , Close )
END_MESSAGE_MAP()


CDlgERCListBox::CDlgERCListBox()
{
  open=FALSE;
  stop=FALSE;
}

void CDlgERCListBox::Open(CMultiSheetDoc *pDesign)
{
	m_pDesign = pDesign;

  if (!open) {
  	Create(IDD_ERCLIST,AfxGetMainWnd());

	theListBox = (CListBox *)GetDlgItem(ERCLIST_LIST);
	theListBox->ResetContent();
  }
  open = TRUE;
  stop = FALSE;
}

// Close the dialog window 
void CDlgERCListBox::Close()
{
  if (open) {
	DestroyWindow();
	// Now remove all the errors from the design
	  for (int i = 0; i < m_pDesign->GetNumberOfSheets(); i++)
	  {
		m_pDesign->GetSheet(i)->DeleteErrors();
		open = FALSE;
	  }
  }
}


void CDlgERCListBox::AddString(CString NewString)
{
  if (open) {
	theListBox = (CListBox *)GetDlgItem(ERCLIST_LIST);
	theListBox->AddString(NewString);
  }
}

void CDlgERCListBox::SetSelect(int NewSelect)
{
  if (open) {
	stop = TRUE;
	theListBox = (CListBox *)GetDlgItem(ERCLIST_LIST);
	theListBox->SetCurSel(NewSelect);
	stop = FALSE;
  }
}

void CDlgERCListBox::OnClick()
{
  // Do we notice this?
  if (stop)
	return;

  // Get the item which has been selected
  int WhichItem;
  theListBox = (CListBox *)GetDlgItem(ERCLIST_LIST);
  if ((WhichItem = theListBox->GetCurSel())==LB_ERR)
	return;

  // Now select the item in the current design (if we can...)
  m_pDesign->GetCurrentSheet()->UnSelect();

  for (int i = 0; i < m_pDesign->GetNumberOfSheets(); i++)
  {
	drawingIterator it = m_pDesign->GetSheet(i)->GetDrawingBegin();
	while (it != m_pDesign->GetSheet(i)->GetDrawingEnd()) 
	{
		CDrawingObject *pointer = *it;

		if (pointer->GetType()==xError && static_cast<CDrawError*>(pointer)->GetErrorNumber() == WhichItem)
		{
			TRACE("CDlgERCListBox::OnClick():  Successfully found the xError object to select\n");
			//The next statement appears to display the page at some arbitrary location - it does not
			//guarantee that the page will be displayed with the error object visible.
			m_pDesign->SelectSheetView( i );	//This displays the page (does nothing if page is already displayed)

			//The next statement is supposed to "select" the error object and turn it from a heavy bordered circle in dark red
			//into a heavy bordered circle in bright red, but this is not enough "motion" and not a bright enough color 
			//difference to easily spot the error marker on a busy page.  This is where a blinking error marker (when selected) 
			//could really make a difference.
			m_pDesign->GetCurrentSheet()->Select( pointer );	//This selects the object

			pointer->Display();	//This updates the page name selection tab
		}

		++ it;
	}
  }
}

