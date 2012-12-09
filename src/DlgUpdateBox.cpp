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
#include "resource.h"
#include "DlgUpdateBox.h"
#include <ctype.h>
#include "TinyCadView.h"
#include "registry.h"
#include "colour.h"
#include "TinyCad.h"

#include "Net.h"

////// The Update Dialog Box //////

CDlgUpdateBox::CDlgUpdateBox(CWnd* pParentWnd) :
	CDialog(IDD_UPDATE, pParentWnd)
{
	//{{AFX_DATA_INIT(CDlgUpdateBox)
	//}}AFX_DATA_INIT
	m_capture = FALSE;
	m_current_record = 0;
}

void CDlgUpdateBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgUpdateBox)
	DDX_Control(pDX, IDC_PRO_SPIN, m_Pro_Spin);
	DDX_Control(pDX, IDC_PRO_PRIORITY, m_Pro_Priority);
	DDX_Control(pDX, IDC_EPI_SPIN, m_Epi_Spin);
	DDX_Control(pDX, IDC_EPI_PRIORITY, m_Epi_Priority);
	DDX_Control(pDX, IDC_SPICE_PROLOG, m_Spice_Prolog);
	DDX_Control(pDX, IDC_SPICE_EPILOG, m_Spice_Epilog);
	DDX_Control(pDX, IDC_STATIC4, m_Static4);
	DDX_Control(pDX, IDC_STATIC3, m_Static3);
	DDX_Control(pDX, IDC_STATIC2, m_Static2);
	DDX_Control(pDX, IDC_STATIC1, m_Static1);
	DDX_Control(pDX, UPDATE_DESCRIPTION, m_Description);
	DDX_Control(pDX, IDC_ADD, m_Add);
	DDX_Control(pDX, IDC_SPICE_MODEL, m_Spice_Model);
	DDX_Control(pDX, IDC_TAB1, m_Tab);
	DDX_Control(pDX, IDC_DELETE_NAME, m_Delete_Name);
	DDX_Control(pDX, IDC_LIST_NAMES, m_List_Names);
	DDX_Control(pDX, IDC_ORIENTATION, m_Orientation);
	DDX_Control(pDX, IDC_DELETE, m_Delete);
	DDX_Control(pDX, IDC_UPDATE_LIST, m_list);
	//}}AFX_DATA_MAP
}

CDlgUpdateBox::~CDlgUpdateBox()
{
}

