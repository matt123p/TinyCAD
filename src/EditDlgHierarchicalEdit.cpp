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


// EditToolbar.cpp : implementation file
//

#include "stdafx.h"
#include "tinycad.h"
#include "EditToolbar.h"
#include "EditDlgHierarchicalEdit.h"
#include "colour.h"
#include ".\editdlghierarchicaledit.h"

extern CEditToolbar g_EditToolBar;



////// The Method Edit dialog //////

CEditDlgHierarchicalEdit::CEditDlgHierarchicalEdit()
{  
	opens=0; 
	//{{AFX_DATA_INIT(CEditDlgHierarchicalEdit)
	//}}AFX_DATA_INIT
}


// Create this window
void CEditDlgHierarchicalEdit::Create()
{
	  CDialog::Create( IDD, &g_EditToolBar );
}


void CEditDlgHierarchicalEdit::DoDataExchange(CDataExchange* pDX)
{
	CEditDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditDlgHierarchicalEdit)
	DDX_Control(pDX, IDC_DELETE, m_Delete);
	DDX_Control(pDX, IDC_LIST, m_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( CEditDlgHierarchicalEdit, CEditDlg )
	//{{AFX_MSG_MAP(CEditDlgHierarchicalEdit)
	ON_BN_CLICKED(METHODBOX_NAMEON, OnChangeText )
	ON_BN_CLICKED(METHODBOX_MIRROR, OnChangeRotation )
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnClickList)
	ON_NOTIFY(NM_SETFOCUS, IDC_LIST, OnSetfocusList)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(METHODBOX_UP, OnChangeRotation )
	ON_BN_CLICKED(METHODBOX_DOWN, OnChangeRotation )
	ON_BN_CLICKED(METHODBOX_LEFT, OnChangeRotation )
	ON_BN_CLICKED(METHODBOX_RIGHT, OnChangeRotation )
	ON_BN_CLICKED(METHODBOX_OTHERON, OnChangeText )
	ON_BN_CLICKED(METHODBOX_REFON, OnChangeText )
	ON_BN_CLICKED(METHODBOX_RESIZE, OnChangeText )
	ON_EN_CHANGE(METHODBOX_NAME, OnChangeText )
	ON_EN_CHANGE(METHODBOX_OTHER, OnChangeText )
	ON_EN_CHANGE(METHODBOX_REF, OnChangeText )
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	//}}AFX_MSG_MAP
	ON_EN_KILLFOCUS(101, OnKillfocusEdit)
	ON_BN_CLICKED(IDC_DESIGN, OnBnClickedDesign)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditDlgHierarchicalEdit message handlers


