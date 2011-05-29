/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "LibraryCollection.h"
#include "ListOfSymbols.h"

//=========================================================================
//== ctor/dtor/initializing                                              ==
//=========================================================================

//-------------------------------------------------------------------------
ListOfSymbols::ListOfSymbols() :
	super()
{
	Init();
}
//-------------------------------------------------------------------------
ListOfSymbols::ListOfSymbols(const ListOfSymbols& o)
{
	Init(o.m_pSymbol);
}
//-------------------------------------------------------------------------
ListOfSymbols::ListOfSymbols(CDesignFileSymbol* pSymbol) :
	super()
{
	Init(pSymbol);
}
//-------------------------------------------------------------------------
ListOfSymbols::~ListOfSymbols()
{
	delete m_pSymbol;
}
//-------------------------------------------------------------------------
void ListOfSymbols::Init()
{
	Init(NULL);
}
//-------------------------------------------------------------------------
void ListOfSymbols::Init(CDesignFileSymbol* pSymbol)
{
	m_pSymbol = pSymbol;
}
//-------------------------------------------------------------------------

//=========================================================================
//== accessor/mutator                                                    ==
//=========================================================================

//-------------------------------------------------------------------------
CDesignFileSymbol* ListOfSymbols::GetSymbol()
{
	return m_pSymbol;
}
//-------------------------------------------------------------------------

//=========================================================================
//== serialization                                                       ==
//=========================================================================

//-------------------------------------------------------------------------
void ListOfSymbols::SaveItemXML(CTinyCadDoc *pDesign, CXMLWriter &xml)
{
	// Save the symbol with the document...
	m_pSymbol->SaveXML(xml);
}
//-------------------------------------------------------------------------
void ListOfSymbols::LoadItemXML(CTinyCadDoc *pDesign, CXMLReader &xml)
{
	hRESOURCE n;
	CString name;
	CDesignFileSymbol *psymbol = new CDesignFileSymbol();

	xml.getAttribute(_T("id"), n);
	xml.intoTag();

	psymbol->LoadXML(pDesign, xml);

	xml.outofTag();

	ListOfSymbols *nFS = new ListOfSymbols(psymbol);
	Add(nFS, n);
}
//-------------------------------------------------------------------------
void ListOfSymbols::LoadItem(CTinyCadDoc *pDesign, CStream &theFile, hRESOURCE n)
{
	// Save the symbol with the document...
	CDesignFileSymbol *psymbol = new CDesignFileSymbol;
	psymbol->Load(pDesign, theFile);

	ListOfSymbols *nFS = new ListOfSymbols(psymbol);
	Add(nFS, n);
}
//-------------------------------------------------------------------------

//=========================================================================
//== Comparison                                                          ==
//=========================================================================

//-------------------------------------------------------------------------
// The resource for line styles
BOOL ListOfSymbols::Compare(CDocResource *o)
{
	CDesignFileSymbol *pSymbol = static_cast<CDesignFileSymbol*> ( ((ListOfSymbols *) o)->m_pSymbol);

	// Correct for bug in previous versions of TinyCAD XML parsing
	if (m_pSymbol->name.IsEmpty() || pSymbol->name.IsEmpty())
	{
		return FALSE;
	}

	// Now do the correct testing...
	//This test will often yield wrong result because FilePos is always -1 when editing existing symbols on a sheet.
	//// return m_pSymbol->name == pSymbol->name && m_pSymbol->FilePos == pSymbol->FilePos;

	// Do a deep compare of all symboldef contents.
	// (The ref_point info will not be taken into account)
	return *m_pSymbol == *pSymbol;
}
//-------------------------------------------------------------------------