BEGIN_MESSAGE_MAP( CDlgUpdateBox, CDialog )
	//{{AFX_MSG_MAP(CDlgUpdateBox)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_NOTIFY(NM_CLICK, IDC_UPDATE_LIST, OnClickUpdateList)
	ON_NOTIFY(NM_SETFOCUS, IDC_UPDATE_LIST, OnSetfocusUpdateList)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_ADD_NAME, OnAddName)
	ON_BN_CLICKED(IDC_DELETE_NAME, OnDeleteName)
	ON_LBN_SELCHANGE(IDC_LIST_NAMES, OnSelchangeListNames)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnSelchangeTab1)
	ON_EN_KILLFOCUS(101, OnKillfocusEdit)
	ON_CBN_KILLFOCUS(102, OnKillfocusList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CDlgUpdateBox::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText(UPDATE_LIBFILE, NameLength( (m_NewSymbol->GetLibName()), 30));
	SetDlgItemInt(UPDATE_PPP, m_NewSymbol->ppp);

	m_index = -1;
	m_Delete.EnableWindow(FALSE);

	m_Orientation.ResetContent();
	m_Orientation.AddString(_T("Up"));
	m_Orientation.AddString(_T("Down"));
	m_Orientation.AddString(_T("Left"));
	m_Orientation.AddString(_T("Right"));
	m_Orientation.SetCurSel(m_NewSymbol->orientation);

	m_current_record = 0;

	// Create some data
	int i;
	for (i = 0; i < m_NewSymbol->GetNumRecords(); i++)
	{
		m_records.push_back(m_NewSymbol->GetRecord(i));
	}

	// Create the treeview control
	m_list.InsertColumn(0, _T("Parameter"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(1, _T("Display Method"), LVCFMT_LEFT, 170);
	m_list.InsertColumn(2, _T("Default Value"), LVCFMT_LEFT, 150);

	// Update the names list box
	m_List_Names.ResetContent();

	for (unsigned int i = 0; i < m_records.size(); i++)
	{
		CSymbolRecord &r = m_records[i];
		m_List_Names.AddString(r.name);
	}
	m_List_Names.SetCurSel(m_current_record);

	// Update the delete name button
	m_Delete_Name.EnableWindow(m_records.size() > 1);

	// Create the two tabs
	m_Tab.InsertItem(0, _T("Attributes"));
	m_Tab.InsertItem(1, _T("SPICE"));

	InitFromRecord();

	return TRUE;
}

void CDlgUpdateBox::InitFromRecord()
{
	CSymbolRecord &r = getCurrentRecord();

	SetDlgItemText(UPDATE_DESCRIPTION, r.description);

	m_list.DeleteAllItems();

	// Now add the name & reference so that they can be
	// switched off
	CSymbolField f;
	f.field_name = _T("Reference");
	f.field_type = r.ref_type;
	f.field_default = r.reference;
	AddSymbolField(f);

	f.field_name = _T("Name");
	f.field_type = r.name_type;
	f.field_default = r.name;
	AddSymbolField(f);

	BOOL have_package = FALSE;
	int pro_priority = 5;
	int epi_priority = 5;

	for (unsigned int i = 0; i < r.fields.size(); i++)
	{
		CString field = r.fields[i].field_name;
		if (field.CompareNoCase(AttrSpice) == 0)
		{
			// This is the spice model
			m_Spice_Model.SetWindowText(r.fields[i].field_default);
		}
		else if (field.CompareNoCase(AttrSpiceProlog) == 0)
		{
			// This is the spice includes
			m_Spice_Prolog.SetWindowText(r.fields[i].field_default);
		}
		else if (field.CompareNoCase(AttrSpiceEpilog) == 0)
		{
			// This is the spice includes
			m_Spice_Epilog.SetWindowText(r.fields[i].field_default);
		}
		else if (field.CompareNoCase(AttrSpicePrologPri) == 0)
		{
			pro_priority = _tstoi(r.fields[i].field_default);
			if (pro_priority < 0 || pro_priority > 10)
			{
				pro_priority = 5;
			}
		}
		else if (field.CompareNoCase(AttrSpiceEpilogPri) == 0)
		{
			epi_priority = _tstoi(r.fields[i].field_default);
			if (epi_priority < 0 || epi_priority > 10)
			{
				epi_priority = 5;
			}
		}
		else
		{
			AddSymbolField(r.fields[i]);

			if (r.fields[i].field_name.CompareNoCase(_T("Package")) == 0)
			{
				have_package = TRUE;
			}
		}
	}

	// Write the priority fields
	CString s;
	s.Format(_T("%d"), pro_priority);
	m_Pro_Priority.SetWindowText(s);
	s.Format(_T("%d"), epi_priority);
	m_Epi_Priority.SetWindowText(s);

	// Now set-up the spin controls
	m_Pro_Spin.SetRange(0, 9);
	m_Epi_Spin.SetRange(0, 9);

	if (!have_package)
	{
		f.field_name = _T("Package");
		f.field_type = default_hidden;
		f.field_default = _T("package name for PCB layout");
		AddSymbolField(f);
	}

}

void CDlgUpdateBox::UpdateToRecord()
{
	CSymbolField f;
	CSymbolRecord &r = getCurrentRecord();

	// Get the reference
	f = GetSymbolField(0);
	r.reference = f.field_default;
	r.ref_type = f.field_type;

	// Get the name
	f = GetSymbolField(1);
	r.name = f.field_default;
	r.name_type = f.field_type;

	// Get the orientation
	m_NewSymbol->orientation = m_Orientation.GetCurSel();

	// Clear out all of the other fields
	r.fields.erase(r.fields.begin(), r.fields.end());

	// Write back the spice data
	CString spice_model;
	CString spice_prolog;
	CString spice_epilog;
	CString spice_pro_priority;
	CString spice_epi_priority;
	m_Spice_Model.GetWindowText(spice_model);
	m_Spice_Prolog.GetWindowText(spice_prolog);
	m_Spice_Epilog.GetWindowText(spice_epilog);
	m_Pro_Priority.GetWindowText(spice_pro_priority);
	m_Epi_Priority.GetWindowText(spice_epi_priority);
	if (!spice_model.IsEmpty())
	{
		f.field_name = AttrSpice;
		f.field_default = spice_model;
		f.field_type = always_hidden;
		r.fields.push_back(f);
	}
	if (!spice_prolog.IsEmpty())
	{
		f.field_name = AttrSpiceProlog;
		f.field_default = spice_prolog;
		f.field_type = always_hidden;
		r.fields.push_back(f);
	}
	if (!spice_epilog.IsEmpty())
	{
		f.field_name = AttrSpiceEpilog;
		f.field_default = spice_epilog;
		f.field_type = always_hidden;
		r.fields.push_back(f);
	}
	if (!spice_pro_priority.IsEmpty())
	{
		f.field_name = AttrSpicePrologPri;
		f.field_default = spice_pro_priority;
		f.field_type = always_hidden;
		r.fields.push_back(f);
	}
	if (!spice_epi_priority.IsEmpty())
	{
		f.field_name = AttrSpiceEpilogPri;
		f.field_default = spice_epi_priority;
		f.field_type = always_hidden;
		r.fields.push_back(f);
	}

	// Now write back the rest of the fields
	for (int i = 2; i < m_list.GetItemCount(); i++)
	{
		f = GetSymbolField(i);
		r.fields.push_back(f);
	}

	GetDlgItemText(UPDATE_DESCRIPTION, r.description);
}

void CDlgUpdateBox::OnOK()
{
	if (m_capture)
	{
		EndEdit();
		return;
	}

	UpdateToRecord();

	m_NewSymbol->ppp = (BYTE) GetDlgItemInt(UPDATE_PPP);
	m_NewSymbol->SetRecords(m_records);

	EndDialog(IDOK);

}

CLibraryStoreSymbol& CDlgUpdateBox::getCurrentRecord()
{
	if (m_current_record >= m_records.size())
	{ //is the current record out of range?  This can happen during certain editing operations.
		return m_records[m_records.size() - 1]; //return the last record
	}
	return m_records[m_current_record];
}

static const TCHAR *field_types[] = { //Note:  These values MUST be kept in sync with enum SymbolFieldType defined in Symbol.h
        _T("Show Value Only"), //SymbolFieldType = 0
                _T("Hide Value"), //SymbolFieldType = 1
                _T("Never Show"), //SymbolFieldType = 2
                _T("Show Value (Extra Only)"), //SymbolFieldType = 3 (extra_parameter)
                _T("Show Name=Value when present"), //SymbolFieldType = 4
                _T("Show Name=Value"), //SymbolFieldType = 5
                _T("Show Value when present") //SymbolFieldType = 6
        };

void CDlgUpdateBox::AddSymbolField(CSymbolField &f)
{
	int index = m_list.GetItemCount();
	LVITEM item;
	item.mask = LVIF_TEXT | LVIF_PARAM;
	item.iItem = index;
	item.iSubItem = 0;
	item.state = 0;
	item.stateMask = 0;
	item.iImage = 0;
	item.iIndent = 0;
	item.lParam = index;
	item.pszText = f.field_name.GetBuffer(256);
	m_list.InsertItem(&item);
	f.field_name.ReleaseBuffer();

	if (f.field_type >= 0 && f.field_type <= last_symbol_field_type)
	{
		item.mask = LVIF_TEXT;
		item.iSubItem = 1;
		item.pszText = (TCHAR *) field_types[f.field_type];
		m_list.SetItem(&item);
	}

	item.iSubItem = 2;
	item.pszText = f.field_default.GetBuffer(256);
	m_list.SetItem(&item);
	f.field_default.ReleaseBuffer();
}

void CDlgUpdateBox::OnAdd()
{
	// Add a new parameter to the end of the list
	// control
	CSymbolField f;
	f.field_default = "..";
	f.field_name = "Other";
	f.field_type = default_show;
	AddSymbolField(f);
}

void CDlgUpdateBox::OnDelete()
{
	if (m_index != 0)
	{
		m_list.DeleteItem(m_index);
		m_index = -1;
		m_Delete.EnableWindow(FALSE);
	}
}

void CDlgUpdateBox::OnClickUpdateList(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Find out where we clicked...
	const MSG *pMsg = GetCurrentMessage();
	CPoint clk = CPoint(pMsg->pt.x, pMsg->pt.y);

	// If we are already capturing then send this to the
	// capture item...
	if (m_capture)
	{
		CWnd *pwnd;
		if (m_column == 1)
		{
			pwnd = &m_list_control;
		}
		else
		{
			pwnd = &m_edit_control;
		}
		CRect q;
		pwnd->GetWindowRect(q);
		if (q.PtInRect(clk))
		{
			pwnd->PostMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(clk.x,clk.y));
			pwnd->PostMessage(WM_LBUTTONUP, 0, MAKELPARAM(clk.x,clk.y));
			return;
		}
		else
		{
			EndEdit();
		}
	}

	m_list.ScreenToClient(&clk);
	int index = m_list.HitTest(CPoint(5, clk.y));

	// Did we find this item?
	if (index == -1) return;

	// Release any old edit dlg
	if (m_capture)
	{
		EndEdit();
	}
	// Select this item
	LVITEM item;
	item.mask = LVIF_STATE;
	item.iItem = index;
	item.iSubItem = 0;
	item.state = LVIS_SELECTED | LVIS_FOCUSED;
	item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
	m_list.SetItem(&item);
	m_index = index;

	// If this index is over 2 then we can enable delete...
	m_Delete.EnableWindow(m_index >= 2);

	// Which column did the user click in?
	CRect rect;
	CRect r;
	m_list.GetItemRect(index, &rect, LVIR_BOUNDS);
	r = rect;
	int x = rect.right;
	r.left = x;
	int column;
	for (column = 2; column >= 0; column--)
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

	m_list.ClientToScreen(&r);
	ScreenToClient(&r);
	m_column = column;

	// Get this column's current value
	CString v;
	item.iItem = m_index;
	item.iSubItem = m_column;
	item.mask = LVIF_TEXT;
	item.pszText = v.GetBuffer(256);
	item.cchTextMax = 256;
	m_list.GetItem(&item);
	v.ReleaseBuffer();

	// Now perform the appropriate action
	switch (column)
	{
		case 0: //parameter name
		case 2: //parameter value
			// Now create the edit control
			if (m_index >= 2 || m_column > 0)
			{
				DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;

				m_capture = TRUE;
				m_edit_control.Create(style, r, this, 101);
				m_edit_control.SetLimitText(255);
				m_edit_control.SetFont(GetFont());
				m_edit_control.SetWindowText(v);
				m_edit_control.SetSel(0, v.GetLength());
				m_edit_control.SetFocus();
			}
			break;
		case 1: //parameter display control (whether to show parameter or not)
		{
			int sel = -1;
			m_capture = TRUE;
			r.bottom = r.bottom + 250;
			m_list_control.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | CBS_DROPDOWNLIST, r, this, 102);
			m_list_control.SetFont(GetFont());

			for (int i = 0; i < last_symbol_field_type; i++)
			{
				m_list_control.AddString(field_types[i]);
				if (v == field_types[i])
				{
					sel = i;
				}
			}
			m_list_control.SetCurSel(sel);
			m_list_control.SetFocus();
		}
			break;
	}

	*pResult = 0;
}