BOOL CEditDlgHierarchicalEdit::OnInitDialog() 
{
	CEditDlg ::OnInitDialog();
	
	m_list.DeleteAllItems();
	m_list.InsertColumn(0,_T("Parameter"), LVCFMT_LEFT, 90);
	m_list.InsertColumn(1,_T("Show?"), LVCFMT_LEFT, 50);
	m_list.InsertColumn(2,_T("Value"), LVCFMT_LEFT, 140);

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditDlgHierarchicalEdit::OnOK() 
{ 
	EndEdit(); 
	SetFocus(); 
}


void CEditDlgHierarchicalEdit::ReadFields()
{
  CDrawHierarchicalSymbol *pMethod = static_cast<CDrawHierarchicalSymbol*>(getObject());


  // Load in the field parameters
  m_list.DeleteAllItems();
  int index = 0;
  for (unsigned int i = 0; i < pMethod->m_fields.size(); i ++)
  {
	  // Don't display the reference
	  if (i == CDrawHierarchicalSymbol::Ref || pMethod->m_fields[i].m_type == always_hidden)
	  {
		  continue;
	  }

		LVITEM item;
		item.mask = LVIF_TEXT|LVIF_PARAM; 
		item.iItem = index; 
		item.iSubItem = 0; 
		item.state = 0; 
		item.stateMask = 0; 
		item.iImage = 0; 
		item.iIndent = 0;
		item.lParam = i;
		item.pszText = pMethod->m_fields[i].m_description.GetBuffer(256); 
		m_list.InsertItem( &item );
		pMethod->m_fields[i].m_description.ReleaseBuffer(); 


		item.mask = LVIF_TEXT;
		item.iSubItem = 1; 
		item.pszText = pMethod->m_fields[i].m_show ? _T("Yes") : _T("No"); 
		m_list.SetItem( &item );

		item.mask = LVIF_TEXT;
		item.iSubItem = 2; 
		item.pszText = pMethod->m_fields[i].m_value.GetBuffer(256); 
		m_list.SetItem( &item );		
		pMethod->m_fields[i].m_value.ReleaseBuffer(); 

		index ++;
  }

  m_Delete.EnableWindow( FALSE );

}



void CEditDlgHierarchicalEdit::Open(CTinyCadDoc *pDesign,CDrawingObject *pObject)
{

  stop=TRUE;

  Show( pDesign, pObject );

  CDrawHierarchicalSymbol *pMethod = static_cast<CDrawHierarchicalSymbol*>(getObject());


  CheckRadioButton(METHODBOX_UP,METHODBOX_RIGHT,((pMethod->rotate)&3)+METHODBOX_UP);
  CheckDlgButton(METHODBOX_MIRROR,((pMethod->rotate)&4)!=0 ? 1 : 0);
  CheckDlgButton(METHODBOX_REFON,pMethod->m_fields[CDrawHierarchicalSymbol::Ref].m_show);
  CheckDlgButton(METHODBOX_RESIZE,pMethod->can_scale);

  SetDlgItemText(METHODBOX_REF,pMethod->m_fields[CDrawHierarchicalSymbol::Ref].m_value);

  ReadFields();

  stop=FALSE;
}


// Update the method after a change in the dialog
void CEditDlgHierarchicalEdit::OnChangeRotation()
{
  CDrawHierarchicalSymbol *pMethod = static_cast<CDrawHierarchicalSymbol*>(getObject());

  if (stop)
	return;

  getObject()->Display();

  // Set the rotation
  pMethod->rotate= (BYTE) GetDir();

  // Update the method's attributes
  pMethod->NewRotation();

  // re-display it
  getObject()->Display();
}


// Update the method after a change in the dialog
void CEditDlgHierarchicalEdit::OnChangeText()
{

  CDrawHierarchicalSymbol *pMethod = static_cast<CDrawHierarchicalSymbol*>(getObject());

  if (stop)
	return;

  getObject()->Display();

  // Set the text strings
  GetDlgItemText(METHODBOX_REF, pMethod->m_fields[CDrawHierarchicalSymbol::Ref].m_value );

  // Get the bools
  pMethod->m_fields[CDrawHierarchicalSymbol::Ref].m_show=IsDlgButtonChecked(METHODBOX_REFON)!=0;
  pMethod->can_scale=IsDlgButtonChecked(METHODBOX_RESIZE)!=0;

  // re-display it
  getObject()->Display();
}




// Get the rotation and mirroring
int CEditDlgHierarchicalEdit::GetDir()
{
  //CDrawHierarchicalSymbol *pMethod = static_cast<CDrawHierarchicalSymbol*>(getObject());

  int dir;

  dir=GetCheckedRadioButton(METHODBOX_UP,METHODBOX_RIGHT);

  return dir-METHODBOX_UP+(IsDlgButtonChecked(METHODBOX_MIRROR)!=0 ? 4 : 0);
}





void CEditDlgHierarchicalEdit::OnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Find out where we clicked...
	const MSG *pMsg = GetCurrentMessage();
	CPoint clk = CPoint( pMsg->pt.x, pMsg->pt.y );
	m_list.ScreenToClient(&clk);

	int index = m_list.HitTest(CPoint(5,clk.y));

	// Did we find this item?
	if (index == -1)
	{
		m_Delete.EnableWindow( FALSE );
		return;
	}

	// Select this item
	LVITEM item;
	item.mask = LVIF_STATE; 
	item.iItem = index; 
	item.iSubItem = 0;
	item.state = LVIS_SELECTED | LVIS_FOCUSED; 
	item.stateMask = LVIS_SELECTED | LVIS_FOCUSED; 
	m_list.SetItem( &item );
	

	// Which column did the user click in?
	CRect rect;
	CRect r;
	m_list.GetItemRect( index, &rect, LVIR_BOUNDS );
	r = rect;
	int x = rect.right;
	r.left = x;
	int column;
	for (column=2;column>=0;column--)
	{
		int w = m_list.GetColumnWidth(column);
		x -= w;
		r.right = r.left;
		r.left = x;
		
		if (clk.x > x)
		{
			break;
		}
	}

	// look up index to enable/disable delete button
	m_index = index;
	CDrawHierarchicalSymbol::CField &f = GetField( index );
	m_Delete.EnableWindow( f.m_type == extra_parameter );

	if (m_capture)
	{
		EndEdit();
	}

	m_column = column;

	// Now perform the appropriate action
	switch (column)
	{
	case 1:
		HideShow( index );
		break;
	case 0:
		if (f.m_type == extra_parameter)
		{
			BeginEdit( index, r );
		}
		break;
	case 2:
		if (index != 0)
		{
			BeginEdit( index, r );
		}
		else
		{
			// The user wants to change the design
			// this symbol is linked to...
			OnBnClickedDesign();
		}
		break;
	}

	
	*pResult = 0;
}

