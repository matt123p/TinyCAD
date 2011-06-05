/*
 TinyCAD program for schematic capture
 Copyright 1994-2004 Matt Pyne.

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

// XMLException.h: interface for the CXMLException class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLEXCEPTION_H__495E52A1_E606_4024_ACF2_B8AA8B813697__INCLUDED_)
#define AFX_XMLEXCEPTION_H__495E52A1_E606_4024_ACF2_B8AA8B813697__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class CXMLException: public CException
{
protected:
	CString m_error;

public:
	//CXMLException( const TCHAR *name, BOOL b_AutoDelete = FALSE );
	CXMLException(const TCHAR *name, int line_counter, BOOL b_AutoDelete = FALSE);
	virtual ~CXMLException();

	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext = NULL);

};

#endif // !defined(AFX_XMLEXCEPTION_H__495E52A1_E606_4024_ACF2_B8AA8B813697__INCLUDED_)
