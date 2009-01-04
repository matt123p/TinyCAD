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

// XMLException.cpp: implementation of the CXMLException class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tinycad.h"
#include "XMLException.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//CXMLException::CXMLException(const TCHAR *name, BOOL b_AutoDelete )
//: CException( b_AutoDelete  )
//{
//	m_error = name;
//}

CXMLException::CXMLException(const TCHAR *name, int line_counter, BOOL b_AutoDelete )
: CException( b_AutoDelete  )
{

    m_error.Format(_T("%s.\nError first detected on line #%d.\n"), name, line_counter);
}

CXMLException::~CXMLException()
{
}

BOOL CXMLException::GetErrorMessage( LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext )
{
	#ifdef USE_VS2003
		_tcsncpy( lpszError, m_error, nMaxError );
	#else
		_tcsncpy_s( lpszError,nMaxError, m_error, nMaxError );
	#endif
	lpszError[ nMaxError -1 ] = 0;
	return TRUE;
}
