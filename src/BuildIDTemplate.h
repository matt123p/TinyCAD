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
//         The generated output file should NOT be placed under version control, or it will cause you to 
//         have to commit this generated file each time that you want to make an official release, bumping
//         the actual revision number one higher than the compiled revision number.
//
#define SVN_WCREV _T("$WCREV$")		//Highest committed revision number
#define SVN_WCDATE _T("$WCDATE$")	//Date of highest committed revision
#define SVN_WCRANGE _T("$WCRANGE$")	//Update revision range - should be a single revision for a clean production build
#define SVN_WCURL _T("$WCURL$")		//Repository URL of the working copy
#define SVN_WCNOW _T("$WCNOW$")		//Current system date and time
//Note:  $WCMODS$ will be True if local modifications are present in the working copy
//Note:  $WCMIXED$ will be True if mixed update revisions are found in the working copy
//Placeholders of the form "$WCxxx?TrueText:FalseText$" are replaced with TrueText if the tested condition is true, and FalseText if false.
#define SVN_MODIFICATIONS $WCMODS?SVN_LOCAL_MODIFICATIONS_PRESENT:SVN_NO_LOCAL_MODIFICATIONS_PRESENT$
#define SVN_MODIFICATIONS_POSTFIX $WCMODS?_T("++"):_T("")$
#define SVN_MIXED $WCMIXED?SVN_MIXED_REVISIONS_PRESENT:SVN_NO_MIXED_REVISIONS$
//
//	Note:  The following is the help info from the SubWCRev command that is used to process this template:
//
//Usage: SubWCRev WorkingCopyPath [SrcVersionFile DstVersionFile] [-nmdf]
//
//Params:
//WorkingCopyPath    :   path to a Subversion working copy.
//SrcVersionFile     :   path to a template file containing keywords.
//DstVersionFile     :   path to save the resulting parsed file.
//-n                 :   if given, then SubWCRev will error if the working
//                       copy contains local modifications.
//-m                 :   if given, then SubWCRev will error if the working
//                       copy contains mixed revisions.
//-d                 :   if given, then SubWCRev will only do its job if
//                       DstVersionFile does not exist.
//
//-f                 :   if given, then SubWCRev will include the
//                       last-committed revision of folders. Default is
//                       to use only files to get the revision numbers.
//                       this only affects $WCREV$ and $WCDATE$.
//-e                 :   if given, also include dirs which are included
//                       with svn:externals, but only if they're from the
//                       same repository.
//
//-x                 :   if given, then SubWCRev will write the revisions
//                       numbers in HEX instead of decimal
//-X                 :   if given, then SubWCRev will write the revisions
//                       numbers in HEX with '0x' before them
//
//Switches must be given in a single argument, e.g. '-nm' not '-n -m'.
//
//SubWCRev reads the Subversion status of all files in a working copy
//excluding externals. If SrcVersionFile is specified, it is scanned
//for special placeholders of the form "$WCxxx$".
//SrcVersionFile is then copied to DstVersionFile but the placeholders
//are replaced with information about the working copy as follows:
//
//$WCREV$      Highest committed revision number
//$WCDATE$     Date of highest committed revision
//$WCDATE=$    Like $WCDATE$ with an added strftime format after the =
//$WCRANGE$    Update revision range
//$WCURL$      Repository URL of the working copy
//$WCNOW$      Current system date & time
//$WCNOW=$     Like $WCNOW$ with an added strftime format after the =
//
//Placeholders of the form "$WCxxx?TrueText:FalseText$" are replaced with
//TrueText if the tested condition is true, and FalseText if false.
//
//$WCMODS$     True if local modifications found
//$WCMIXED$    True if mixed update revisions found
//
//The strftime format strings for $WCDATE=$ & $WCNOW=$ must not be longer
//than 64 characters, and must not produce output greater than 128 characters.
