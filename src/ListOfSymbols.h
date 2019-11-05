/*
 * Project:		TinyCAD program for schematic capture
 *				https://www.tinycad.net
 * Copyright:	© 1994-2019 Matt Pyne
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
class ListOfSymbols: public CDocResource
{
	typedef CDocResource super;

private:
	CDesignFileSymbol* m_pSymbol;

	//=====================================================================
	//== ctor/dtor/initializing                                          ==
	//=====================================================================
public:
	ListOfSymbols();
	ListOfSymbols(const ListOfSymbols& o);
	ListOfSymbols(CDesignFileSymbol* oSymbol);
	virtual ~ListOfSymbols();
private:
	void Init();
	void Init(CDesignFileSymbol* oSymbol);

	//=====================================================================
	//== accessor/mutator                                                ==
	//=====================================================================
	//-- Get the data associated with this resource
public:
	CDesignFileSymbol* GetSymbol();

	//=====================================================================
	//== serialization                                                   ==
	//=====================================================================

	virtual void SaveItemXML(CTinyCadDoc*, CXMLWriter& xml);
	virtual void LoadItem(CTinyCadDoc* pDesign, CStream&, hRESOURCE);
	virtual void LoadItemXML(CTinyCadDoc* pDesign, CXMLReader& xml);

	//=====================================================================
	//== Comparison                                                      ==
	//=====================================================================
	//-- Compare this resource with another resource of the same type
	virtual BOOL Compare(CDocResource*);
};
//=========================================================================
