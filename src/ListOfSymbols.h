/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#pragma once

#include "NewTypes.h"
#include "DocResource.h"
class CDesignFileSymbol;

typedef hRESOURCE hSYMBOL;

//*************************************************************************
//*                                                                       *
//*                Storage of font settings                               *
//*                                                                       *
//*************************************************************************

//=========================================================================
class ListOfSymbols
: public CDocResource
{
	typedef CDocResource super;

	private:	CDesignFileSymbol* m_pSymbol;

	//=====================================================================
	//== ctor/dtor/initializing                                          ==
	//=====================================================================
	public:						ListOfSymbols();
	public:						ListOfSymbols( const ListOfSymbols& o );
	public:						ListOfSymbols( CDesignFileSymbol* oSymbol );
	public:	virtual				~ListOfSymbols();
	private:			void	Init();
	private:			void	Init( CDesignFileSymbol* oSymbol );

	//=====================================================================
	//== accessor/mutator                                                ==
	//=====================================================================
	//-- Get the data associated with this resource
	public:			CDesignFileSymbol* 	GetSymbol();

	//=====================================================================
	//== serialization                                                   ==
	//=====================================================================

	public:virtual void SaveItemXML( CTinyCadDoc*, CXMLWriter& xml );
	public:virtual void LoadItem( CTinyCadDoc* pDesign, CStream&, hRESOURCE );
	public:virtual void LoadItemXML( CTinyCadDoc* pDesign, CXMLReader& xml );

	//=====================================================================
	//== Comparison                                                      ==
	//=====================================================================
	//-- Compare this resource with another resource of the same type
	public:	virtual	BOOL		Compare( CDocResource* );
};
//=========================================================================
