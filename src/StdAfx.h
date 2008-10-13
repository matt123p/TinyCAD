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

#if !defined(AFX_STDAFX_H__7554172E_EEA9_40FA_8A4A_C9D910CB873F__INCLUDED_)
#define AFX_STDAFX_H__7554172E_EEA9_40FA_8A4A_C9D910CB873F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Disable the annoying warning from MSVC
#pragma warning( disable:4786 )

// I just don't care about this warning...
#pragma warning( disable:4018 )
#pragma warning( disable:4995 )


#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <tchar.h>

#include <afxdao.h>			// MFC db support

#include <afxsock.h>		// MFC socket extensions

#include <OdbcInst.h>		// ODBC Installer support

#include <afxrich.h>		// RichEditView support

// The STL classes we use.
#include <vector>
#include <map>
#include <set>
#include <list>
#include <deque>

// Other useful stuff
#include <math.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__7554172E_EEA9_40FA_8A4A_C9D910CB873F__INCLUDED_)
