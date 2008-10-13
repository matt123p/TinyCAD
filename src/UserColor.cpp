/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "UserColor.h"
#include "Registry.h"
#include "Colour.h"
#include "Assert.h"

//=========================================================================
//== init static variables                                               ==
//=========================================================================
const int		CUserColor::WIRE		= 1;
const int		CUserColor::BUS			= 2;
const int		CUserColor::JUNCTION	= 3;
const int		CUserColor::NOCONNECT	= 4;
const int		CUserColor::LABEL		= 5;
const int		CUserColor::POWER		= 6;
const int		CUserColor::PIN			= 7;
const int		CUserColor::HIDDEN_PIN	= 8;
const int		CUserColor::BACKGROUND  = 9;

TNamesMap 		CUserColor::m_colNames;

//=========================================================================
//== ctor/dtor/initializing                                              ==
//=========================================================================

//-------------------------------------------------------------------------
CUserColor::CUserColor()
{
	Init();
	ReadRegistry();
}
//-------------------------------------------------------------------------
CUserColor::CUserColor( CUserColor& oColor )
{
	Init();
	ReadRegistry();

	for( TColorRefMapIterator i = m_colColorRef.begin(); i != m_colColorRef.end(); i++ )
	{
		int nID = i->first;
		m_colColorRef[ nID ] = oColor.Get( nID );
	}
}
//-------------------------------------------------------------------------
/** Set the default colours (ignoring the registry). */
void CUserColor::Init()
{
	// Create the default colors
	m_colNames.clear();
	m_colNames[ WIRE ] 			= "COLOR_WIRE";
	m_colNames[ BUS ] 			= "COLOR_BUS";
	m_colNames[ JUNCTION ] 		= "COLOR_JUNCTION";
	m_colNames[ NOCONNECT ] 	= "COLOR_NOCONNECT";
	m_colNames[ LABEL ] 		= "COLOR_LABEL";
	m_colNames[ POWER ] 		= "COLOR_POWER";
	m_colNames[ PIN ] 			= "COLOR_PIN";
	m_colNames[ HIDDEN_PIN ] 	= "COLOR_HIDDEN_PIN";
	m_colNames[ BACKGROUND ] 	= "COLOR_BACKGROUND";

	m_colColorRef.clear();
	m_colColorRef[ WIRE ] 		= cBLUE;
	m_colColorRef[ BUS ] 		= cRED;
	m_colColorRef[ JUNCTION ] 	= cBLACK;
	m_colColorRef[ NOCONNECT ] 	= cBLACK;
	m_colColorRef[ LABEL ] 		= RGB(0,128,32);
	m_colColorRef[ POWER ] 		= cBLACK;
	m_colColorRef[ PIN ] 		= RGB(192,64,64);
	m_colColorRef[ HIDDEN_PIN ] = RGB(32,128,32);
	m_colColorRef[ BACKGROUND ] = RGB(255,255,255);
}
//-------------------------------------------------------------------------

//=========================================================================
//== Accessor                                                            ==
//=========================================================================

//-------------------------------------------------------------------------
COLORREF CUserColor::Get( int nID )
{
	assert( m_colColorRef.find(nID) != m_colColorRef.end() );

	COLORREF crReturn = m_colColorRef[ nID ];

	return crReturn;
}
//-------------------------------------------------------------------------
/** Saves user colors in registry if default values have really changed. */
void CUserColor::WriteRegistry()
{
	CUserColor crDefault;

	crDefault.Init();

	for( TNamesMapIterator i = m_colNames.begin(); i != m_colNames.end(); i++ )
	{
		int			nID		= i->first;
		CString		sName	= i->second;
		COLORREF	cr		= m_colColorRef[ nID ];

		// Only non-default colors are stored in registry
		if( cr == crDefault.Get(nID) )
		{
			// Delete key if exists
			CRegistry::DeleteValue( sName );
		}
		else
		{
			CRegistry::Set( sName, cr );
		}
	}
}
//-------------------------------------------------------------------------

//=========================================================================
//== Mutator                                                             ==
//=========================================================================

//-------------------------------------------------------------------------
/** Sets a certain user color. */
void CUserColor::Set( int nID, COLORREF cr )
{
	assert( m_colColorRef.find(nID) != m_colColorRef.end() );

	m_colColorRef[ nID ] = cr;
}
//-------------------------------------------------------------------------
/** Set the default colours from the registry (if available). */
void CUserColor::ReadRegistry()
{
	for( TNamesMapIterator i = m_colNames.begin(); i != m_colNames.end(); i++ )
	{
		int		nColor	= i->first;
		CString	sName	= i->second;

		m_colColorRef[ nColor ] = CRegistry::GetLong( sName, m_colColorRef[ nColor ] );
	}
}
//-------------------------------------------------------------------------

//=========================================================================
//== serialization                                                       ==
//=========================================================================

//-------------------------------------------------------------------------
void CUserColor::ReadXML( CXMLReader& xml, CString sTagName )
{
	// Is this a colour?
	for( TNamesMapIterator i = m_colNames.begin(); i != m_colNames.end(); i++ )
	{
		int			nID		= i->first;
		CString		sName	= i->second;
		COLORREF	crColor;

		if( sTagName == sName )
		{
			xml.getChildData( crColor );
			m_colColorRef[ nID ] = crColor;
			break;
		}
	}
}
//-------------------------------------------------------------------------
void CUserColor::WriteXML( CXMLWriter& xml )
{
	for( TNamesMapIterator i = m_colNames.begin(); i != m_colNames.end(); i++ )
	{
		int			nID		= i->first;
		CString		sName	= i->second;
		COLORREF	crColor	= Get( nID );

		xml.addTag( sName, crColor );
	}
}
//-------------------------------------------------------------------------
