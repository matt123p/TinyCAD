/*
 * Project:		TinyCAD program for schematic capture
 *				https://www.tinycad.net/
 * Copyright:	© 1994-2009 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "resource.h"
#include "DlgAbout.h"
#include "TinyCad.h"

//*************************************************************************
//*                                                                       *
//* Shows information of the program like name of the programmer e.g.     *
//*                                                                       *
//*************************************************************************

//=========================================================================
//== ctor/dtor/initializing                                              ==
//=========================================================================

//-------------------------------------------------------------------------
CDlgAbout::CDlgAbout() :
	super(IDD_ABOUTBOX)
{
}
//-------------------------------------------------------------------------
BOOL CDlgAbout::OnInitDialog()
{
	wchar_t buffer[256];
	super::OnInitDialog();

	CString sVersion;
	sVersion.Format(_T("%s %s %s"), (LPCTSTR)CTinyCadApp::GetName(), (LPCTSTR)CTinyCadApp::GetVersion(), (LPCTSTR)CTinyCadApp::GetReleaseType());
	GetDlgItem(IDC_VERSION)->SetWindowText(sVersion);

	CString sWindowsVersion;
	//In Windows 8.1 and above, only the compatibility mode settings will be reported unless no compatibility mode settings have been set.
	CTinyCadApp::GetWindowsVersionName(buffer, 256);
	sWindowsVersion.Format(_T("Reported Windows version (may be altered by compatibility mode settings):\n%s."), buffer);

	GetDlgItem(IDC_WINDOWS_VERSION)->SetWindowText(sWindowsVersion);

	return TRUE;
}
//-------------------------------------------------------------------------
