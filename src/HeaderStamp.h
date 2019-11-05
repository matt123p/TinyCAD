/*
 * Project:		TinyCAD program for schematic capture
 *				https://www.tinycad.net
 * Copyright:	© 1994-2019 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#pragma once

#include "stream.h"

//*************************************************************************
//*                                                                       *
//*  Represents the bytes identifying a tinycad file                      *
//*                                                                       *
//*************************************************************************

//=========================================================================
class CHeaderStamp
{
	//=====================================================================
	//== ctor/dtor/initializing                                          ==
	//=====================================================================
public:
	CHeaderStamp();
private:
	void Init();

	//=====================================================================
	//== accessor                                                        ==
	//=====================================================================
	//-- Write to a file
public:
	void Write(CStream& oArchive) const;

	//-- Return true if file reading was OK
public:
	bool IsChecked(bool ReportError) const;

	//-- Show error dialog if errors occure
private:
	void ShowErrorMsg(int nMsgID, bool bIsErrorReport) const;

	//=====================================================================
	//== mutator                                                         ==
	//=====================================================================
	//-- Read from a file
public:
	void Read(CStream& oArchive);

private:
	//=====================================================================
	//== class constants                                                 ==
	//=====================================================================
	//-- 7 bit ASCII code, with a one byte length prefix
	//-- 8 bytes size
	static const CString M_SNAME;

	//-- revision number of the file format it saves
	//-- 1 byte size
	static const BYTE M_NREVISION;

	//-- magic number to spot TinyCAD files
	//-- 2 bytes size
	static const short M_NMAGIC;

	//=====================================================================
	//== class variables                                                 ==
	//=====================================================================
	//-- Program name
	CString m_sName;

	//-- file format revision
	BYTE m_nRevision;

	//-- identifying number for TinyCAD files
	short m_nMagic;
};
//=========================================================================
