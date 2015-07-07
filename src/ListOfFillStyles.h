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

//*************************************************************************
//*                                                                       *
//*                Storage of fill style settings                         *
//*                                                                       *
//*************************************************************************

// The fill style type
typedef hRESOURCE hFILL;

//=========================================================================
class FillStyle
{
public:
	Int32 Index; // -1:No fill 0:Black fill
public:
	COLORREF Colour;
};
//=========================================================================
class ListOfFillStyles: public CDocResource
{
	typedef CDocResource super;

	// The Fill style this describes
private:
	FillStyle m_oFillStyle;

	//=====================================================================
	//== ctor/dtor/initializing                                          ==
	//=====================================================================
public:
	ListOfFillStyles();
public:
	ListOfFillStyles(Int32 nIndex, COLORREF crLine);
public:
	ListOfFillStyles(FillStyle oStyle);
public:
	~ListOfFillStyles();
private:
	void Init();
private:
	void Init(Int32 nIndex, COLORREF crLine);

	//=====================================================================
	//== accessor/mutator                                                ==
	//=====================================================================
	// Get the data associated with this resource
public:
	FillStyle* GetFillStylePtr();

	//=====================================================================
	//== serialization                                                   ==
	//=====================================================================
public:
	virtual void ReadNative(CStream& oStream);
public:
	virtual void Read(CXMLReader& xml, hFILL& nID);
public:
	virtual void Write(CXMLWriter& xml) const;

public:
	virtual void LoadItem(CTinyCadDoc*, CStream&, hRESOURCE);
public:
	virtual void SaveItemXML(CTinyCadDoc*, CXMLWriter& xml);
public:
	virtual void LoadItemXML(CTinyCadDoc*, CXMLReader& xml);

	//=====================================================================
	//== Comparision                                                     ==
	//=====================================================================
	//-- Compare this resource with another resource of the same type
public:
	virtual BOOL Compare(CDocResource*);
};
//=========================================================================
