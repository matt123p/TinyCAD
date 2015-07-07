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

#ifndef __EditDlgDrawLineEdit_H__
#define __EditDlgDrawLineEdit_H__

#include "diag.h"

// The dialog for editing the power object
class CEditDlgDrawLineEdit: public CEditDlg
{

	int m_angle_wire;

public:

	CEditDlgDrawLineEdit();

	// Dialog Data
	//{{AFX_DATA(CEditDlgDrawLineEdit)
	enum
	{
		IDD = IDD_DRAW_LINE
	};
	//}}AFX_DATA


	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditDlgDrawLineEdit)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	int mode;

	void Create();
	void Open(CTinyCadDoc *pDesign, CDrawingObject *pObject);
	int GetAngle()
	{
		int r = GetCheckedRadioButton(LINEBOX_FREE, LINEBOX_ARC2);
		m_angle_wire = r;

		return r;
	}

	// Stop Enter closing this dialog
	void OnOK()
	{
	}

	// Implementation
protected:
	//{{AFX_MSG(CEditDlgDrawLineEdit)
	// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // __EditDlgDrawLineEdit_H__