void CDlgUpdateBox::OnKillfocusEdit()
{
	EndEdit();
}

void CDlgUpdateBox::OnKillfocusList()
{
	// EndEdit();
}

void CDlgUpdateBox::OnSetfocusUpdateList(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (!m_capture)
	{
		OnClickUpdateList(pNMHDR, pResult);
	}

	*pResult = 0;
}

void CDlgUpdateBox::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);

	CPen *old_pen;
	CBrush *old_brush;
	CPen select_pen;
	CBrush select_brush;
	COLORREF old_colour;

	if ( (lpDrawItemStruct->itemState & ODS_SELECTED) != 0)
	{
		select_pen.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_HIGHLIGHT));
		select_brush.CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));

		old_pen = dc.SelectObject(&select_pen);
		old_brush = dc.SelectObject(&select_brush);

		old_colour = dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
	}
	else
	{
		old_pen = (CPen *) dc.SelectStockObject(WHITE_PEN);
		old_brush = (CBrush *) dc.SelectStockObject(WHITE_BRUSH);
		old_colour = dc.SetTextColor(RGB(0,0,0));
	}

	dc.Rectangle(&lpDrawItemStruct->rcItem);

	TCHAR buffer[256];
	int left = 0;
	int right = lpDrawItemStruct->rcItem.right;
	for (int column = 2; column >= 0; column--)
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
		item.cchTextMax = sizeof (buffer);
		m_list.GetItem(&item);

		dc.DrawText(buffer, &r, DT_LEFT | DT_VCENTER | DT_NOPREFIX);

		right = left;
	}

	dc.SetTextColor(old_colour);
	dc.SelectObject(old_pen);
	dc.SelectObject(old_brush);
	dc.Detach();
}

