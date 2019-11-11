/*
 * Project:		TinyCAD program for schematic capture
 *				https://www.tinycad.net
 * Copyright:	© 1994-2019 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#pragma once
#include <atlbase.h>

//*************************************************************************
//* interface to the registry database                                    *
//*                                                                       *
//* Each registry entry is placed into                                    *
//* HKEY_CURRENT_USER\Software\TinyCAD\TinyCAD                            *
//* (where name is specified on construction of registry)                 *
//* They are then sub divided into keys.                                  *
//* Each key is a single string in the database                           *
//*                                                                       *
//*************************************************************************
class CRegistry
{
	//=====================================================================
	//== constructor/destructor/initializing                             ==
	//=====================================================================
protected:
	CRegistry();
public:
	~CRegistry();

	//=====================================================================
	//== accessor/mutator                                                ==
	//=====================================================================
public:
	static void Set(CString sKey, DWORD nVal);
	static void Set(CString sKey, WORD nVal);
	static void Set(CString sKey, double nVal);
	static void Set(CString sKey, int nVal);
	static void Set(CString sKey, bool bVal);
protected:
	static void Set(CString sKey, CSize szVal);
public:
	static void Set(CString sKey, CString sVal);
private:
	static void Set(CString sKey, void* data, int length);

public:
	static CString GetString(CString sKey, CString sDefaultVal);
	static bool GetBool(CString sKey, bool bDefaultVal);
	static BOOL GetBOOL(CString sKey, BOOL nDefaultVal);
	static int GetInt(CString sKey, int nDefaultVal);
	static char GetChar(CString sKey, char nDefaultVal);
	static WORD GetWord(CString sKey, WORD nDefaultVal);
	static LONG GetLong(CString sKey, LONG nDefaultVal);
	static double GetDouble(CString sKey, double nDefaultVal);
protected:
	static CSize GetSize(CString sKey, CSize szDefault);
private:
	static bool GetRaw(CString sKey, char* data, unsigned int length);
protected:
	static CStringList* GetStringList(CString sKey);
public:
	static void DeleteValue(CString sKey);

	//=====================================================================
	//== These are some standard functions which use the registry        ==
	//=====================================================================
	static void SetPrinterMode(HANDLE theDevMode, HANDLE thePrinterName);

protected:

	//-- Check key
	static bool keyExists(CString sKey);

private:
	static int QueryValueEx(CString sKey, DWORD *Type, void* data, unsigned long *size);
	static int SetValueEx(CString sKey, DWORD Type, const void* data, unsigned long length);

	//=====================================================================
	//== class variables                                                 ==
	//=====================================================================
protected:
	static CRegKey m_oKey;

	// Forces a ctor call before calling any static method
	// Simulates a java static ctor block
private:
	static CRegistry m_oDummi;
};
//=========================================================================

//*************************************************************************
//** The Find File class                                                 **
//*************************************************************************
class FindFile
{
	HANDLE handle;
	WIN32_FIND_DATA fileinfo;

public:
	FindFile(CString);
	virtual ~FindFile();
	bool FindNext();
	TCHAR *GetName();
	bool IsFile();
	bool Success();
};
//=========================================================================
