/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */


// This file contains all the revision information for TinyCAD

///////////////////////////////////////////////////////////////////////////////
//
// Nov. 1994	Version 0.95
//		First release! Beta test version.
//
// Jan. 1995	Version 1.00
//		Full release version.
//		Added:		Toolbar
//					Drag 'n' Drop, "File Manager" Associations
//					Undo/Redo
//
//		Bug Fixes:	Edit Item, Import
//
//
// Apr. 1995	Version 1.20			( Release version)
//		Multi-layer version
//		Added:		New File revision number
//					Find/Tags
//					Font style, line style
//					Bus Name
//					CMainWindow destructor (version 1.20a)
//					Win32s support:
//						Arc segments now use Arc rather than PolyBezier
//						Use Tinycad.ini instead of Registry when in Win32s
//
//		Bug Fixes:	Printing, Names Dialog, IsInside Text
//
//
// May 1995		Version 1.21		 	(for Massimo only!)
//		Added:		Win16 Support
//					Line colour and style selection
//					Layers
//					Rulers
//		Bug Fixes:	crash on exit caused by "last got symbol" in library
//
// Sept 1995	Version 1.25			(release version)
//		Added		Text file support
//
// Nov  1995	Version 1.26
//		Added		Clipboard support
//					Drag select in EditItem
//
// Feb  2002	Version 1.50
//		Yes, this really is 7 years later!!!
//		Many, many changes in preparation for re-release.
//
//		Changed:	Renamed classes so that they all are in the Cxxx format
//					Drawing now not based on XOR
//					Stripped out use of global variable "thisDesign"
//						- added the m_pDesign variable into the CDrawingObject classes
//					Stripped out Win32s and Win16 support (not required these days...)
//					Separated Transform into Transform & Transform Snap
//
//	Aug 2002	Version 1.60.00
//		First stable release of TinyCAD.  This version works better than
//		version 1.50, and is really the first stable release of TinyCAD since
//		1.26.  
//
//		Changed:	So many changes can't document them all here!
//
//	Aug 2002	Version 1.60.01
//	
//		Changed:	Libraries are now a single file (the idx file)
//					The printing now automatically picks landscape / portrait
//					Symbols now saved with "fields" which will be used for additional
//					symbol information - e.g. for Spice.
//					Print in black and white makes a come-back
//		Added:		New non-printable drawing rulers for aligning objects
//					New reference painter 
//
//  Sep 2002	Version 1.60.02
//
//		Bug fixes:	Left & Right pins did not rotate properly.
//		Added:		Use the middle (scrollwheel) mouse button to pan
//
//	Oct 2002	Version 1.60.10
//
//		Bug fixes:	Print scaled printing
//					Fixed problem with pins not lining up with their text
//
//  Mar 2003	Version 1.60.11
//
//		Bug fixes:	Printing lines is too thin bug fix
//					Improved snaping when importing symbols so they always stay on grid
//					Tooltips now working again...
//
//

#define CLIPBOARD_FORMAT	_T("TinyCAD Design")	// The clipboard name for a tinycad copy
#define CLIPBOARD_SY_FORMAT	_T("TinyCAD Symbol")	// The clipboard name for a tinycad library copy

#define PIXELSPERMM			5					// How many pixels in each milimetre
#define NormalGrid			100					// The normal grid spacing (in pixels)
#define FineGrid			50					// The fine grid spacing (in pixels)

