/*
	TinyCAD program for schematic capture
	Copyright 1994-2009 Matt Pyne.

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
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
//	Note:  This file is a template only - it will be parsed by the TortoiseSVN utility subwcrev.exe.
//         All macros will be substituted with values determined by scanning the entire repository.
//         A new output file will be generated each time that subwcrev is run, independently of whether
//         any changes have been made or not.
//
#define SVN_WCREV _T("$WCREV$")		//Highest committed revision number
#define SVN_WCDATE _T("$WCDATE$")	//Date of highest committed revision
#define SVN_WCRANGE _T("$WCRANGE$")	//Update revision range
#define SVN_WCURL _T("$WCURL$")		//Repository URL of the working copy
#define SVN_WCNOW _T("$WCNOW$")		//Current system date and time
//Note:  $WCMODS$ will be True if local modification are present in the working copy
//Note:  $WCMIXED$ will be True if mixed update revisions are found in the working copy
//Placeholders of the form "$WCxxx?TrueText:FalseText$" are replaced with TrueText if the tested condition is true, and FalseText if false.
#define SVN_MODIFICATIONS $WCMODS?SVN_LOCAL_MODIFICATIONS_PRESENT:SVN_NO_LOCAL_MODIFICATIONS_PRESENT$
#define SVN_MODIFICATIONS_POSTFIX $WCMODS?_T("++"):_T("")$
#define SVN_MIXED $WCMIXED?SVN_MIXED_REVISIONS_PRESENT:SVN_NO_MIXED_REVISIONS$