void CDlgUpdateBox::EndEdit()
{
	// Now end the editing...
	if (m_capture)
	{
		CString v;
		m_capture = FALSE;

		if (m_column == 1)
		{
			m_list_control.GetWindowText(v);
			m_list_control.DestroyWindow();
		}
		else
		{

			m_edit_control.GetWindowText(v);
			m_edit_control.DestroyWindow();
		}
		// .. and re-display...
		LVITEM item;
		item.iItem = m_index;
		item.iSubItem = m_column;
		item.mask = LVIF_TEXT;
		item.pszText = v.GetBuffer(256);
		m_list.SetItem(&item);
		v.ReleaseBuffer();

		// .. if this was the name, then we update the list box...
		if (m_index == 1 && m_column == 2)
		{
			m_List_Names.DeleteString(m_current_record);
			m_List_Names.InsertString(m_current_record, v);
		}
	}

}

CSymbolField CDlgUpdateBox::GetSymbolField(int index)
{
	CSymbolField f;
	LVITEM item;
	item.mask = LVIF_TEXT;
	item.iItem = index;
	item.iSubItem = 0;
	item.cchTextMax = 256;
	item.pszText = f.field_name.GetBuffer(256);
	m_list.GetItem(&item);
	f.field_name.ReleaseBuffer();

	CString ft;
	item.iSubItem = 1;
	item.pszText = ft.GetBuffer(256);
	m_list.GetItem(&item);
	ft.ReleaseBuffer();

	// Perform conversion...
	for (int i = 0; i < last_symbol_field_type; i++)
	{
		if (ft == field_types[i])
		{
			f.field_type = static_cast<SymbolFieldType> (i);
		}
	}

	item.iSubItem = 2;
	item.pszText = f.field_default.GetBuffer(256);
	m_list.GetItem(&item);
	f.field_default.ReleaseBuffer();

	return f;
}

