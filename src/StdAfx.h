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

// Visual Studio 2003:  _MSC_VER = 1300 through 1399
// Visual Studio 2005:  _MSC_VER = 1400 through 1499
// Visual Studio 2008:  _MSC_VER = 1500 through 1599
// Visual Studio ????:  _MSC_VER > 1600
#ifndef _MSC_VER
	#pragma message("Error:  TinyCAD is not supported or tested with non-Microsoft compilers")
#elif _MSC_VER < 1300
	#define USE_VS2003
	#pragma message("Warning:  Microsoft Visual C++ compilers older than Visual Studio .NET 2003 are not supported.")
#elif _MSC_VER < 1500
	//This source can be compiled with either VS2003 or VS2005 with this setting
	#define USE_VS2003
	#pragma message("Compiling TinyCAD with Visual Studio .NET 2003 or Visual Studio .NET 2005")
#elif _MSC_VER < 1600
	//This source can be compiled with VS2008 with this setting
	#define USE_VS2008
	#pragma message("Compiling TinyCAD with Visual Studio .NET 2008")
#else
	//This source has not been tested with a Visual Studio C++ compiler newer than 2008
	#define USE_VS2008
	#pragma message("Warning:  TinyCAD has not been evaluated or tested with Microsoft Visual Studio versions newer than Microsoft Visual Studio .NET 2008")
#endif

#ifdef USE_VS2003
	//Tell the Visual Studio 2003 compiler to compile for Windows XP and Windows .NET Server
	#define WINVER 0x0501
	#define _WIN32_WINNT 0x0501

	//The TinyCAD source now uses VS2008 "safe" MSVCRT runtime library functions that are created 
	//by appending an "_s" onto the end of the non-safe version (i.e., the VS2003 version) of the function.
	//The following macros along with specific conditional #ifdef's in some places permits TinyCAD
	//to be compiled under VS2003 or VS2008.  I have not tested TinyCAD under VS2005, but this source
	//is likely to be compatible with the VS2003 setting because VS2005 does not yet support the "safe"
	//versions of the MSVCRT libraries and the current TinyCAD source is already compatible with the
	//newer stricter for loop variable definition scope and other newer ISO C++ strict requirements.

	#define _stprintf_s _stprintf		/* just a simple rename to use the older function name - no argument manipulation required */
	#define _tcscpy_s _tcscpy			/* just a simple rename to use the older function name - no argument manipulation required for 2 argument instances */
	#define _tcscat_s _tcscat			/* just a simple rename to use the older function name - no argument manipulation required */
	#define _vsntprintf_s _vsntprintf	/* just a simple rename to use the older function name - no argument manipulation required */
	#define _itot_s _itot				/* just a simple rename to use the older function name - no argument manipulation required for 2 argument instances */
	#define _stscanf_s _stscanf			/* just a simple rename to use the older function name - no argument manipulation required */
	#define _tcsncpy_s _tcsncpy			/* just a simple rename to use the older function name - no argument manipulation required */
	typedef int errno_t;				/* VS2003 doesn't yet support the errno_t type so represent it as a simple int */
	#define _tfopen_s(fp, filename, mode) (((*fp = _tfopen(filename, mode)) == NULL) ? 1: 0)
#endif
#ifdef USE_VS2008
	//Tell the Visual Studio 2008 compiler to compile for Windows XP.  95, 98, no longer supported by VS2008
	#define WINVER 0x0501
	#define _WIN32_WINNT 0x0501
#endif

//I am not sure why Visual Studio 2008 deprecated the CDaoDatabase class, but it
//may have to do with the fact that there is no 64 bit processor support for it.
//This will disable the warnings, for now
#define _AFX_DISABLE_DEPRECATED 1

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
#include <assert.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__7554172E_EEA9_40FA_8A4A_C9D910CB873F__INCLUDED_)
