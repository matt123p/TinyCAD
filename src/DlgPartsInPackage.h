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

#if !defined(AFX_DLGPARTSINPACKAGE_H__94FEBE96_C1B1_44B8_9F7E_398B72EE2398__INCLUDED_)
#define AFX_DLGPARTSINPACKAGE_H__94FEBE96_C1B1_44B8_9F7E_398B72EE2398__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgPartsInPackage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgPartsInPackage dialog

class CDlgPartsInPackage: public CDialog
{
	// Construction
public:
	CDlgPartsInPackage(CWnd* pParent = NULL); // standard constructor

	// Dialog Data
	//{{AFX_DATA(CDlgPartsInPackage)
	enum
	{
		IDD = IDD_SETPPP
	};
	UINT m_Parts;
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgPartsInPackage)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgPartsInPackage)
	// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGPARTSINPACKAGE_H__94FEBE96_C1B1_44B8_9F7E_398B72EE2398__INCLUDED_)
