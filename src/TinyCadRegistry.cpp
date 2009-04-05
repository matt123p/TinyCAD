/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "TinyCadRegistry.h"
#include "tinycad.h"
#include <assert.h>

const CString CTinyCadRegistry::M_SKEY			= "Software\\TinyCAD\\TinyCAD\\1x20";
const CString CTinyCadRegistry::M_SPAGESIZE		= "PageSize";
const CString CTinyCadRegistry::M_SPRINTSCALE	= "PrintScaleFactor";
const CString CTinyCadRegistry::M_SPRINTBANDW	= "PrintBandW";
const CString CTinyCadRegistry::M_SMDIMAXIMIZE	= "MdiMaximize";
const CString CTinyCadRegistry::M_SMAXIMIZE		= "Maximize";
const CString CTinyCadRegistry::M_SLIBRARIES	= "Libraries";

//=========================================================================
//== ctor/dtor/initializing                                              ==
//=========================================================================

//-------------------------------------------------------------------------
//-- Attach this object to the registry
CTinyCadRegistry::CTinyCadRegistry()
: CRegistry()
{
	m_oKey.Create( HKEY_CURRENT_USER, M_SKEY );

	// Does the registry information exist - if not create it
	if( ! keyExists(M_SPAGESIZE) )
	{
		CreateDefaultEntries();
	}
}
//-------------------------------------------------------------------------
CTinyCadRegistry::~CTinyCadRegistry()
{
}
//-------------------------------------------------------------------------
//-- Write the initial data to the registry
void CTinyCadRegistry::CreateDefaultEntries()
{
	// Save the page size setup
	SetPageSize( CSize(297 * PIXELSPERMM, 210 * PIXELSPERMM) );
	SetMaximize( false );

		// Search the default library directory for libraries
		for (int l=0;l<3;l++)
		{
			CString sSearch = CTinyCadApp::GetMainDir() + "library\\";

			if( l == 0 )
			{
				sSearch += "*.idx";
			}
			else if( l == 1 )
			{
				sSearch += "*.mdb";
			}
			else
			{
				sSearch += "*.TCLib";
			}

			FindFile theFind( sSearch );

			if (theFind.Success())
			{
				CString Libraries = "";

				do {
					// Add this library to the list of libraries in use
					if (Libraries != "")
						Libraries += ",";
					Libraries += CTinyCadApp::GetMainDir() + "library\\" + theFind.GetName();
					// Remove the extension (of .idx)
					Libraries = Libraries.Left(Libraries.ReverseFind('.'));
				} while (theFind.FindNext());

				super::Set( "Libraries", Libraries );
			}
		}

		// Finially place the association of this file and the design type
		super::Associate( ".dsn","TinyCAD design", "TinyCAD" );
}
//-------------------------------------------------------------------------

//=========================================================================
//== Accessor for application settings                                   ==
//=========================================================================

//-------------------------------------------------------------------------
//-- Returns the page size
CSize CTinyCadRegistry::GetPageSize()
{
	CSize szReturn = super::GetSize( M_SPAGESIZE, CSize() );

	return szReturn;
}
//-------------------------------------------------------------------------
//-- Returns the print scale
double	CTinyCadRegistry::GetPrintScale()
{
	double nReturn = super::GetDouble( M_SPRINTSCALE, 100 );

	return nReturn;
}
//-------------------------------------------------------------------------
//-- Returns black and white print
bool CTinyCadRegistry::GetPrintBandW()
{
	bool bReturn = super::GetBool( M_SPRINTBANDW, false );

	return bReturn;
}
//-------------------------------------------------------------------------
//-- Returns MDI child window state
bool CTinyCadRegistry::GetMDIMaximize()
{
	bool bReturn = super::GetBool( M_SMDIMAXIMIZE, false );

	return bReturn;
}
//-------------------------------------------------------------------------
//-- Returns main window state
bool CTinyCadRegistry::GetMaximize()
{
	bool bReturn = super::GetBool( M_SMAXIMIZE, false );

	return bReturn;
}
//-------------------------------------------------------------------------
//-- Returns list of libraries
CStringList* CTinyCadRegistry::GetLibraryNames()
{
	CStringList* colReturn = CRegistry::GetStringList( M_SLIBRARIES );

	return colReturn;
}
//-------------------------------------------------------------------------

//=========================================================================
//== Mutator for application settings                                    ==
//=========================================================================

//-------------------------------------------------------------------------
//-- Changes the page size
void CTinyCadRegistry::SetPageSize( CSize szPage )
{
	super::Set( M_SPAGESIZE, szPage );
}
//-------------------------------------------------------------------------
//-- Changes the print scale
void CTinyCadRegistry::SetPrintScale( double nPrintScale )
{
	super::Set( M_SPRINTSCALE, nPrintScale );
}
//-------------------------------------------------------------------------
//-- Changes black and white print
void CTinyCadRegistry::SetPrintBandW( bool bPrintBandW )
{
	CTinyCadRegistry::Set( M_SPRINTBANDW, bPrintBandW );
}
//-------------------------------------------------------------------------
//-- Changes MDI child window state
void CTinyCadRegistry::SetMDIMaximize( bool bMDIMaximize )
{
	CTinyCadRegistry::Set( M_SMDIMAXIMIZE, bMDIMaximize );
}
//-------------------------------------------------------------------------
//-- Changes Changes main window state
void CTinyCadRegistry::SetMaximize( bool bMaximize )
{
	CTinyCadRegistry::Set( M_SMAXIMIZE, bMaximize );
}
//-------------------------------------------------------------------------
