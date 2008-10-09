/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "AutoSave.h"
#include "Registry.h"

//=========================================================================
//== init class constants                                                ==
//=========================================================================
//-- Remember: The timer event identifier 1 was used in Startup.cpp
const int	CAutoSave::M_NID 		= 2;

//=========================================================================
//== init class variables                                                ==
//=========================================================================
UInt32		CAutoSave::m_nDelay		= 10;
CAutoSave*	CAutoSave::m_pInstance	= NULL;

//=========================================================================
//== ctor/dtor/initializing                                              ==
//=========================================================================

//-------------------------------------------------------------------------
CAutoSave::CAutoSave()
{
	// Timer's initial delay is ten minutes
	m_nDelay = CRegistry::GetInt( "AutoSave", 10 );
}
//-------------------------------------------------------------------------
CAutoSave*	CAutoSave::GetInstance()
{
	if( m_pInstance == NULL )
	{
		m_pInstance = new CAutoSave();
	}

	return m_pInstance;
}
//-------------------------------------------------------------------------
CAutoSave::~CAutoSave()
{
	Stop();
}
//-------------------------------------------------------------------------

//=========================================================================
//== accessor/mutator                                                    ==
//=========================================================================

//-------------------------------------------------------------------------
//-- Returns true if the Timer is running.
boolean CAutoSave::IsEnabled()
{
	return (GetDelay() > 0);
}
//-------------------------------------------------------------------------
//-- Enables/Disables timer events
void CAutoSave::SetEnabled( boolean bIsEnabled )
{
	if( bIsEnabled )
	{
		if( GetInstance()->m_nDelay == 0 )
		{
			// Force minimal delay
			SetDelay( 1 );
		}
	}
	else
	{
		SetDelay( 0 );
	}
}
//-------------------------------------------------------------------------
//-- Returns the delay in minutes between timer events
UInt32 CAutoSave::GetDelay()
{
	return GetInstance()->m_nDelay;
}
//-------------------------------------------------------------------------
//-- Sets the Timer's delay, the number of minutes between
//-- successive action events
void CAutoSave::SetDelay( UInt32 nDelay )
{
	GetInstance()->m_nDelay = nDelay;

	CRegistry::Set( "AutoSave", m_nDelay );
}
//-------------------------------------------------------------------------

//=========================================================================
//== Timer action                                                        ==
//=========================================================================

//-------------------------------------------------------------------------
//-- Starts the Timer, causing it to start sending action events
//-- to its listener window loop
void CAutoSave::Start()
{
	if( IsEnabled() )
	{
		::SetTimer( AfxGetMainWnd()->m_hWnd, M_NID, m_nDelay * 60 * 1000 , NULL );
	}
}
//-------------------------------------------------------------------------
//-- Stops the Timer, causing it to stop sending action events
//-- to its listener window
void CAutoSave::Stop()
{
	::KillTimer( AfxGetMainWnd()->m_hWnd, M_NID );
}
//-------------------------------------------------------------------------
