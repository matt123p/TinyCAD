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



#ifndef __EDITDLGLINEEDIT_H__
#define __EDITDLGLINEEDIT_H__

#include "diag.h"

// The dialog for editing the power object
class CEditDlgLineEdit : public CEditDlg {

	BOOL use_wire_angle;
	int angle_wire;
	int angle_polygon;
public:

	CEditDlgLineEdit();

// Dialog Data
	//{{AFX_DATA(CEditDlgLineEdit)
	enum { IDD = IDD_LINE };
	//}}AFX_DATA


// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditDlgLineEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	int mode;

	void Create();
	void Open(CTinyCadDoc *pDesign, CDrawingObject *pObject);
	int GetAngle() 
	{ 
		int r = GetCheckedRadioButton(LINEBOX_FREE,LINEBOX_ARC2);
		if (use_wire_angle)
		{
			angle_wire = r;
		}
		else
		{
			angle_polygon = r;
		}

		return r;
	}

	void SetArcAngle()
	{
		if (   angle_polygon != LINEBOX_ARC1 
			&& angle_polygon != LINEBOX_ARC2)
		{
			angle_polygon = LINEBOX_ARC2;
		}
	}

	void SetLineAngle()
	{
		if (   angle_polygon == LINEBOX_ARC1 
			|| angle_polygon == LINEBOX_ARC2)
		{
			angle_polygon = LINEBOX_FREE;
		}
	}


	CArcPoint::arc_type GetArcType()
	{
		CArcPoint::arc_type at;
		switch (GetAngle())
		{
			case LINEBOX_ARC1:
				at = CArcPoint::Arc_in;
				break;
			case LINEBOX_ARC2:
				at = CArcPoint::Arc_out;
				break;
			default:
				at = CArcPoint::Arc_none;
				break;
		}

		return at;
	}

	void SetArcType( CArcPoint::arc_type t )
	{
		switch (t)
		{
		case CArcPoint::Arc_in:
			CheckRadioButton(LINEBOX_FREE,LINEBOX_ARC2,LINEBOX_ARC1);
			break;
		case CArcPoint::Arc_out:
			CheckRadioButton(LINEBOX_FREE,LINEBOX_ARC2,LINEBOX_ARC2);
			break;
		default:
			CheckRadioButton(LINEBOX_FREE,LINEBOX_ARC2,LINEBOX_FREE);
			break;
		}
	}

	// Stop Enter closing this dialog
	void OnOK() { }


// Implementation
protected:
	//{{AFX_MSG(CEditDlgLineEdit)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#endif // __EDITDLGLINEEDIT_H__
