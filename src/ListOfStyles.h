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
//*                Storage of line style                                  *
//*                                                                       *
//*************************************************************************
// The line style type
typedef hRESOURCE hSTYLE;

//=========================================================================
class LineStyle
{
public:
	Int32 Style;
public:
	Int32 Thickness;
public:
	COLORREF Colour;
};
//=========================================================================
// Storage of line style settings
class ListOfStyles: public CDocResource
{
	typedef CDocResource super;

	//-- The Line style this describes
private:
	LineStyle m_oStyle;

	//=====================================================================
	//== ctor/dtor/initializing                                          ==
	//=====================================================================
public:
	ListOfStyles();
public:
	ListOfStyles(const ListOfStyles& oStyle);
public:
	ListOfStyles(LineStyle s);
public:
	ListOfStyles(Int32 nStyle, Int32 nThickness, COLORREF crLine);
public:
	virtual ~ListOfStyles();
private:
	void Init();
private:
	void Init(Int32 nStyle, Int32 nThickness, COLORREF crLine);

	//=====================================================================
	//== accessor/mutator                                                ==
	//=====================================================================
	//-- Get the data associated with this resource
public:
	LineStyle* GetLineStylePtr();

	//=====================================================================
	//== serialization                                                   ==
	//=====================================================================
public:
	virtual void ReadNative(CStream& oStream);
public:
	virtual void Read(CXMLReader& xml, hSTYLE& nID);
public:
	virtual void Write(CXMLWriter& xml) const;

public:
	virtual void SaveItemXML(CTinyCadDoc*, CXMLWriter& xml);
public:
	virtual void LoadItem(CTinyCadDoc*, CStream &, hRESOURCE);
public:
	virtual void LoadItemXML(CTinyCadDoc*, CXMLReader& xml);

	//=====================================================================
	//== Comparision                                                     ==
	//=====================================================================
	//-- Compare this resource with another resource of the same type
public:
	virtual BOOL Compare(CDocResource *);
};
//=========================================================================
