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

// DlgExportPNG.cpp : implementation file
//

#include "stdafx.h"
#include "tinycad.h"
#include "DlgExportPNG.h"
#include "TinyCadRegistry.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgExportPNG dialog


CDlgExportPNG::CDlgExportPNG(CWnd* pParent /*=NULL*/) :
	CDialog(CDlgExportPNG::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgExportPNG)
	m_Filename = _T("");
	m_Scaling = 0;
	//}}AFX_DATA_INIT

	m_type = CTinyCadRegistry::GetInt("BitmapType", 0);
	m_Scaling = CTinyCadRegistry::GetInt("BitmapScaling", 100);
	m_Rotate = CTinyCadRegistry::GetInt("BitmapRotation", 0) != 0;
}

void CDlgExportPNG::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgExportPNG)
	DDX_Control(pDX, IDC_SCALING, m_ScalingCtrl);
	DDX_Control(pDX, IDC_FILE_TYPE, m_Filetype);
	DDX_Control(pDX, IDC_ROTATE, m_RotateCtrl);
	DDX_Text(pDX, IDC_FILE_NAME, m_Filename);
	DDX_Text(pDX, IDC_SCALING, m_Scaling);
	DDV_MinMaxInt(pDX, m_Scaling, 1, 1000);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgExportPNG, CDialog)
	//{{AFX_MSG_MAP(CDlgExportPNG)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_CBN_SELCHANGE(IDC_FILE_TYPE, OnSelchangeFileType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgExportPNG message handlers

BOOL CDlgExportPNG::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_Filetype.ResetContent();
	m_Filetype.AddString(_T("Colour PNG bitmap"));
	m_Filetype.AddString(_T("B&W PNG bitmap"));
	m_Filetype.AddString(_T("Colour Enhanced Metafile (EMF)"));
	m_Filetype.AddString(_T("B&W Enhanced Metafile (EMF)"));

	m_Filetype.SetCurSel(m_type);
	m_RotateCtrl.SetCheck(m_Rotate ? 1 : 0);

	UpdateData(FALSE);

	UpdateExtension();

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgExportPNG::OnBrowse()
{
	UpdateData(TRUE);

	CString ext = getExtension();
	CString t = ext == ".emf" ? "Metafile" : "Bitmap";

	// Generate the filter string from the extension
	TCHAR szFilter[256];
	_stprintf_s(szFilter, _T("%s (*%s)|*%s|All files (*.*)|*.*||"), t, ext, ext);

	CFileDialog dlg(FALSE, "*" + ext, m_Filename, OFN_HIDEREADONLY, szFilter, AfxGetMainWnd());

	if (dlg.DoModal() == IDOK)
	{
		m_Filename = dlg.GetPathName();
		UpdateData(FALSE);
	}
}

void CDlgExportPNG::OnOK()
{
	// Get the data before exiting...
	UpdateData(TRUE);

	m_type = m_Filetype.GetCurSel();
	m_Rotate = m_RotateCtrl.GetCheck() != 0;

	CTinyCadRegistry::Set("BitmapType", m_type);
	CTinyCadRegistry::Set("BitmapScaling", m_Scaling);
	CTinyCadRegistry::Set("BitmapRotation", m_Rotate);

	CDialog::OnOK();
}

void CDlgExportPNG::OnSelchangeFileType()
{
	UpdateExtension();
}

void CDlgExportPNG::UpdateExtension()
{
	UpdateData(TRUE);
	CString ext = getExtension();

	int b = m_Filename.ReverseFind('.');
	if (b != -1)
	{
		m_Filename = m_Filename.Left(b) + ext;
	}

	m_ScalingCtrl.EnableWindow(ext != ".emf");
	m_RotateCtrl.EnableWindow(ext != ".emf");

	UpdateData(FALSE);

}

CString CDlgExportPNG::getExtension()
{
	bool emf = m_Filetype.GetCurSel() >= 2;
	return emf ? ".emf" : ".png";
}
