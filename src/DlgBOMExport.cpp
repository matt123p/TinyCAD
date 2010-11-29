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

// DlgBOMExport.cpp : implementation file

#include "stdafx.h"
#include "tinycad.h"
#include "DlgBOMExport.h"
#include "TinyCadRegistry.h"


/////////////////////////////////////////////////////////////////////////////
// CDlgBOMExport dialog


CDlgBOMExport::CDlgBOMExport(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgBOMExport::IDD, pParent)
	, m_Hierarchical(FALSE)
{
	//{{AFX_DATA_INIT(CDlgBOMExport)
	m_Filename = _T("");
	m_Prefix = FALSE;
	m_All_Attrs = FALSE;
	m_All_Sheets = FALSE;
	//}}AFX_DATA_INIT

	m_type = CTinyCadRegistry::GetInt( "BOMNetlist", 0 );
	m_Hierarchical = CTinyCadRegistry::GetBool( "BOMHierarchical", 0 );
	m_Prefix = CTinyCadRegistry::GetBool( "BOMPrefixNetlist", 0 );
	m_All_Sheets = CTinyCadRegistry::GetBool( "BOMAllSheets", 0 );
	m_All_Attrs = CTinyCadRegistry::GetBool( "BOMAllAttrs", 0 );
}


void CDlgBOMExport::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgBOMExport)
	DDX_Control(pDX, IDC_FILE_TYPE, m_Filetype);
	DDX_Text(pDX, IDC_FILE_NAME, m_Filename);
	DDX_Check(pDX, IDC_PREFIX, m_Prefix);
	DDX_Check(pDX, IDC_ALL_ATTRS, m_All_Attrs);
	DDX_Check(pDX, IDC_ALL_SHEETS, m_All_Sheets);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_ALL_HIERARCHICAL, m_Hierarchical);
}


BEGIN_MESSAGE_MAP(CDlgBOMExport, CDialog)
	//{{AFX_MSG_MAP(CDlgBOMExport)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_CBN_SELCHANGE(IDC_FILE_TYPE, OnSelchangeFileType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgBOMExport message handlers

BOOL CDlgBOMExport::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_Filetype.ResetContent();
	m_Filetype.AddString( _T("Text file") );
	m_Filetype.AddString( _T("CSV file") );
	m_Filetype.AddString( _T("Spreadsheet compatible CSV file") );

	m_Filetype.SetCurSel( m_type );

	UpdateData( FALSE );

	OnSelchangeFileType();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgBOMExport::OnBrowse() 
{
	CString ext = GetExtension();

	// Generate the filter string from the extension
	TCHAR szFilter[256];
	_stprintf_s(szFilter, _T("Parts List (*%s)|*%s|All files (*.*)|*.*||"),
		ext, ext );

	CFileDialog dlg( FALSE, _T("*")+ext, m_Filename, OFN_HIDEREADONLY,
		szFilter, AfxGetMainWnd() ); 

	if (dlg.DoModal() == IDOK)
	{
		m_Filename = dlg.GetPathName();
		UpdateData( FALSE );
	}
}


void CDlgBOMExport::OnOK() 
{
	// Get the data before exiting...
	UpdateData( TRUE );

	m_type = m_Filetype.GetCurSel();
	CTinyCadRegistry::Set( "BOMNetlist", m_type );
	CTinyCadRegistry::Set( "BOMPrefixNetlist", m_Prefix );
	CTinyCadRegistry::Set( "BOMAllSheets", m_All_Sheets );
	CTinyCadRegistry::Set( "BOMAllAttrs", m_All_Attrs  );
	CTinyCadRegistry::Set( "BOMHierarchical", m_Hierarchical  );

	CDialog::OnOK();
}

void CDlgBOMExport::OnSelchangeFileType() 
{
	CString ext = GetExtension();

	int b = m_Filename.ReverseFind( '.' );
	if (b != -1)
	{
		m_Filename = m_Filename.Left( b ) + ext;
	}

	UpdateData( FALSE );
}

CString CDlgBOMExport::GetExtension()
{
	UpdateData( TRUE );
	int sel = m_Filetype.GetCurSel();

	CString ext;
	switch (sel)
	{
	case 0:	// TinyCAD
		return ".txt";
	case 1: // CSV
		return ".csv";
	case 2: // Spreadsheet type CSV
		return ".csv";
	}

	return ".txt";
}
