#include "stdafx.h"
#include "TinyCad.h"
#include "TinyCadDoc.h"
#include "DlgDetails.h"
#include "TinyCadMultiDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-------------------------------------------------------------------------
CDlgDetails::CDlgDetails( CWnd* wndParent, CMultiSheetDoc* pDesign )
: CDialog( IDD_DESIGN, wndParent )
{
	m_pDesign		= pDesign;
	m_bIsVisible	= FALSE;
	m_bHasRulers	= FALSE;
	m_sAuthor		= "";
	m_sDate			= "";
	m_sDoc			= "";
	m_sFile			= "";
	m_sOrg			= "";
	m_sRevision		= "";
	m_sSheets		= "";
	m_sTitle		= "";
}
//-------------------------------------------------------------------------
void CDlgDetails::DoDataExchange( CDataExchange* pDX )
{
	super::DoDataExchange( pDX );

	DDX_Text(	pDX, DESIGNBOX_AUTHOR,		m_sAuthor );
	DDX_Text(	pDX, DESIGNBOX_DATE,		m_sDate );
	DDX_Text(	pDX, DESIGNBOX_DOCUMENT,	m_sDoc );
	DDX_Text(	pDX, DESIGNBOX_FILE,		m_sFile );
	DDX_Text(	pDX, DESIGNBOX_ORGANISATION,m_sOrg );
	DDX_Text(	pDX, DESIGNBOX_REVISION,	m_sRevision );
	DDX_Text(	pDX, DESIGNBOX_SHEET,		m_sSheets );
	DDX_Text(	pDX, DESIGNBOX_TITLE,		m_sTitle );
	DDX_Check(	pDX, DESIGNBOX_DISPLAY,		m_bIsVisible );
	DDX_Check(	pDX, IDC_SHOW_RULERS,		m_bHasRulers );
}
//-------------------------------------------------------------------------
// The methods for the details dialog
BOOL CDlgDetails::OnInitDialog()
{
	super::OnInitDialog();

	m_sFile			= m_pDesign->GetPathName();
	m_bIsVisible	= m_pDesign->GetDetails().IsVisible();
	m_bHasRulers	= m_pDesign->GetDetails().HasRulers();
	m_sDate			= m_pDesign->GetDetails().GetLastChangeAsString();
	m_sTitle		= m_pDesign->GetDetails().GetTitle();
	m_sAuthor		= m_pDesign->GetDetails().GetAuthor();
	m_sRevision		= m_pDesign->GetDetails().GetRevision();
	m_sDoc			= m_pDesign->GetDetails().GetDocumentNumber();
	m_sOrg			= m_pDesign->GetDetails().GetOrganisation();
	m_sSheets		= m_pDesign->GetDetails().GetSheets();

	UpdateData( FALSE );

	return TRUE;
}
//-------------------------------------------------------------------------
void CDlgDetails::OnOK()
{
	UpdateData( TRUE );

	m_pDesign->GetDetails().SetVisible( m_bIsVisible == TRUE );
	m_pDesign->GetDetails().SetRulers( m_bHasRulers == TRUE );
	m_pDesign->GetDetails().SetTitle( m_sTitle );
	m_pDesign->GetDetails().SetAuthor( m_sAuthor );
	m_pDesign->GetDetails().SetRevision( m_sRevision );
	m_pDesign->GetDetails().SetDocumentNumber( m_sDoc );
	m_pDesign->GetDetails().SetOrganisation( m_sOrg );
	m_pDesign->GetDetails().SetSheets( m_sSheets );
	m_pDesign->GetDetails().SetLastChange( m_sDate );

	EndDialog(IDOK);
}
//-------------------------------------------------------------------------
