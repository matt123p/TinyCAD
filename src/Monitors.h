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
#pragma once

//this is only necessary if the app is supporting win95
#if WINVER < 0x0500
#include "multimon.h"
#endif // WINVER < 0x0500

class CMonitor;

// CMonitors command target

class CMonitors : public CObject
{
public:
	CMonitors();
	virtual ~CMonitors();

	CMonitor GetMonitor( const int index ) const;


#if _MFC_VER >= 0x0700
	int GetCount() const { return (int)m_MonitorArray.GetCount(); } 
#else
	int GetCount() const { return (int)m_MonitorArray.GetSize(); } 
#endif

//static members
	static CMonitor GetNearestMonitor( const LPRECT lprc );
	static CMonitor GetNearestMonitor( const POINT pt );
	static CMonitor GetNearestMonitor( const CWnd* pWnd );

	static BOOL IsOnScreen( const POINT pt );
	static BOOL IsOnScreen( const CWnd* pWnd );
	static BOOL IsOnScreen( const LPRECT lprc );

	static void GetVirtualDesktopRect( LPRECT lprc );

	static BOOL IsMonitor( const HMONITOR hMonitor );

	static CMonitor GetPrimaryMonitor();
	static BOOL AllMonitorsShareDisplayFormat();

	static int GetMonitorCount();

private:
	CObArray m_MonitorArray;

	typedef struct tagMATCHMONITOR
	{
		HMONITOR target;
		BOOL foundMatch;
	} MATCHMONITOR, *LPMATCHMONITOR;

	static BOOL CALLBACK FindMatchingMonitorHandle(
		HMONITOR hMonitor,  // handle to display monitor
		HDC hdcMonitor,     // handle to monitor DC
		LPRECT lprcMonitor, // monitor intersection rectangle
		LPARAM dwData       // data
	);


	typedef struct tagADDMONITOR
	{
		CObArray* pMonitors;
		int currentIndex;
	} ADDMONITOR, *LPADDMONITOR;

	static BOOL CALLBACK AddMonitorsCallBack(
		HMONITOR hMonitor,  // handle to display monitor
		HDC hdcMonitor,     // handle to monitor DC
		LPRECT lprcMonitor, // monitor intersection rectangle
		LPARAM dwData       // data
	);

};


