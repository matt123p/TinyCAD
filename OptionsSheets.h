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

// OptionsSheets.h : header file
//

#ifndef __OPTIONSSHEETS_H__
#define __OPTIONSSHEETS_H__

/////////////////////////////////////////////////////////////////////////////
// COptionsGrid dialog

class CTinyCadDoc;

class COptionsGrid : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsGrid)

// Construction
public:
	COptionsGrid();
	~COptionsGrid();

// Dialog Data
	//{{AFX_DATA(COptionsGrid)
	enum { IDD = IDD_GRID };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

	CTinyCadDoc *GetDocument();
	void OnChange();

	double Grid;
	BOOL ShowGrid;
	BOOL SnapGrid;
	int	Units;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsGrid)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsGrid)
	virtual BOOL OnInitDialog();
	afx_msg void OnMm();
	afx_msg void OnInches();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// COptionsAutosnap dialog

class COptionsAutosnap : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsAutosnap)

// Construction
public:
	COptionsAutosnap();
	~COptionsAutosnap();

	CTinyCadDoc *GetDocument();

// Dialog Data
	//{{AFX_DATA(COptionsAutosnap)
	enum { IDD = IDD_AUTOSNAP };
	CEdit	m_Range_Ctrl;
	BOOL	m_Autodrag_Enable;
	BOOL	m_Autosnap_Enable;
	int		m_Autosnap_Range;
	CString	m_Units;
	BOOL	m_Autojunc_Enable;
	//}}AFX_DATA

	int		Units;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsAutosnap)
	public:
	virtual BOOL OnApply();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsAutosnap)
	virtual BOOL OnInitDialog();
	afx_msg void OnAutosnapEnable();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
//=========================================================================



//*************************************************************************
//*                                                                       *
//*                                                                       *
//*                                                                       *
//*************************************************************************
class COptionsAutoSave
: public CPropertyPage
{
	DECLARE_DYNCREATE( COptionsAutoSave )

	typedef CPropertyPage super;

	enum { IDD = IDD_AUTOSAVE };

	//-- Time in minutes between autosave events
	private:	int					m_nDelay;

	//-- Indicates if autobackup is enabled
	private:	BOOL				m_bIsEnabled;

	//-- spinbutton for altering m_nDelay in the range
	//-- between 1 and 60 minutes
	private:	CSpinButtonCtrl*	m_spnDelay;

	//-- Checkbox control for changing m_bIsAutoSave
	private:	CButton*			m_btnSave;

	//-- Label control for displaying m_nDelay value
	private:	CEdit*				m_wndBuddy;

	//=====================================================================
	//== constructor/destructor/initializing                             ==
	//=====================================================================
	public: 						COptionsAutoSave();
	protected:	virtual BOOL 		OnInitDialog();

	//=====================================================================
	//== Message handling                                                ==
	//=====================================================================
	private:	void 				OnChange();

	//-- Handels OK and APPLY button, but no Escape
	public:		virtual BOOL 		OnApply();

	DECLARE_MESSAGE_MAP()
};

#endif // __OPTIONSSHEETS_H__
