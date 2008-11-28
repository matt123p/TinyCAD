/*
	TinyCAD program for schematic capture
	Copyright 1994/1995/2002-2005 Matt Pyne.

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
// DlgReplaceBox.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>

#include "tinycad.h"
#include "DlgReplaceBox.h"
#include "LibraryCollection.h"
#include "Context.h"
#include "TinyCadDoc.h"
#include "Registry.h"


/////////////////////////////////////////////////////////////////////////////
// CDlgReplaceBox dialog


CDlgReplaceBox::CDlgReplaceBox(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgReplaceBox::IDD, pParent)
	, m_keep_old_fields(FALSE)
{
	//{{AFX_DATA_INIT(CDlgReplaceBox)
	m_search_string = _T("");
	m_all_symbols = -1;
	//}}AFX_DATA_INIT
	m_Symbol = NULL;
	m_all_symbols = CRegistry::GetInt( "ReplaceDefault", 0 );
	m_keep_old_fields = CRegistry::GetInt( "KeepOldFields", 0 );
}


void CDlgReplaceBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgReplaceBox)
	DDX_Control(pDX, IDC_SHOW_SYMBOL, m_Show_Symbol);
	DDX_Control(pDX, FINDGET_LIST, m_List);
	DDX_Text(pDX, IDC_SEARCH_STRING, m_search_string);
	DDX_Radio(pDX, IDC_RADIO1, m_all_symbols);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_CHECK1, m_keep_old_fields);
}


BEGIN_MESSAGE_MAP(CDlgReplaceBox, CDialog)
	//{{AFX_MSG_MAP(CDlgReplaceBox)
	ON_EN_CHANGE(IDC_SEARCH_STRING, OnChangeSearchString)
	ON_WM_DRAWITEM()
	ON_LBN_DBLCLK(FINDGET_LIST, OnDblclkList)
	ON_LBN_SELCHANGE(FINDGET_LIST, OnSelchangeList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgReplaceBox message handlers


BOOL CDlgReplaceBox::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	BuildSearchList();
	m_List.SetCurSel( 0 );
	OnSelchangeList();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



void CDlgReplaceBox::OnChangeSearchString() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	UpdateData( TRUE );
	BuildSearchList();
}


void CDlgReplaceBox::OnDblclkList() 
{
	EndDialog( IDOK );
}

void CDlgReplaceBox::OnSelchangeList() 
{
	CListBox*	theListBox 	= (CListBox*) GetDlgItem( FINDGET_LIST );
	int			WhichItem 	= theListBox->GetCurSel();

 	if (WhichItem != LB_ERR)
	{
		m_Symbol = static_cast<CLibraryStoreSymbol *>(theListBox->GetItemDataPtr( WhichItem ));
		GetDlgItem( IDC_SHOW_SYMBOL )->RedrawWindow();
	}		
}


void CDlgReplaceBox::BuildSearchList()
{
	// Build the list box
	CListBox* theListBox = (CListBox*) GetDlgItem( FINDGET_LIST );
	theListBox->ResetContent();

	// Ensure the string is in Lower case for searching
	m_search_string.MakeLower();

	CLibraryCollection::FillMatchingSymbols( theListBox, m_search_string, NULL );

	// Try and find the selected symbol in the list
	int i;
	for (i = 0; i < theListBox->GetCount(); i++)
	{
		if (theListBox->GetItemDataPtr( i ) == m_Symbol)
		{
			theListBox->SetCurSel( i );
			break;
		}
	}
	if (i == theListBox->GetCount())
	{
		m_Symbol = NULL;
		GetDlgItem( IDC_SHOW_SYMBOL )->RedrawWindow();
	}
}

void CDlgReplaceBox::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDC dc;
	dc.Attach( lpDrawItemStruct->hDC );


	dc.SelectStockObject( WHITE_BRUSH );
	dc.Rectangle( &lpDrawItemStruct->rcItem );

	if (m_Symbol != NULL)
	{
		Transform transform;
		CContext q( &dc, transform );

		// Now access the symbol and draw it next to the name
		CDPoint p;
		CTinyCadDoc doc;
		CDesignFileSymbol *pSymbol = m_Symbol->GetDesignSymbol( &doc );

		// Determine the rotation
		int orientation = m_Symbol->m_pParent->orientation;


		drawingCollection method;
		if (pSymbol)
		{
			pSymbol->GetMethod( 0, false, method );
			p = pSymbol->GetTr( 0, false );
		}
		else
		{
			p = CDPoint(-75,-15);
		}

		switch (orientation)
		{
		case 2:
			{
				CDPoint q = p;
				p.x = q.y;
				p.y = q.x;
			}
			break;
		case 3:
			{
				CDPoint q = p;
				p.x = q.y;
				p.y = q.x;
			}
		}
		int height = (lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top);
		int width = (lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left);

		// Determine the zoom factor
		double zoom = min( (width * 75) / fabs(p.x), (height*75)/fabs(p.y) );
		zoom = min( zoom, 100 );
		q.SetZoomFactor( zoom / 100.0 );

		height = static_cast<int>((height * 100) / zoom);
		width = static_cast<int>((width * 100) / zoom);

		// Draw the name
		q.SetTextAlign(TA_LEFT | TA_BOTTOM | TA_NOUPDATECP);
		q.SetBkMode( TRANSPARENT );

		// Now display the symbol
		CDPoint old;

		switch (orientation)
		{
		case 2:
			old = q.SetTRM( CDPoint((width-p.x)/2, (height - p.y)/2) , CDPoint(0,0), orientation );
			break;
		case 3:
			old = q.SetTRM( CDPoint(width / 2 + p.x/2, (height-p.y)/2) , CDPoint(0,0), orientation );
			break;
		default:
			old = q.SetTRM( CDPoint((width-p.x)/2, (height-p.y)/2) , CDPoint(0,0), orientation );
			break;
		}
		if (pSymbol)
		{
			drawingIterator paint_it = method.begin();
			while (paint_it != method.end())
			{
				(*paint_it)->Paint( q, draw_normal );
				++ paint_it;
			}

			delete pSymbol;
		}

		q.EndTRM( old );
	}


	dc.Detach();

	// CInitDialogBar::OnDrawItem(nIDCtl, lpDrawItemStruct);
}


void CDlgReplaceBox::OnOK() 
{
	UpdateData( TRUE );
	CRegistry::Set( "ReplaceDefault", m_all_symbols  );
	CRegistry::Set( "KeepOldFields", m_keep_old_fields );
	CDialog::OnOK();
}
