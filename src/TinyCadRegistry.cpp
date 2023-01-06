/*
 * Project:		TinyCAD program for schematic capture
 *				https://www.tinycad.net
 * Copyright:	© 1994-2019 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "TinyCadRegistry.h"
#include "tinycad.h"
#include "BuildId.h"
#include <assert.h>

const CString CTinyCadRegistry::M_SKEY = "Software\\TinyCAD\\TinyCAD\\1x20";
const CString CTinyCadRegistry::M_BUILDID = "BuildID";
const CString CTinyCadRegistry::M_INSTALLED = "Installed";
const CString CTinyCadRegistry::M_SPAGESIZE = "PageSize";
const CString CTinyCadRegistry::M_SPRINTSCALE = "PrintScaleFactor";
const CString CTinyCadRegistry::M_SPRINTBANDW = "PrintBandW";
const CString CTinyCadRegistry::M_SMDIMAXIMIZE = "MdiMaximize";
const CString CTinyCadRegistry::M_SMAXIMIZE = "Maximize";
const CString CTinyCadRegistry::M_SLIBRARIES = "Libraries";
const CString CTinyCadRegistry::M_AUTOUDPATES = "AutomaticUpdates";
const CString CTinyCadRegistry::M_AUTOUDPATEVERSION = "AutomaticUpdateVersion";

//=========================================================================
//== ctor/dtor/initializing                                              ==
//=========================================================================

//-------------------------------------------------------------------------
//-- Attach this object to the registry
CTinyCadRegistry::CTinyCadRegistry() :
	CRegistry()
{
	m_oKey.Create(HKEY_CURRENT_USER, M_SKEY);

	// Get the installer date
	CString installedAt = GetInstalledFileTime();

	// Do we need to copy the example files?
	if (super::GetString(M_BUILDID, "") != BUILD_UUID || super::GetString(M_INSTALLED, "") != installedAt)
	{
		CopyExampleFiles();
		if (!installedAt.IsEmpty())
		{
			super::Set(M_INSTALLED, installedAt);
		}
	}

	// Does the registry information exist - if not create it
	if (!keyExists(M_SPAGESIZE))
	{
		CreateDefaultEntries();
	}
}
//-------------------------------------------------------------------------
CTinyCadRegistry::~CTinyCadRegistry()
{
}

//-------------------------------------------------------------------------
//-- Try and determine when TinyCAD was installed, so we can see if it has
//	 changed/been re-installed.
CString CTinyCadRegistry::GetInstalledFileTime()
{
	CString app_installed_file = CTinyCadApp::GetAppDir("") + _T("installed.txt");
	auto hFile = CreateFile(app_installed_file, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, 0, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		// File not found
		return "";
	}
	FILETIME ftCreate, ftAccess, ftWrite;
	SYSTEMTIME stUTC, stLocal;

	// Retrieve the file times for the file.
	if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
	{
		return "";
	}

	// Convert the last-write time to local time.
	FileTimeToSystemTime(&ftWrite, &stUTC);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

	// Build a string showing the date and time.
	CString r;
	r.Format(TEXT("%02d/%02d/%d  %02d:%02d"),
		stLocal.wMonth, stLocal.wDay, stLocal.wYear,
		stLocal.wHour, stLocal.wMinute);

	return r;
}

//-------------------------------------------------------------------------
//-- Copy the example files from the installation folder to the users
//   My Documents folder
void CTinyCadRegistry::CopyExampleFiles()
{
	// Make a copy of the examples and libraries
	const TCHAR* from_paths[] = { _T("library"), _T("examples") };
	for (int i = 0; i < 2; ++i)
	{
		CString app_dir = CTinyCadApp::GetAppDir(from_paths[i]);
		CString to_dir = CTinyCadApp::GetMyDocumentDir(from_paths[i]);

		// Create the folder
		SHCreateDirectoryEx(NULL,to_dir, NULL);

		// Now copy the files
		FindFile theFind(app_dir + _T("*.*"));
		if (theFind.Success())
		{
			do
			{
				if (theFind.IsFile())
				{
					CString from_file = app_dir + theFind.GetName();
					CString to_file = to_dir + theFind.GetName();
					CopyFile(from_file, to_file, TRUE);
				}
			} while (theFind.FindNext());
		}
	}

	// Set a flag in the registry
	super::Set(M_BUILDID, CString(BUILD_UUID));

}

//-------------------------------------------------------------------------
//-- Write the initial data to the registry
void CTinyCadRegistry::CreateDefaultEntries()
{
	TRACE("Virgin install detected:  CTinyCadRegistry::CreateDefaultEntries() is creating the very first registry entries for Tinycad\n");

	// Save the page size setup
	SetPageSize(CSize(297 * PIXELSPERMM, 210 * PIXELSPERMM));
	SetMaximize(false);

	// Search the default library directory for libraries
	for (int l = 0; l < 3; l++)
	{
		CString sSearch = CTinyCadApp::GetMyDocumentDir("library");

		if (l == 0)
		{
			sSearch += "*.idx";
		}
		else if (l == 1)
		{
			sSearch += "*.mdb";
		}
		else
		{
			sSearch += "*.TCLib";
		}

		FindFile theFind(sSearch);

		if (theFind.Success())
		{
			CString Libraries = "";

			do
			{
				if (theFind.IsFile())
				{
					// Add this library to the list of libraries in use
					if (Libraries != "") Libraries += ",";
					Libraries += CTinyCadApp::GetMyDocumentDir("library") /* + "library\\" */ + theFind.GetName();
					// Remove the extension (of .idx or .mdb or .TCLib)
					Libraries = Libraries.Left(Libraries.ReverseFind('.'));
				}
			} while (theFind.FindNext());

			super::Set("Libraries", Libraries);
		}
	}
}
//-------------------------------------------------------------------------

