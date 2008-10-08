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

#pragma once

#include "Details.h"

class CMultiSheetDoc;

//=========================================================================
//== The dialog to set the design details
//=========================================================================
class CDlgDetails
: public CDialog
{
private:
	typedef	CDialog super;

	enum { IDD = IDD_DESIGN };

	//--
	private:	CMultiSheetDoc*	m_pDesign;
	private: 	CString			m_sVersion;
	private: 	CString			m_sAuthor;
	private: 	CString			m_sDate;
	private: 	BOOL			m_bIsVisible;
	private: 	CString			m_sDoc;
	private: 	CString			m_sFile;
	private: 	CString			m_sOrg;
	private: 	CString			m_sRevision;
	private: 	CString			m_sSheets;
	private: 	CString			m_sTitle;
	private: 	BOOL			m_bHasRulers;

	//---------------------------------------------------------------------
	public:						CDlgDetails( CWnd* wndParent, CMultiSheetDoc* pDesign );
	//---------------------------------------------------------------------
	protected:	virtual BOOL	OnInitDialog();
	//---------------------------------------------------------------------
	protected:	virtual void	DoDataExchange( CDataExchange* pDX );
	//---------------------------------------------------------------------
	protected:	virtual void	OnOK();
	//---------------------------------------------------------------------
};
//=========================================================================