/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#pragma once

/** For future compatibility with .NET and Win64

 Microsoft Help for Microsoft Specific data types says:
 "The types __int8, __int16, and __int32 are synonyms for the ANSI types
 that have the same size, and are useful for writing portable code that behaves identically
 across multiple platforms. Note that the __int8 data type is synonymous with type char,
 __int16 is synonymous with type short, and __int32 is synonymous with type int.
 The __int64 type has no ANSI equivalent."
 */

typedef __int8 Int8;
typedef __int16 Int16;
typedef __int32 Int32;
typedef __int64 Int64;
typedef unsigned char UInt8;
typedef WORD UInt16;
typedef DWORD UInt32;
typedef UINT64 UInt64;
//typedef	BYTE	Bool8;
typedef BOOL Bool8;
typedef Int16 hRESOURCE;
typedef hRESOURCE hSTYLE;
typedef hRESOURCE hFILL;
typedef hRESOURCE hFONT;
typedef hRESOURCE hMETAFILE;
typedef hRESOURCE hSYMBOL;

#ifndef TODO
#define chSTR(x)  #x
#define chSTR2(x) chSTR(x)
#define TODO(desc) message( __FILE__"(" chSTR2(__LINE__) "): TODO => " #desc)
#endif
