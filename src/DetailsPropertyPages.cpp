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

// DetailsPropertyPages.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "MultiSheetDoc.h"
#include "DetailsPropertyPages.h"
#include "TinyCadDoc.h"


IMPLEMENT_DYNCREATE(CDetailsPropertyPage1, CPropertyPage)
IMPLEMENT_DYNCREATE(CDetailsPropertyPage2, CPropertyPage)


/////////////////////////////////////////////////////////////////////////////
// CDetailsPropertyPage1 property page

CDetailsPropertyPage1::CDetailsPropertyPage1(CMultiSheetDoc* pDesign) 
: CPropertyPage(CDetailsPropertyPage1::IDD)
{
	//{{AFX_DATA_INIT(CDetailsPropertyPage1)
	m_sAuthor = _T("");
	m_sDate = _T("");
	m_sOrg = _T("");
	m_sDoc = _T("");
	m_sRevision = _T("");
	m_sSheets = _T("");
	m_sTitle = _T("");
	m_bIsVisible = FALSE;
	//}}AFX_DATA_INIT

	m_pDesign = pDesign;
}

CDetailsPropertyPage1::~CDetailsPropertyPage1()
{
}

void CDetailsPropertyPage1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDetailsPropertyPage1)
	DDX_Text(pDX, DESIGNBOX_AUTHOR, m_sAuthor);
	DDX_Text(pDX, DESIGNBOX_DATE, m_sDate);
	DDX_Text(pDX, DESIGNBOX_ORGANISATION, m_sOrg);
	DDX_Text(pDX, DESIGNBOX_DOCUMENT, m_sDoc);
	DDX_Text(pDX, DESIGNBOX_REVISION, m_sRevision);
	DDX_Text(pDX, DESIGNBOX_SHEET, m_sSheets);
	DDX_Text(pDX, DESIGNBOX_TITLE, m_sTitle);
	DDX_Check(pDX, DESIGNBOX_DISPLAY, m_bIsVisible);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDetailsPropertyPage1, CPropertyPage)
	//{{AFX_MSG_MAP(CDetailsPropertyPage1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDetailsPropertyPage2 property page

CDetailsPropertyPage2::CDetailsPropertyPage2(CMultiSheetDoc* pDesign) 
: CPropertyPage(CDetailsPropertyPage2::IDD)
{
	//{{AFX_DATA_INIT(CDetailsPropertyPage2)
	m_bHasRulers = FALSE;
	m_horiz = _T("");
	m_vert = _T("");
	//}}AFX_DATA_INIT

	m_pDesign = pDesign;
}

CDetailsPropertyPage2::~CDetailsPropertyPage2()
{
}

void CDetailsPropertyPage2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDetailsPropertyPage2)
	DDX_Control(pDX, IDC_VERT_GUIDE, m_vert_enable);
	DDX_Control(pDX, IDC_HORIZ_GUIDE, m_horiz_enable);
	DDX_Check(pDX, IDC_SHOW_RULERS, m_bHasRulers);
	DDX_Text(pDX, IDC_HORIZ_GUIDE, m_horiz);
	DDX_Text(pDX, IDC_VERT_GUIDE, m_vert);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDetailsPropertyPage2, CPropertyPage)
	//{{AFX_MSG_MAP(CDetailsPropertyPage2)
	ON_BN_CLICKED(IDC_SHOW_RULERS, OnShowRulers)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



BOOL CDetailsPropertyPage1::OnApply() 
{
	UpdateData( TRUE );

	m_pDesign->GetCurrentSheet()->GetDetails().SetVisible( m_bIsVisible == TRUE );
	m_pDesign->GetCurrentSheet()->GetDetails().SetTitle( m_sTitle );
	m_pDesign->GetCurrentSheet()->GetDetails().SetAuthor( m_sAuthor );
	m_pDesign->GetCurrentSheet()->GetDetails().SetRevision( m_sRevision );
	m_pDesign->GetCurrentSheet()->GetDetails().SetDocumentNumber( m_sDoc );
	m_pDesign->GetCurrentSheet()->GetDetails().SetOrganisation( m_sOrg );
	m_pDesign->GetCurrentSheet()->GetDetails().SetSheets( m_sSheets );
	m_pDesign->GetCurrentSheet()->GetDetails().SetLastChange( m_sDate );
	
	return CPropertyPage::OnApply();
}

BOOL CDetailsPropertyPage1::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
//	m_sFile			= m_pDesign->GetPathName();
	m_bIsVisible	= m_pDesign->GetCurrentSheet()->GetDetails().IsVisible();
	m_sDate			= m_pDesign->GetCurrentSheet()->GetDetails().GetLastChange();
	m_sTitle		= m_pDesign->GetCurrentSheet()->GetDetails().GetTitle();
	m_sAuthor		= m_pDesign->GetCurrentSheet()->GetDetails().GetAuthor();
	m_sRevision		= m_pDesign->GetCurrentSheet()->GetDetails().GetRevision();
	m_sDoc			= m_pDesign->GetCurrentSheet()->GetDetails().GetDocumentNumber();
	m_sOrg			= m_pDesign->GetCurrentSheet()->GetDetails().GetOrganisation();
	m_sSheets		= m_pDesign->GetCurrentSheet()->GetDetails().GetSheets();

	UpdateData( FALSE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDetailsPropertyPage2::OnApply() 
{
	UpdateData( TRUE );

	m_vert.MakeUpper();

	int h = _tstoi( m_horiz );
	int v = m_vert.IsEmpty() ? -1 : m_vert[0] - 'A' +1;

	m_pDesign->GetCurrentSheet()->GetDetails().SetRulers( m_bHasRulers == TRUE, v, h );
	
	return CPropertyPage::OnApply();
}

BOOL CDetailsPropertyPage2::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_bHasRulers	= m_pDesign->GetCurrentSheet()->GetDetails().HasRulers();
	m_horiz.Format(_T("%d"), m_pDesign->GetCurrentSheet()->GetDetails().m_iHorizRulerSize );
	m_vert.Format(_T("%c"), 'A' + m_pDesign->GetCurrentSheet()->GetDetails().m_iVertRulerSize - 1);
	m_horiz_enable.EnableWindow(m_bHasRulers);
	m_vert_enable.EnableWindow(m_bHasRulers);

	UpdateData( FALSE );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDetailsPropertyPage2::OnShowRulers() 
{
	UpdateData( TRUE );
	m_horiz_enable.EnableWindow(m_bHasRulers);
	m_vert_enable.EnableWindow(m_bHasRulers);
}
