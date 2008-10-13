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

#if !defined(AFX_SPECIALPRINTDLG_H__A52AD60C_0C1F_48DD_9C78_261C52FB61E3__INCLUDED_)
#define AFX_SPECIALPRINTDLG_H__A52AD60C_0C1F_48DD_9C78_261C52FB61E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpecialPrintDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpecialPrintDlg dialog

class CSpecialPrintDlg : public CPrintDialog
{
	DECLARE_DYNAMIC(CSpecialPrintDlg)

public:
	CSpecialPrintDlg(BOOL bPrintSetupOnly,
		DWORD dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS
			| PD_HIDEPRINTTOFILE | PD_NOSELECTION,
		CWnd* pParentWnd = NULL);

	BOOL	m_BlackAndWhite;
	double	m_Scale;
	double	m_FitScale;
	int		m_Copies;

private:
	CButton *m_BandWCtrl;

public:
// Dialog Data
	//{{AFX_DATA(CSpecialPrintDlg)
	enum { IDD = IDD_PRINT };
	CButton	m_Fit;
	int		m_print_all_sheets;
	//}}AFX_DATA

protected:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpecialPrintDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


	//{{AFX_MSG(CSpecialPrintDlg)
	afx_msg void OnBandw();
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
	afx_msg void OnFit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPECIALPRINTDLG_H__A52AD60C_0C1F_48DD_9C78_261C52FB61E3__INCLUDED_)
