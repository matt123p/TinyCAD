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
#include "TinyCadRegistry.h"
#include "TinyCadView.h"
#include "TinyCad.h"
#include "MainFrm.h"
#include "diag.h"

////// The Page Size dialog Box //////
BEGIN_MESSAGE_MAP( CDlgPageSizeBox, CDialog )
	ON_BN_CLICKED(PAGESIZE_A4, OnChange )
	ON_BN_CLICKED(PAGESIZE_A3, OnChange )
	ON_BN_CLICKED(PAGESIZE_A2, OnChange )
	ON_BN_CLICKED(PAGESIZE_A1, OnChange )
	ON_BN_CLICKED(PAGESIZE_LETTER, OnChange)
	ON_BN_CLICKED(PAGESIZE_LEGAL, OnChange)
	ON_BN_CLICKED(PAGESIZE_LEDGER, OnChange)
	ON_BN_CLICKED(PAGESIZE_PRINTER, OnChange )
	ON_BN_CLICKED(PAGESIZE_PORT, OnMakePort )
	ON_BN_CLICKED(PAGESIZE_LAND, OnMakeLand )
	ON_BN_CLICKED(PAGESIZE_PRINTERSET, OnPrinterSet )
	ON_EN_CHANGE(PAGESIZE_WIDTH, OnChangeText )
	ON_EN_CHANGE(PAGESIZE_HEIGHT, OnChangeText )
END_MESSAGE_MAP()

// Arrays storing the sizes of the pre-defined pages
//					  A4   A3   A2   A1	 Letter  Legal  Ledger	Printer Default
int Page_SizesA[] = { 297, 420, 594, 840, 216,   216,   280,	297 };
int Page_SizesB[] = { 210, 297, 420, 594, 280,	 356, 	432,    210 };
const int NumberOfSizes = _countof(Page_SizesA);

BOOL CDlgPageSizeBox::SetPrinter(BOOL display_dialogue)
{
	HDC hdc = NULL;

	// Get the dimensions of the default printer's page
	CPrintDialog pdlg(FALSE, 0, AfxGetMainWnd());

	// Create a printer dialogue
	if (display_dialogue)
	{
		if (pdlg.DoModal())
		{
			hdc = pdlg.GetPrinterDC();
		}
	}
	else
	{
		if (AfxGetApp()->GetPrinterDeviceDefaults(&pdlg.m_pd))
		{
			if (pdlg.m_pd.hDC == NULL)
			{
				// call CreatePrinterDC if DC was not created by above
				hdc = pdlg.CreatePrinterDC();
			}
		}
	}

	if (hdc)
	{
		// Change the scaling to print correct size
		int x = ::GetDeviceCaps(hdc, HORZSIZE);
		int y = ::GetDeviceCaps(hdc, VERTSIZE);
		Page_SizesA[NumberOfSizes - 1] = max(x,y);
		Page_SizesB[NumberOfSizes - 1] = min(x,y);
	}

	::DeleteDC(hdc);

	return hdc != NULL;
}

BOOL CDlgPageSizeBox::OnInitDialog()
{
	PageSizeChanged = FALSE;
	PageSetupChanged = FALSE;

	SetPrinter(FALSE);

	// Set the correct settings in the dialog box
	SetDlgItemInt(PAGESIZE_WIDTH, Size.x / PIXELSPERMM);
	SetDlgItemInt(PAGESIZE_HEIGHT, Size.y / PIXELSPERMM);

	return TRUE;
}

void CDlgPageSizeBox::OnPrinterSet()
{

	if (SetPrinter(TRUE))
	{
		// Update the dialog to reflect any changes
		CheckRadioButton(PAGESIZE_A4, PAGESIZE_PRINTER, PAGESIZE_PRINTER);
		OnChange();

		PageSetupChanged = TRUE;
	}
}