void CDlgUpdateBox::OnAddName()
{
	// Write back the old data
	UpdateToRecord();

	// Create a new name that is a copy of the current name...
	CLibraryStoreSymbol r = getCurrentRecord();

	r.NameID = (DWORD) -1;
	r.name = "New Symbol";
	m_records.push_back(r);

	// Now update the list box...
	m_List_Names.AddString(r.name);
	m_List_Names.SetCurSel(((int) m_records.size()) - 1);

	m_Delete_Name.EnableWindow(TRUE);
}

void CDlgUpdateBox::OnDeleteName()
{
	// Write back the old data
	UpdateToRecord();

	// Delete the current record
	m_records.erase(m_records.begin() + m_current_record);
	m_List_Names.DeleteString(m_current_record);

	m_Delete_Name.EnableWindow(m_records.size() > 1);

	// .. and reset the pointer
	if (m_current_record > ((unsigned int) m_records.size()) - 1)
	{
		m_current_record = ((unsigned int) m_records.size()) - 1;
	}

	InitFromRecord();

	m_List_Names.SetCurSel(m_current_record);
}

void CDlgUpdateBox::OnSelchangeListNames()
{
	// Write back the old data
	UpdateToRecord();

	// Change the current record to the new selection
	m_current_record = m_List_Names.GetCurSel();

	// Read in the new data
	InitFromRecord();
}

void CDlgUpdateBox::OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Remove edit box
	EndEdit();

	int sel = m_Tab.GetCurSel();

	m_Add.ShowWindow(sel == 0);
	m_Delete.ShowWindow(sel == 0);
	m_list.ShowWindow(sel == 0);
	m_Description.ShowWindow(sel == 0);
	m_Static1.ShowWindow(sel == 0);

	m_Spice_Prolog.ShowWindow(sel == 1);
	m_Spice_Epilog.ShowWindow(sel == 1);
	m_Spice_Model.ShowWindow(sel == 1);
	m_Static2.ShowWindow(sel == 1);
	m_Static3.ShowWindow(sel == 1);
	m_Static4.ShowWindow(sel == 1);
	m_Pro_Priority.ShowWindow(sel == 1);
	m_Epi_Priority.ShowWindow(sel == 1);
	m_Pro_Spin.ShowWindow(sel == 1);
	m_Epi_Spin.ShowWindow(sel == 1);

	*pResult = 0;
}