CDrawHierarchicalSymbol::CField &CEditDlgHierarchicalEdit::GetField( int index )
{
	// Switch the position of this item
	CDrawHierarchicalSymbol *pMethod = static_cast<CDrawHierarchicalSymbol*>(getObject());

	// look up index
	LVITEM item;
	item.iItem = index; 
	item.iSubItem = 1; 
	item.mask = LVIF_PARAM; 
	m_list.GetItem( &item );

	return pMethod->m_fields[ item.lParam ];
}

void CEditDlgHierarchicalEdit::HideShow(int index)
{
	// Switch the position of this item
	CDrawHierarchicalSymbol *pMethod = static_cast<CDrawHierarchicalSymbol*>(getObject());
	CDrawHierarchicalSymbol::CField &f = GetField( index );

	pMethod->Display();
	f.m_show = !f.m_show;

	// .. and re-display...
	LVITEM item;
	item.iItem = index; 
	item.iSubItem = 1; 
	item.mask = LVIF_TEXT;
	item.pszText = f.m_show ? _T("Yes") : _T("No"); 
	m_list.SetItem( &item );

	pMethod->Display();
}



void CEditDlgHierarchicalEdit::BeginEdit(int index, CRect r)
{
	// Are we already editing?
	if (m_capture)
	{
		EndEdit();
	}

	m_capture = TRUE;
	m_index = index;

	m_list.ClientToScreen( &r );
	ScreenToClient( &r );

	// Determine the value to edit
	CDrawHierarchicalSymbol::CField &f = GetField( index );
	CString v;
	if (m_column == 0)
	{
		v = f.m_description;
	}
	else
	{
		v = f.m_value;
	}

	// Now create the edit control
	m_edit_control.Create( WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL, r, this, 101 );
	m_edit_control.SetLimitText(255);
	m_edit_control.SetFont( GetFont() );
	m_edit_control.SetWindowText( v );
	m_edit_control.SetSel( 0, v.GetLength() );
	m_edit_control.SetFocus();
}

void CEditDlgHierarchicalEdit::EndEdit()
{
	// Now end the editing...
	if (m_capture)
	{
		CDrawHierarchicalSymbol *pMethod = static_cast<CDrawHierarchicalSymbol*>(getObject());

		m_capture = FALSE;
		pMethod->Display();
		CString v;
		CDrawHierarchicalSymbol::CField &f = GetField( m_index );
		m_edit_control.GetWindowText( v );
		m_edit_control.DestroyWindow();

		// Always show after editing...
		if (!f.m_show)
		{
			HideShow( m_index );
		}

		pMethod->Display();

		// .. and re-display...
		LVITEM item;
		item.iItem = m_index; 
		item.iSubItem = m_column; 
		item.mask = LVIF_TEXT;
		item.pszText = v.GetBuffer( 256 );
		m_list.SetItem( &item );
		v.ReleaseBuffer();

		// Which column to update?
		if (m_column == 0)
		{
			f.m_description = v;
		}
		else
		{
			f.m_value = v;
		}
	}
}

void CEditDlgHierarchicalEdit::OnKillfocusEdit() 
{
	EndEdit();
	
}

void CEditDlgHierarchicalEdit::OnSetfocusList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnClickList( pNMHDR, pResult );
	
	*pResult = 0;
}

