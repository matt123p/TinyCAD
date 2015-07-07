/*
 TinyCAD program for schematic capture
 Copyright 1994/1995/2002 Matt Pyne.

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

#ifndef __DLGUPDATEBOX_H__
#define __DLGUPDATEBOX_H__

#include "Symbol.h"

// The Update dialog box
class CDlgUpdateBox: public CDialog
{
	CLibraryStoreNameSet* m_NewSymbol;

	void AddSymbolField(CSymbolField &f);

	CEdit m_edit_control;
	CComboBox m_list_control;
	BOOL m_capture;
	int m_index;
	int m_column;

	recordCollection m_records;
	unsigned int m_current_record;

	CLibraryStoreSymbol& getCurrentRecord();

public:

	CDlgUpdateBox(CWnd* pParentWnd = NULL);
	~CDlgUpdateBox();

	// Dialog Data
	//{{AFX_DATA(CDlgUpdateBox)
	enum
	{
		IDD = IDD_UPDATE
	};
	CSpinButtonCtrl m_Pro_Spin;
	CEdit m_Pro_Priority;
	CSpinButtonCtrl m_Epi_Spin;
	CEdit m_Epi_Priority;
	CEdit m_Spice_Prolog;
	CEdit m_Spice_Epilog;
	CStatic m_Static4;
	CStatic m_Static3;
	CStatic m_Static2;
	CStatic m_Static1;
	CEdit m_Description;
	CButton m_Add;
	CEdit m_Spice_Model;
	CTabCtrl m_Tab;
	CButton m_Delete_Name;
	CListBox m_List_Names;
	CComboBox m_Orientation;
	CButton m_Delete;
	CListCtrl m_list;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgUpdateBox)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	void UpdateToRecord();
	void InitFromRecord();
	CSymbolField GetSymbolField(int index);
	CLibraryStoreNameSet *GetSymbol()
	{
		return m_NewSymbol;
	}

	void SetSymbol(CLibraryStoreNameSet *s)
	{
		m_NewSymbol = s;
	}

	// Implementation
protected:
	void EndEdit();
	//{{AFX_MSG(CDlgUpdateBox)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnClickUpdateList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetfocusUpdateList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnAddName();
	afx_msg void OnDeleteName();
	afx_msg void OnSelchangeListNames();
	afx_msg void OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit();
	afx_msg void OnKillfocusList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // __DLGUPDATEBOX_H__
