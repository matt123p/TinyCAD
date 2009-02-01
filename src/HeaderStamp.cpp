/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "HeaderStamp.h"
#include "TinyCad.h"		

//*************************************************************************
//*                                                                       *
//*  Represents the bytes identifying a tinycad file                      *
//*                                                                       *
//*************************************************************************

const CString	CHeaderStamp::M_SNAME		= CTinyCadApp::GetName();
const BYTE 		CHeaderStamp::M_NREVISION	= 0x06;
const short 	CHeaderStamp::M_NMAGIC		= 0x0C5A;

//=========================================================================
//== ctor/dtor/initializing                                              ==
//=========================================================================

//-------------------------------------------------------------------------
CHeaderStamp::CHeaderStamp()
{
	Init();
}
//-------------------------------------------------------------------------
void CHeaderStamp::Init()
{
	m_sName		= M_SNAME;
	m_nRevision	= M_NREVISION;
	m_nMagic	= M_NMAGIC;
}
//-------------------------------------------------------------------------

//=========================================================================
//== accessor                                                            ==
//=========================================================================

//-------------------------------------------------------------------------
//-- Write to a file
//-- Forces the writing of the current constant values
//-- In the future with different format versions
//-- this behaviour has to be changed
void CHeaderStamp::Write( CStream& oArchive ) const
{
	oArchive << M_SNAME;
	oArchive << M_NREVISION;
	oArchive << M_NMAGIC;
}
//-------------------------------------------------------------------------
//-- Return true if file reading was OK
bool CHeaderStamp::IsChecked( bool bIsErrorReport ) const
{
	bool bReturn = false;

	if( (m_sName != M_SNAME) || (m_nMagic != M_NMAGIC) )
	{
		// Not written by this program
		ShowErrorMsg( IDS_ABORTNOTMINE, bIsErrorReport );
	}
	else if( m_nRevision > M_NREVISION )
	{
		// Written by a more advanced version of this program
		ShowErrorMsg( IDS_ABORTVERSION, bIsErrorReport );
	}
	else
	{
		bReturn = true;
	}

	return bReturn;
}
//-------------------------------------------------------------------------
//-- Show error dialog if errors occure
void CHeaderStamp::ShowErrorMsg( int nMsgID, bool bIsErrorReport ) const
{
	if( bIsErrorReport )
	{
		Message( nMsgID, MB_ICONEXCLAMATION );
	}
}
//-------------------------------------------------------------------------

//=========================================================================
//== mutator                                                             ==
//=========================================================================

//-------------------------------------------------------------------------
//-- Read from a file
void CHeaderStamp::Read( CStream& oArchive )
{
	oArchive >> m_sName;
	oArchive >> m_nRevision;
	oArchive >> m_nMagic;
}
//-------------------------------------------------------------------------