//=========================================================================
//== Accessor for application settings                                   ==
//=========================================================================

//-------------------------------------------------------------------------
//-- Returns the page size
CSize CTinyCadRegistry::GetPageSize()
{
	CSize szReturn = super::GetSize(M_SPAGESIZE, CSize());

	return szReturn;
}
//-------------------------------------------------------------------------
//-- Returns the print scale
double CTinyCadRegistry::GetPrintScale()
{
	double nReturn = super::GetDouble(M_SPRINTSCALE, 100);

	return nReturn;
}
//-------------------------------------------------------------------------
//-- Returns black and white print
bool CTinyCadRegistry::GetPrintBandW()
{
	bool bReturn = super::GetBool(M_SPRINTBANDW, false);

	return bReturn;
}
//-------------------------------------------------------------------------
//-- Returns MDI child window state
bool CTinyCadRegistry::GetMDIMaximize()
{
	bool bReturn = super::GetBool(M_SMDIMAXIMIZE, false);

	return bReturn;
}
//-------------------------------------------------------------------------
//-- Returns main window state
bool CTinyCadRegistry::GetMaximize()
{
	bool bReturn = super::GetBool(M_SMAXIMIZE, false);

	return bReturn;
}
//-------------------------------------------------------------------------
//-- Returns list of libraries
CStringList* CTinyCadRegistry::GetLibraryNames()
{
	CStringList* colReturn = CRegistry::GetStringList(M_SLIBRARIES);

	return colReturn;
}
//-------------------------------------------------------------------------
//-- Are automatic updates on (1), off (0) or unconfigured (-1)?
int CTinyCadRegistry::GetAutomaticUpdatesOn()
{
	return CRegistry::GetInt(M_AUTOUDPATES,-1);
}
//-------------------------------------------------------------------------
//-- What was the last version of TinyCAD update the user was told about?
CString CTinyCadRegistry::GetLastAutomaticUpdateVersion()
{
	return CRegistry::GetString(M_AUTOUDPATEVERSION, "");
}
//-------------------------------------------------------------------------


//=========================================================================
//== Mutator for application settings                                    ==
//=========================================================================

//-------------------------------------------------------------------------
//-- Changes the page size
void CTinyCadRegistry::SetPageSize(CSize szPage)
{
	super::Set(M_SPAGESIZE, szPage);
}
//-------------------------------------------------------------------------
//-- Changes the print scale
void CTinyCadRegistry::SetPrintScale(double nPrintScale)
{
	super::Set(M_SPRINTSCALE, nPrintScale);
}
//-------------------------------------------------------------------------
//-- Changes black and white print
void CTinyCadRegistry::SetPrintBandW(bool bPrintBandW)
{
	CTinyCadRegistry::Set(M_SPRINTBANDW, bPrintBandW);
}
//-------------------------------------------------------------------------
//-- Changes MDI child window state
void CTinyCadRegistry::SetMDIMaximize(bool bMDIMaximize)
{
	CTinyCadRegistry::Set(M_SMDIMAXIMIZE, bMDIMaximize);
}
//-------------------------------------------------------------------------
//-- Changes Changes main window state
void CTinyCadRegistry::SetMaximize(bool bMaximize)
{
	CTinyCadRegistry::Set(M_SMAXIMIZE, bMaximize);
}
//-------------------------------------------------------------------------
//-- Save auto-update preferences
void CTinyCadRegistry::SetAutomaticUpdatesOn(bool on)
{
	CTinyCadRegistry::Set(M_AUTOUDPATES, on ? 1 : 0);
}
//-------------------------------------------------------------------------
//-- What was the last version of TinyCAD update the user was told about
void CTinyCadRegistry::SetLastAutomaticUpdateVersion(CString version)
{
	CTinyCadRegistry::Set(M_AUTOUDPATEVERSION, version);
}
//-------------------------------------------------------------------------

