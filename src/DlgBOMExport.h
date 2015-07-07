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

#if !defined(AFX_DlgBOMExport_H__83B4BA94_722D_4B75_B882_EF77441F5749__INCLUDED_)
#define AFX_DlgBOMExport_H__83B4BA94_722D_4B75_B882_EF77441F5749__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgBOMExport.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgBOMExport dialog

class CDlgBOMExport: public CDialog
{
	// Construction
public:
	CString GetExtension();
	CDlgBOMExport(CWnd* pParent = NULL); // standard constructor

	// Dialog Data
	//{{AFX_DATA(CDlgBOMExport)
	enum
	{
		IDD = IDD_EXPORT_BOM
	};
	CComboBox m_Filetype;
	CString m_Filename;
	BOOL m_Prefix;
	BOOL m_All_Attrs;
	BOOL m_All_Sheets;
	//}}AFX_DATA

	int m_type;

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgBOMExport)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgBOMExport)
	afx_msg void OnBrowse();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelchangeFileType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	BOOL m_Hierarchical;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DlgBOMExport_H__83B4BA94_722D_4B75_B882_EF77441F5749__INCLUDED_)
