/*
 * Project:		TinyCAD program for schematic capture
 *				https://www.tinycad.net
 * Copyright:	© 1994-2019 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#pragma once

#include "stdafx.h"
#include "resource.h"
#include "UserColor.h"
#include "XmlWriter.h"
#include "XmlReader.h"

//*************************************************************************
//*                                                                       *
//*                                                                       *
//*                                                                       *
//*************************************************************************

//=========================================================================
//== ctor/dtor/initializing                                              ==
//=========================================================================
class CDlgColours: public CDialog
{
	typedef CDialog super;

	// Dialog Data
	//{{AFX_DATA(CDlgColours)
	enum
	{
		IDD = IDD_COLOURS
	};
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

private:
	COLORREF m_oaCustomColors[16];
private:
	CUserColor& m_oColorRef;
private:
	CUserColor m_oColor;
private:
	std::map<int, int> m_id_map;

	//=====================================================================
	//== ctor/dtor/initializing                                          ==
	//=====================================================================
public:
	CDlgColours(CUserColor& oColor);

	//=====================================================================
	//== accessor/mutator                                                ==
	//=====================================================================
public:
	void EditColor(int nID);

	//=====================================================================
	//== Message handling                                                ==
	//=====================================================================
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgColours)
	afx_msg void OnColBus();
	afx_msg void OnColJunction();
	afx_msg void OnColNoconnect();
	afx_msg void OnColPin();
	afx_msg void OnColPower();
	afx_msg void OnColWire();
	afx_msg void OnColNoteTextFill();
	afx_msg void OnColNoteTextLine();
	afx_msg void OnColNoteTextText();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnReset();
	afx_msg void OnColBackground();
	//}}AFX_MSG

	virtual void OnOK();

DECLARE_MESSAGE_MAP()
};
//=========================================================================
