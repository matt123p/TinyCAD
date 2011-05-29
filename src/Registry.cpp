/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include <winreg.h>
#include "registry.h"
#include "assert.h"

//=========================================================================
//== init static variables                                               ==
//=========================================================================
CRegKey CRegistry::m_oKey = CRegKey();
CRegistry CRegistry::m_oDummi;

//-------------------------------------------------------------------------
//-- Attach this object to the registry
CRegistry::CRegistry()
{
}
//-------------------------------------------------------------------------
//-- Close the registry database                                         --
//-------------------------------------------------------------------------
CRegistry::~CRegistry()
{
	m_oKey.Close();
}
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
//-- Associate this software with this key
void CRegistry::Associate(CString sExtension, CString sTitle, CString sProgram)
{
	// a separate key
	CRegKey oRegKey;

	// Associate the file type with an extension
	oRegKey.Create(HKEY_CLASSES_ROOT, sExtension);
	oRegKey.SetValue(HKEY_CLASSES_ROOT, sExtension, sTitle);

	// Write the description for this key
	oRegKey.SetValue(HKEY_CLASSES_ROOT, sTitle, sTitle);

	// Associate the file type with a program
	oRegKey.Create(HKEY_CLASSES_ROOT, sTitle + "\\shell\\open\\command");
	oRegKey.SetValue(HKEY_CLASSES_ROOT, sTitle + "\\shell\\open\\command", sProgram + " %1");

	// Associate the file with a program for printing
	oRegKey.Create(HKEY_CLASSES_ROOT, sTitle + "\\shell\\print\\command");
	oRegKey.SetValue(HKEY_CLASSES_ROOT, sTitle + "\\shell\\print\\command", sProgram + " /p %1");

	oRegKey.Close();
}
//-------------------------------------------------------------------------
int CRegistry::QueryValueEx(CString sKey, DWORD* Type, void *data, unsigned long *size)
{
	return RegQueryValueEx((HKEY) m_oKey, sKey, NULL, Type, (BYTE *) data, size);
}
//-------------------------------------------------------------------------
int CRegistry::SetValueEx(CString sKey, DWORD Type, const void *data, unsigned long length)
{
	return RegSetValueEx((HKEY) m_oKey, sKey, 0, Type, (const BYTE *) data, length);
}
//-------------------------------------------------------------------------
// Place a CString item
void CRegistry::Set(CString sKey, CString sVal)
{
	//TRACE("CRegistry::Set():  sKey=[%S], sVal.length()=%d, sVal=[%S]\n",
	//	sKey,
	//	sVal.GetLength()*sizeof(TCHAR),
	//	sVal);
	SetValueEx(sKey, REG_SZ, sVal, sVal.GetLength() * sizeof(TCHAR));
}
//-------------------------------------------------------------------------
// Place a DWORD item
void CRegistry::Set(CString sKey, DWORD nVal)
{
	SetValueEx(sKey, REG_DWORD, &nVal, sizeof (nVal));
}
//-------------------------------------------------------------------------
// Place a WORD item
void CRegistry::Set(CString sKey, WORD nVal)
{
	//djl - this next line appears to be a real bug!  There is no overload to support those arguments.
	//I think that this method is probably just never used and Visual Studio 2003 didn't flag it since it 
	//wasn't used, but I am going to implement it anyway just in case...
	//Set( nVal, sKey );
	SetValueEx(sKey, REG_DWORD, &nVal, sizeof (nVal));
}
//-------------------------------------------------------------------------
// Place a double item
void CRegistry::Set(CString sKey, double nVal)
{
	CString s;
	s.Format(_T("%lg"), nVal);
	Set(sKey, s);
}
//-------------------------------------------------------------------------
// Place int and BOOL items
void CRegistry::Set(CString sKey, int nVal)
{
	SetValueEx(sKey, REG_DWORD, &nVal, sizeof (nVal));
}
//-------------------------------------------------------------------------
// Place a bool value
void CRegistry::Set(CString sKey, bool bVal)
{
	DWORD nVal = bVal ? 1 : 0;

	Set(sKey, nVal);
}
//-------------------------------------------------------------------------
// Place a raw data block item
void CRegistry::Set(CString sKey, void* data, int length)
{
	SetValueEx(sKey, REG_BINARY, (unsigned char *) ((char *) data), length);
}
//-------------------------------------------------------------------------
// Place a CSize
void CRegistry::Set(CString sKey, CSize szVal)
{
	CString sPageSize = "";

	sPageSize.Format(_T("%d,%d"), szVal.cx, szVal.cy);

	Set(sKey, sPageSize);
}
//-------------------------------------------------------------------------
// Get a sub item from the registry
bool CRegistry::GetRaw(CString sKey, char *data, unsigned int length)
{
	DWORD type;
	DWORD size = length;
	int r = QueryValueEx(sKey, &type, data, &size);

	return r == ERROR_SUCCESS;
}
//-------------------------------------------------------------------------
// Get a String subkey
CString CRegistry::GetString(CString sKey, CString sDefaultVal)
{
	CString sReturn = sDefaultVal;
	DWORD nType = 0;
	DWORD nBytes = 0;

	if (RegQueryValueEx((HKEY) m_oKey, sKey, NULL, NULL, NULL, &nBytes) != ERROR_SUCCESS) return sDefaultVal; // failed - key not found

	//Note:  Returned byte length nBytes includes the terminating zero if the key represents a zero terminated string such as REG_SZ or REG_MULTI_SZ
	TCHAR* pTest = sReturn.GetBuffer(nBytes / sizeof(TCHAR) + 1);
	RegQueryValueEx((HKEY) m_oKey, sKey, NULL, &nType, (BYTE *) pTest, &nBytes);

	pTest[nBytes / sizeof(TCHAR)] = _T('\0'); // Windows Registry may return an unterminated string
	sReturn.ReleaseBuffer(); //Deallocates unused space.  Assumes that the string is zero terminated.  Does not affect space that is actually used.
	return sReturn;
}
//-------------------------------------------------------------------------
// Get a String array
// Splits a comma separated string into its substrings.
// nSize is the max buffer length that will be allocated when working with this string
CStringList* CRegistry::GetStringList(CString sKey)
{
	CStringList* colReturn = new CStringList();

	// Is there a list of items in the registry?
	CString sItems = CRegistry::GetString(sKey, "");

	// single item
	CString sItem = "";

	// Comma position
	int nFoundSep = -1;

	if (sItems != "")
	{
		while (sItems.GetLength() != 0)
		{
			nFoundSep = sItems.Find(',');

			if (nFoundSep == -1)
			{
				sItem = sItems;
				sItems = "";
			}
			else
			{
				sItem = sItems.Left(nFoundSep);
				sItems = sItems.Mid(nFoundSep + 1);
			}

			colReturn->AddTail(sItem);
		}
	}

	return colReturn;
}
//-------------------------------------------------------------------------
bool CRegistry::GetBool(CString sKey, bool bDefaultVal)
{
	return GetInt(sKey, bDefaultVal) != 0;
}
//-------------------------------------------------------------------------
int CRegistry::GetInt(CString sKey, int nDefaultVal)
{
	int nReturn = nDefaultVal;
	DWORD nVal = 0;

	DWORD dwType = NULL;
	DWORD dwCount = sizeof (nVal);
	LONG lRes = RegQueryValueEx(m_oKey.m_hKey, sKey, NULL, &dwType, (LPBYTE) &nVal, &dwCount);

	if (lRes == ERROR_SUCCESS)
	{
		return nVal;
	}

	return nReturn;
}
//-------------------------------------------------------------------------
BOOL CRegistry::GetBOOL(CString sKey, BOOL nDefaultVal)
{
	BOOL nReturn = nDefaultVal;
	DWORD nVal = 0;
	DWORD nType = 0;
	DWORD nSize = sizeof (nVal);

	if (QueryValueEx(sKey, &nType, &nVal, &nSize) == ERROR_SUCCESS)
	{
		nReturn = (BOOL) nVal;
	}

	return nReturn;
}
//-------------------------------------------------------------------------
char CRegistry::GetChar(CString sKey, char nDefaultVal)
{
	char nReturn = nDefaultVal;
	DWORD nVal = 0;
	DWORD nType = 0;
	DWORD nSize = sizeof(char);

	if (QueryValueEx(sKey, &nType, &nVal, &nSize) == ERROR_SUCCESS)
	{
		nReturn = static_cast<char> (nVal);
	}

	return nReturn;
}
//-------------------------------------------------------------------------
LONG CRegistry::GetLong(CString sKey, LONG nDefaultVal)
{
	LONG nReturn = nDefaultVal;
	DWORD nVal = 0;
	DWORD nType = 0;
	DWORD nSize = sizeof (nVal);

	if (QueryValueEx(sKey, &nType, &nVal, &nSize) == ERROR_SUCCESS)
	{
		nReturn = (LONG) nVal;
	}

	return nReturn;
}
//-------------------------------------------------------------------------
WORD CRegistry::GetWord(CString sKey, WORD nDefaultVal)
{
	WORD nReturn = nDefaultVal;
	DWORD nVal = 0;
	DWORD nType = 0;
	DWORD nSize = sizeof (nVal);

	if (QueryValueEx(sKey, &nType, &nVal, &nSize) == ERROR_SUCCESS)
	{
		nReturn = (WORD) nVal;
	}

	return nReturn;
}
//-------------------------------------------------------------------------
double CRegistry::GetDouble(CString sKey, double nDefaultVal)
{
	CString s;
	s.Format(_T("%lg"), nDefaultVal);
	return _tstof(GetString(sKey, s));
}
//-------------------------------------------------------------------------
CSize CRegistry::GetSize(CString sKey, CSize szDefault)
{
	CSize szReturn;
	CString sSize = GetString(sKey, "");
	int nFoundComma = -1;

	if (sSize != "")
	{
		nFoundComma = sSize.Find(',');

		if (nFoundComma != -1)
		{
			szReturn.cy = _tstoi(sSize.Mid(nFoundComma + 1));
			szReturn.cx = _tstoi(sSize.Left(nFoundComma));
		}
	}

	return szReturn;
}
//-------------------------------------------------------------------------
bool CRegistry::keyExists(CString sKey)
{
	DWORD nSize = 0;

	return QueryValueEx(sKey, NULL, NULL, &nSize) == ERROR_SUCCESS;
}
//-------------------------------------------------------------------------
void CRegistry::DeleteValue(CString sKey)
{
	m_oKey.DeleteValue(sKey);
}
//-------------------------------------------------------------------------
//-- These functions use the registry
void CRegistry::SetPrinterMode(HANDLE DevMode, HANDLE PrinterName)
{
	DEVMODE* theDevMode = (DEVMODE *) ::GlobalLock(DevMode);
	DEVNAMES* thePrinterName = (DEVNAMES *) ::GlobalLock(PrinterName);

	if (theDevMode != NULL)
	{
		Set("PrinterMode", (void *) theDevMode, theDevMode->dmSize + theDevMode->dmDriverExtra);
	}

	if (thePrinterName != NULL)
	{
		CString theNames;

		theNames = ((TCHAR *) thePrinterName) + thePrinterName->wDriverOffset;
		theNames += ",";
		theNames += ((TCHAR *) thePrinterName) + thePrinterName->wDeviceOffset;
		theNames += ",";
		theNames += ((TCHAR *) thePrinterName) + thePrinterName->wOutputOffset;

		Set("Printer", theNames);
	}

	::GlobalUnlock(DevMode);
	::GlobalUnlock(PrinterName);
}
//-------------------------------------------------------------------------


////// The find files class //////


// Find the first file

FindFile::FindFile(CString FileSpec)
{
	handle = ::FindFirstFile(FileSpec, &fileinfo);
}

// Find the next file
bool FindFile::FindNext()
{
	return (::FindNextFile(handle, &fileinfo) != 0);
}

// Close the FindFile structure
FindFile::~FindFile()
{
	::FindClose(handle);
}

TCHAR *FindFile::GetName()
{
	return fileinfo.cFileName;
}

// Was the find file successful?
bool FindFile::Success()
{
	return handle != INVALID_HANDLE_VALUE;
}
