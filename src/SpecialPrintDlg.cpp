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
#include "tinycad.h"
#include "SpecialPrintDlg.h"
#include "TinyCadRegistry.h"
/////////////////////////////////////////////////////////////////////////////
// CSpecialPrintDlg
IMPLEMENT_DYNAMIC(CSpecialPrintDlg, CPrintDialog)

CSpecialPrintDlg::CSpecialPrintDlg(BOOL bPrintSetupOnly, DWORD dwFlags, CWnd* pParentWnd) :
	CPrintDialog(bPrintSetupOnly, dwFlags, pParentWnd)
{
	m_pd.Flags |= PD_ENABLEPRINTTEMPLATE;
	m_pd.hInstance = AfxGetInstanceHandle();
	m_pd.lpPrintTemplateName = MAKEINTRESOURCE( IDD_PRINT );
	//{{AFX_DATA_INIT(CSpecialPrintDlg)
	m_print_all_sheets = 0;
	//}}AFX_DATA_INIT

}

void CSpecialPrintDlg::DoDataExchange(CDataExchange* pDX)
{
	CPrintDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpecialPrintDlg)
	DDX_Radio(pDX, IDC_SHEET1, m_print_all_sheets);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSpecialPrintDlg, CPrintDialog)
	//{{AFX_MSG_MAP(CSpecialPrintDlg)
	ON_BN_CLICKED(PRINT_BANDW, OnBandw)
	ON_BN_CLICKED(1, OnOK)
	ON_BN_CLICKED(IDC_FIT, OnFit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSpecialPrintDlg::OnBandw()
{
	m_BlackAndWhite = m_BandWCtrl->GetCheck();
	CTinyCadRegistry::SetPrintBandW(m_BlackAndWhite != 0);
}

BOOL CSpecialPrintDlg::OnInitDialog()
{
	CPrintDialog::OnInitDialog();

	m_BandWCtrl = (CButton*) GetDlgItem(PRINT_BANDW);
	m_BlackAndWhite = CTinyCadRegistry::GetPrintBandW();
	m_Scale = CTinyCadRegistry::GetPrintScale();

	m_BandWCtrl->SetCheck(m_BlackAndWhite);

	CString s;
	s.Format(_T("%3.3lg"), m_Scale);
	SetDlgItemText(IDC_SCALE, s);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSpecialPrintDlg::OnOK()
{
	// TODO: Add your control notification handler code here
	CString s;
	GetDlgItemText(IDC_SCALE, s);
	m_Scale = _tstof(s);
	m_Copies = GetDlgItemInt(1154);
	CTinyCadRegistry::SetPrintScale(m_Scale);
	UpdateData(TRUE);
	EndDialog(IDOK);
}

void CSpecialPrintDlg::OnFit()
{
	CButton *b = static_cast<CButton*> (GetDlgItem(IDC_FIT));
	CString s;
	if (b->GetCheck() != 0)
	{
		s.Format(_T("%3.3lg"), m_FitScale);
		GetDlgItem(IDC_SCALE)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_SCALE)->EnableWindow(TRUE);
		s.Format(_T("%3.3lg"), m_Scale);
	}

	SetDlgItemText(IDC_SCALE, s);
}
