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
	ON_BN_CLICKED(IDOK2 , ReCheck )
END_MESSAGE_MAP()


CDlgERCListBox::CDlgERCListBox()
{
	open=FALSE;
	stop=FALSE;
}

void CDlgERCListBox::Open(CMultiSheetDoc *pDesign, CTinyCadView *pView)
{
	m_pDesign = pDesign;
	m_pView = pView;

	if (!open) {
		Create(IDD_ERCLIST,AfxGetMainWnd());

		if (!m_pView) {
			CWnd *wnd = GetDlgItem (IDOK2);
			wnd->ShowWindow(SW_HIDE);
		}
	}
	theListBox = (CListBox *)GetDlgItem(ERCLIST_LIST);
	theListBox->ResetContent();

	open = TRUE;
	stop = FALSE;
}

// Close the dialog window 
void CDlgERCListBox::Close()
{
	if (open) {
		DestroyWindow();
		// Remove all previous errors from the design
		for (int i = 0; i < m_pDesign->GetNumberOfSheets(); i++)
		{
			m_pDesign->GetSheet(i)->DeleteErrors();
		}
		open = FALSE;
	}
}

// Default cancel button must close the dialog window 
void CDlgERCListBox::OnCancel()
{
	CDlgERCListBox::Close();
}


// Re-check for ERC errors
void CDlgERCListBox::ReCheck()
{
	if (open) {
		// Remove all previous errors from the design
		for (int i = 0; i < m_pDesign->GetNumberOfSheets(); i++)
		{
			m_pDesign->GetSheet(i)->DeleteErrors();
		}

		if (m_pView)
		{
			m_pView->DoSpecialCheck();
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

/**
 * This is the "browser" function for browsing through the ERC markers and associated messages.
 */
void CDlgERCListBox::OnClick()
{
	// Do we notice this?
	if (stop) return;

	// Get the item which has been selected
	int WhichItem;
	theListBox = (CListBox *)GetDlgItem(ERCLIST_LIST);
	if ((WhichItem = theListBox->GetCurSel())==LB_ERR) {
		return;
	}

	// Now select the item in the current design (if we can...)
	m_pDesign->GetCurrentSheet()->UnSelect();

	for (int sheetIndex = 0; sheetIndex < m_pDesign->GetNumberOfSheets(); sheetIndex++)
	{
		drawingIterator it = m_pDesign->GetSheet(sheetIndex)->GetDrawingBegin();
		while (it != m_pDesign->GetSheet(sheetIndex)->GetDrawingEnd()) 
		{
			CDrawingObject *ercObject = *it;

			if (ercObject->GetType()==xError && static_cast<CDrawError*>(ercObject)->GetErrorNumber() == WhichItem)
			{
				//The next statement displays the page with the ERC marker centered in view and at a zoom factor that is still under the user's control
				m_pDesign->SelectERCSheetView( sheetIndex, ercObject );	//This displays a view centered around the ERC marker.

				//The next statement selects the error object so that it turns from a heavy bordered circle
				//in brown into a heavy bordered circle in bright red
				m_pDesign->GetCurrentSheet()->Select( ercObject );	//This selects the object

				//Sometimes there is more than one ERC marker at the same spot - it won't appear highlighted 
				//to the user unless it is on top!
				//Note:	Because BringToFront() changes the ordering of objects in the drawing, this action will
				//invalidate the iterator being used to traverse this list!  You must quit iterating this list
				//after bringing this object to the front!
				m_pDesign->GetCurrentSheet()->BringToFront();

				ercObject->Display();	//This draws the object and also updates the page name selection tab
				break;	//While logically appropriate, this statement is also required to avoid crashing when incrementing the iterator which is now invalid
			}

			++ it;
		}
	}
}
