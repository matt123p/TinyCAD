/*
 * Project:		TinyCAD program for schematic capture
 *				https://www.tinycad.net/
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#ifndef __AUTOSAVE_H__
#define __AUTOSAVE_H__

#pragma once
#include "NewTypes.h"

//=========================================================================
class CAutoSave
{
	//=====================================================================
	//== class constants                                                 ==
	//=====================================================================
private:
	static const int M_NID;

	//=====================================================================
	//== class variables                                                 ==
	//=====================================================================
	//-- Time in minutes between autosave events
private:
	static UInt32 m_nDelay;

	//-- Singleton
private:
	static CAutoSave* m_pInstance;

	//=====================================================================
	//== ctor/dtor/initializing                                          ==
	//=====================================================================
private:
	CAutoSave();
public:
	~CAutoSave();

	//-- C++ does not guarantee a certain sequence in the creating of 
	//-- global objects, that's why the direct call of 
	//-- CRegistry functions in CAutoSave ctor raises errors.
	//-- Using GetInstance() creates an CAutoSave object only on demand 
private:
	static CAutoSave* GetInstance();

	//=====================================================================
	//== accessor/mutator                                                ==
	//=====================================================================
	//-- Returns true if the Timer is running.
public:
	static boolean IsEnabled();

	//-- Enables/Disables timer events
public:
	static void SetEnabled(boolean bIsEnabled);

	//-- Returns the delay in minutes between timer events
public:
	static UInt32 GetDelay();

	//-- Sets the Timer's delay, the number of minutes between
	//-- successive action events
public:
	static void SetDelay(UInt32 nTimeSpan);

	//=====================================================================
	//== Timer action                                                    ==
	//=====================================================================
	//-- Starts the Timer, causing it to start sending action events
	//-- to its listener window loop
public:
	static void Start();

	//-- Stops the Timer, causing it to stop sending action events
	//-- to its listener window
public:
	static void Stop();
};
//=========================================================================

#endif // __AUTOSAVE_H__
