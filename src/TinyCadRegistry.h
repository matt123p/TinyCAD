/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#pragma once

#include "registry.h"

//*************************************************************************
//*                                                                       *
//*   Registry entries only for application TinyCAD                       *
//*                                                                       *
//*************************************************************************

//=========================================================================
class CTinyCadRegistry
: public CRegistry
{
	typedef CRegistry super;

	//=====================================================================
	//== ctor/dtor/initializing                                          ==
	//=====================================================================
	public:	CTinyCadRegistry();
	public:	virtual ~CTinyCadRegistry();

	//-- Write the initial data to the registry
	private:static  void	CreateDefaultEntries();

	//=====================================================================
	//== Accessor for application settings                               ==
	//=====================================================================
	//-- Returns the page size
	public:static  CSize	GetPageSize();

	//-- Returns the print scale
	public:static  double	GetPrintScale();

	//-- Returns black and white print
	public:static  bool		GetPrintBandW();

	//-- Returns MDI child window state
	public:static  bool		GetMDIMaximize();

	//-- Returns main window state
	public:static  bool		GetMaximize();

	//-- Returns list of libraries
	public: static CStringList*	GetLibraryNames();

	//=====================================================================
	//== Mutator for application settings                               ==
	//=====================================================================
	//-- Changes the page size
	public:static  void		SetPageSize( CSize szPage);

	//-- Changes the print scale
	public:static  void		SetPrintScale( double nPrintScale );

	//-- Changes black and white print
	public:static  void		SetPrintBandW( bool nPrintBandW );

	//-- Changes MDI child window state
	public:static  void		SetMDIMaximize( bool bMDIMaximize );

	//-- Changes main window state
	public:static  void		SetMaximize( bool bWndMaximize );

	//=====================================================================
	//== class constants                                                 ==
	//=====================================================================
	//-- The revision number for storage in the registry database
	private: static const CString	M_SKEY;

	private: static	const CString	M_SPAGESIZE;
	private: static	const CString	M_SPRINTSCALE;
	private: static	const CString	M_SPRINTBANDW;
	private: static	const CString	M_SMDIMAXIMIZE;
	private: static	const CString	M_SMAXIMIZE;
	private: static	const CString	M_SLIBRARIES;
};
//=========================================================================

