//////////////////////////////////////////////////
// CMonitor - wrapper to Win32 multi-monitor API
//
// Author: Donald Kackman
// Email:  dkackman_2000@yahoo.com
// Copyright 2002, Donald Kackman
//
// You may freely use or modify this code provided this
// Copyright is included in all derived versions.
//
///////////////////////////////////////////////////

//
//David Campbell's article
//How to Exploit Multiple Monitor Support in Memphis and Windows NT 5.0
//is very helpful for multimonitor api calls
//http://www.microsoft.com/msj/defaultframe.asp?page=/msj/0697/monitor/monitor.htm&nav=/msj/0697/newnav.htm
//
#include "stdafx.h"

// this file includes the multimon stub function
// that are implmented in multimon.h

// this is only necessary if the app is supporting win95
#if !defined(COMPILE_MULTIMON_STUBS) && (WINVER < 0x0500)
//
// this needs to be here so that we get the wrapper stubs that deal with win95 platforms
#define COMPILE_MULTIMON_STUBS
#include "multimon.h"
//
#endif // !defined(COMPILE_MULTIMON_STUBS) && (WINVER < 0x0500)