void CDlgPageSizeBox::OnMakePort()
{
	TCHAR BufferWidth[SIZESTRING], BufferHeight[SIZESTRING];

	// If it is not already portrait then swap width and height
	if (GetDlgItemInt(PAGESIZE_WIDTH) > GetDlgItemInt(PAGESIZE_HEIGHT))
	{
		GetDlgItemText(PAGESIZE_WIDTH, BufferWidth, SIZESTRING);
		GetDlgItemText(PAGESIZE_HEIGHT, BufferHeight, SIZESTRING);
		SetDlgItemText(PAGESIZE_HEIGHT, BufferWidth);
		SetDlgItemText(PAGESIZE_WIDTH, BufferHeight);
	}

	PageSizeChanged = TRUE;
}

void CDlgPageSizeBox::OnMakeLand()
{
	TCHAR BufferWidth[SIZESTRING], BufferHeight[SIZESTRING];

	// If it is not already landscape then swap width and height
	if (GetDlgItemInt(PAGESIZE_WIDTH) < GetDlgItemInt(PAGESIZE_HEIGHT))
	{
		GetDlgItemText(PAGESIZE_WIDTH, BufferWidth, SIZESTRING);
		GetDlgItemText(PAGESIZE_HEIGHT, BufferHeight, SIZESTRING);
		SetDlgItemText(PAGESIZE_HEIGHT, BufferWidth);
		SetDlgItemText(PAGESIZE_WIDTH, BufferHeight);
	}

	PageSizeChanged = TRUE;
}

void CDlgPageSizeBox::OnChange()
{
	int which = GetCheckedRadioButton(PAGESIZE_A4, PAGESIZE_PRINTER) - PAGESIZE_A4;

	if (which >= 0 && which < _countof(Page_SizesA))
	{
		int a = Page_SizesA[which];
		int b = Page_SizesB[which];

		if (GetCheckedRadioButton(PAGESIZE_LAND, PAGESIZE_PORT) == PAGESIZE_PORT)
		{
			SetDlgItemInt(PAGESIZE_HEIGHT, a);
			SetDlgItemInt(PAGESIZE_WIDTH, b);
		}
		else
		{
			SetDlgItemInt(PAGESIZE_WIDTH, a);
			SetDlgItemInt(PAGESIZE_HEIGHT, b);
		}

		PageSizeChanged = TRUE;
	}
}

void CDlgPageSizeBox::OnChangeText()
{
	int NewWidth = GetDlgItemInt(PAGESIZE_WIDTH);
	int NewHeight = GetDlgItemInt(PAGESIZE_HEIGHT);

	if (NewWidth < NewHeight)
	{
		CheckRadioButton(PAGESIZE_LAND, PAGESIZE_PORT, PAGESIZE_PORT);
	}
	else
	{
		CheckRadioButton(PAGESIZE_LAND, PAGESIZE_PORT, PAGESIZE_LAND);
	}

	// Do these values correspond to a predefined value?
	int a = max(NewWidth,NewHeight);
	int b = min(NewWidth,NewHeight);

	for (int lp = 0; lp != NumberOfSizes; lp++)
		if (a == Page_SizesA[lp] && b == Page_SizesB[lp]) CheckRadioButton(PAGESIZE_A4, PAGESIZE_PRINTER, PAGESIZE_A4 + lp);

	PageSizeChanged = TRUE;
}

void CDlgPageSizeBox::OnOK()
{
	Size.x = min(GetDlgItemInt(PAGESIZE_WIDTH),2500) * PIXELSPERMM;
	Size.y = min(GetDlgItemInt(PAGESIZE_HEIGHT),2500)* PIXELSPERMM;

	if (PageSetupChanged || PageSizeChanged)
	{
		CTinyCadRegistry::SetPageSize(Size);
	}

	if (Size.x < 10 || Size.y < 10)
	{
		EndDialog(IDCANCEL);
	}
	else
	{
		EndDialog(IDOK);
	}
}

