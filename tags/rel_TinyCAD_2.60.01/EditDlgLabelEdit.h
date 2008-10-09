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



#ifndef __EditDlgLabelEdit_H__
#define __EditDlgLabelEdit_H__

#include "TinyCadDoc.h"

class CDrawingObject;


// The dialog for editing the text object
class CEditDlgLabelEdit : public CEditDlg {

	BOOL stop;

public:

	CEditDlgLabelEdit();
	void Open(CTinyCadDoc *pDesign, CDrawingObject *pObject);
	void Create();
	void ReFocus();

	// Stop Enter closing this dialog
	void OnOK() { }

	afx_msg void OnChange();
	
// Dialog Data
	//{{AFX_DATA(CEditDlgLabelEdit)
	enum { IDD = IDD_LABEL };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditDlgLabelEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CEditDlgLabelEdit)
	afx_msg void OnTextColour();
	afx_msg void OnTextFont();
	afx_msg void OnStyle1();
	afx_msg void OnStyle2();
	afx_msg void OnStyle3();
	afx_msg void OnStyle4();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};



#endif // __EditDlgLabelEdit_H__
