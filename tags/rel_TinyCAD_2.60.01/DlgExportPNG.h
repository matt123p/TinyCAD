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

#if !defined(AFX_DLGEXPORTPNG_H__4BB971E3_8BCA_487B_B915_AFA6B9653D28__INCLUDED_)
#define AFX_DLGEXPORTPNG_H__4BB971E3_8BCA_487B_B915_AFA6B9653D28__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgExportPNG.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgExportPNG dialog

class CDlgExportPNG : public CDialog
{
// Construction
public:
	CDlgExportPNG(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgExportPNG)
	enum { IDD = IDD_EXPORT_PNG };
	CEdit	m_ScalingCtrl;
	CComboBox	m_Filetype;
	CString	m_Filename;
	int		m_Scaling;
	CButton m_RotateCtrl;
	//}}AFX_DATA

	CString getExtension();
	int m_type;
	bool m_Rotate;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgExportPNG)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgExportPNG)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowse();
	virtual void OnOK();
	afx_msg void OnSelchangeFileType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void UpdateExtension();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEXPORTPNG_H__4BB971E3_8BCA_487B_B915_AFA6B9653D28__INCLUDED_)
