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

// DetailsPropertyPages.h : header file
//

#ifndef __DETAILSPROPERTYPAGES_H__
#define __DETAILSPROPERTYPAGES_H__

/////////////////////////////////////////////////////////////////////////////
// CDetailsPropertyPage1 dialog

class CDetailsPropertyPage1 : public CPropertyPage
{
	DECLARE_DYNCREATE(CDetailsPropertyPage1)

private:
	CMultiSheetDoc*	m_pDesign;

// Construction
public:
	CDetailsPropertyPage1(CMultiSheetDoc* pDesign = NULL);
	~CDetailsPropertyPage1();

// Dialog Data
	//{{AFX_DATA(CDetailsPropertyPage1)
	enum { IDD = IDD_DETAILS_PAGE1 };
	CString	m_sAuthor;
	CString	m_sDate;
	CString	m_sOrg;
	CString	m_sDoc;
	CString	m_sRevision;
	CString	m_sSheets;
	CString	m_sTitle;
	BOOL	m_bIsVisible;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CDetailsPropertyPage1)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDetailsPropertyPage1)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CDetailsPropertyPage2 dialog

class CDetailsPropertyPage2 : public CPropertyPage
{
	DECLARE_DYNCREATE(CDetailsPropertyPage2)

private:	
	CMultiSheetDoc*	m_pDesign;

// Construction
public:
	CDetailsPropertyPage2(CMultiSheetDoc* pDesign = NULL);
	~CDetailsPropertyPage2();

// Dialog Data
	//{{AFX_DATA(CDetailsPropertyPage2)
	enum { IDD = IDD_DETAILS_PAGE2 };
	CEdit	m_vert_enable;
	CEdit	m_horiz_enable;
	BOOL	m_bHasRulers;
	CString	m_horiz;
	CString	m_vert;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CDetailsPropertyPage2)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDetailsPropertyPage2)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowRulers();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};



#endif // __DETAILSPROPERTYPAGES_H__