// We don't like the default style of list boxes, so 
// draw our own!
void CEditDlgHierarchicalEdit::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDC dc;
	dc.Attach( lpDrawItemStruct->hDC );

	CPen *old_pen;
	CBrush *old_brush;
	CPen select_pen;
	CBrush select_brush;
	COLORREF old_colour;

	if ((lpDrawItemStruct->itemState & ODS_SELECTED) != 0)
	{
		select_pen.CreatePen( PS_SOLID, 1, GetSysColor(COLOR_HIGHLIGHT) );
		select_brush.CreateSolidBrush( GetSysColor(COLOR_HIGHLIGHT) );

		old_pen = dc.SelectObject( &select_pen );
		old_brush = dc.SelectObject( &select_brush );

		old_colour = dc.SetTextColor( GetSysColor(COLOR_HIGHLIGHTTEXT) );
	}
	else
	{
		old_pen = (CPen *)dc.SelectStockObject( WHITE_PEN );
		old_brush = (CBrush *)dc.SelectStockObject( WHITE_BRUSH );
		old_colour = dc.SetTextColor( RGB(0,0,0) );
	}

	dc.Rectangle( &lpDrawItemStruct->rcItem );

	TCHAR buffer[256];
	int left = 0;
	int right = lpDrawItemStruct->rcItem.right;
	for (int column=2;column>=0;column--)
	{
		// Create the rectangle to draw into
		left = right - m_list.GetColumnWidth(column);
		CRect r = lpDrawItemStruct->rcItem;
		r.left = left + 2;
		r.right = right;

		// Now draw this item's text
		LVITEM item;
		item.mask = LVIF_TEXT; 
		item.iItem = lpDrawItemStruct->itemID; 
		item.iSubItem = column; 
		item.state = 0; 
		item.stateMask = 0; 
		item.iImage = 0; 
		item.iIndent = 0;
		item.pszText = buffer;
		item.cchTextMax = sizeof( buffer );
		m_list.GetItem( &item );

		dc.DrawText( buffer, &r, DT_LEFT | DT_VCENTER | DT_NOPREFIX );
			
		right = left;
	}


	dc.SetTextColor( old_colour );
	dc.SelectObject( old_pen );
	dc.SelectObject( old_brush );
	dc.Detach();
}

// The space between the method and text
#define SPACING 5


void CEditDlgHierarchicalEdit::OnAdd() 
{
	// Add a new field to the method...
	CDrawHierarchicalSymbol *pMethod = static_cast<CDrawHierarchicalSymbol*>(getObject());

	// Work out where to place this symbol...
	CDPoint p = pMethod->m_fields[0].m_position;
	for (unsigned int i = 0; i < pMethod->m_fields.size(); i++)
	{
		if (pMethod->m_fields[i].m_position.y > p.y)
		{
			p = pMethod->m_fields[i].m_position;
		}
	}
	p.y += - ListOfFonts::PIN_HEIGHT + (SPACING/2);

	CDrawHierarchicalSymbol::CField f;
	f.m_description = "Other";
	f.m_position = p;
	f.m_show = TRUE;
	f.m_type = extra_parameter;
	f.m_value = "..";
	pMethod->m_fields.push_back(f);

	pMethod->Display();
	ReadFields();
}

void CEditDlgHierarchicalEdit::OnDelete() 
{
	// TODO: Add your control notification handler code here
	CDrawHierarchicalSymbol *pMethod = static_cast<CDrawHierarchicalSymbol*>(getObject());
	pMethod->Display();

	LVITEM item;
	item.iItem = m_index; 
	item.iSubItem = 1; 
	item.mask = LVIF_PARAM; 
	m_list.GetItem( &item );

	CDrawHierarchicalSymbol::fieldCollection::iterator it = pMethod->m_fields.begin();
	while (it != pMethod->m_fields.end() && item.lParam > 0)
	{
		-- item.lParam;
		++ it;
	}
	if (it != pMethod->m_fields.end())
	{
		pMethod->m_fields.erase( it );
	}

	pMethod->Display();
	ReadFields();
}


void CEditDlgHierarchicalEdit::OnBnClickedDesign()
{
	// All the user to change the design associated with
	// this symbol
	CDrawHierarchicalSymbol *pMethod = static_cast<CDrawHierarchicalSymbol*>(getObject());
	if (pMethod->SelectFile())
	{
		ReadFields();
		pMethod->Display();
	}
}
