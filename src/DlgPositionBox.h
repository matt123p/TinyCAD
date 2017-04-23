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

#if !defined(AFX_DLGPOSITIONBOX_H__0E388BAB_A350_4CCE_B35A_7E523CD1E1BB__INCLUDED_)
#define AFX_DLGPOSITIONBOX_H__0E388BAB_A350_4CCE_B35A_7E523CD1E1BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgPositionBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgPositionBox dialog

// class prototype
class CToolbarLabel : public CMFCToolBarButton
{
	DECLARE_SERIAL(CToolbarLabel)
public:
	CToolbarLabel(UINT nID = NULL, int size = 0);
	CToolbarLabel(UINT nID, CString lpszText, int size = 0);
	virtual SIZE OnCalculateSize(CDC* pDC, const CSize& sizeDefault, BOOL bHorz) OVERRIDE;
	virtual void CToolbarLabel::CopyFrom(const CMFCToolBarButton& s) OVERRIDE;
	virtual void Serialize(CArchive& ar) OVERRIDE;

	virtual void OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages,
		BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
		BOOL bHighlight = FALSE,
		BOOL bDrawBorder = TRUE,
		BOOL bGrayDisabledButtons = TRUE) OVERRIDE;
protected:
	int m_size;
};



class CDlgPositionBox: public CMFCToolBar
{
	// Construction
public:
	CDlgPositionBox(); // standard constructor

	// Change the text in the Dialog
	void SetPosition(const CString s);
	void SetGridSize(double grid, int units);

	// Dialog Data
	//{{AFX_DATA(CDlgPositionBox)
	enum
	{
		IDD = IDR_POSITION
	};
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgPositionBox)
	//}}AFX_VIRTUAL


	// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgPositionBox)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGPOSITIONBOX_H__0E388BAB_A350_4CCE_B35A_7E523CD1E1BB__INCLUDED_)
