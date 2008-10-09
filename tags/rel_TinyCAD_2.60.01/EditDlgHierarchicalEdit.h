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



#ifndef __EDITDLGHIERARCHICAL_H__
#define __EDITDLGHIERARCHICAL_H__

#include "Diag.h"

class CDrawingObject;
#include "DrawHierarchicalSymbol.h"

// The dialog for editing the method object
class CEditDlgHierarchicalEdit : public CEditDlg 
{

// Construction
public:
	CEditDlgHierarchicalEdit();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditDlgHierarchicalEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Dialog Data
	//{{AFX_DATA(CEditDlgHierarchicalEdit)
	enum { IDD = IDD_HIERARCHICAL };
	CButton	m_Delete;
	CListCtrl	m_list;
	//}}AFX_DATA


// Attributes
public:

// Operations
public:

// Overrides

protected:
	BOOL stop;
	CEdit m_edit_control;
	BOOL m_capture;
	int m_index;
	int m_column;

// Implementation
public:
	void ReadFields();
	void EndEdit();
	void BeginEdit( int index, CRect r );
	void HideShow( int index );
	CDrawHierarchicalSymbol::CField &GetField( int index );

	void Open(CTinyCadDoc *pDesign, CDrawingObject *pObject);
	void Create();

	int GetDir();

	// Stop Enter closing this dialog
	void OnOK();

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditDlgHierarchicalEdit)
	afx_msg void OnChangeText();
	afx_msg void OnChangeRotation();
	virtual BOOL OnInitDialog();
	afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetfocusList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	//}}AFX_MSG
	afx_msg void OnKillfocusEdit();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedDesign();
};


#endif // __EDITDLGHIERARCHICAL_H__
