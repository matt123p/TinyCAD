/*
 * Project:		TinyCAD program for schematic capture
 *				https://www.tinycad.net
 * Copyright:	© 1994-2019 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "ListOfFillStyles.h"
#include "Colour.h"

//=========================================================================
//== ctor/dtor/initializing                                              ==
//=========================================================================

//-------------------------------------------------------------------------
ListOfFillStyles::ListOfFillStyles()
{
	Init();
}
//-------------------------------------------------------------------------
ListOfFillStyles::ListOfFillStyles(Int32 nIndex, COLORREF crFill)
{
	Init(nIndex, crFill);
}
//-------------------------------------------------------------------------
ListOfFillStyles::ListOfFillStyles(FillStyle oStyle)
{
	Init(oStyle.Index, oStyle.Colour);
}
//-------------------------------------------------------------------------
ListOfFillStyles::~ListOfFillStyles()
{
}
//-------------------------------------------------------------------------
//-- The default line style
void ListOfFillStyles::Init()
{
	Init(-1, cBLACK);
}
//-------------------------------------------------------------------------
void ListOfFillStyles::Init(Int32 nIndex, COLORREF crFill)
{
	m_oFillStyle.Index = nIndex;
	m_oFillStyle.Colour = crFill;
}
//-------------------------------------------------------------------------

//=========================================================================
//== accessor/mutator                                                    ==
//=========================================================================

//-------------------------------------------------------------------------
FillStyle* ListOfFillStyles::GetFillStylePtr()
{
	return &m_oFillStyle;
}
//-------------------------------------------------------------------------

//=========================================================================
//== serialization                                                       ==
//=========================================================================

//-------------------------------------------------------------------------
void ListOfFillStyles::ReadNative(CStream& oStream)
{
	Init();

	oStream.Read(&m_oFillStyle, sizeof(FillStyle));
}
//-------------------------------------------------------------------------
void ListOfFillStyles::Read(CXMLReader& xml, hFILL& nID)
{
	CString name;

	Init();

	xml.getAttribute(_T("id"), nID);
	xml.intoTag();

	while (xml.nextTag(name))
	{
		if (name == "INDEX")
		{
			xml.getChildData(m_oFillStyle.Index);
		}
		else if (name == "COLOR")
		{
			xml.getChildData(m_oFillStyle.Colour);
		}
	}

	xml.outofTag();
}
//-------------------------------------------------------------------------
void ListOfFillStyles::Write(CXMLWriter& xml) const
{
	xml.addTag(_T("INDEX"), m_oFillStyle.Index);
	xml.addTag(_T("COLOR"), m_oFillStyle.Colour);
}
//-------------------------------------------------------------------------
void ListOfFillStyles::SaveItemXML(CTinyCadDoc*, CXMLWriter& xml)
{
	Write(xml);
}
//-------------------------------------------------------------------------
void ListOfFillStyles::LoadItemXML(CTinyCadDoc*, CXMLReader &xml)
{
	ListOfFillStyles oFillStyle;
	hFILL nID;

	oFillStyle.Read(xml, nID);

	Add(new ListOfFillStyles(oFillStyle), nID);
}
//-------------------------------------------------------------------------
void ListOfFillStyles::LoadItem(CTinyCadDoc*, CStream& oStream, hRESOURCE n)
{
	ListOfFillStyles oStyle;

	oStyle.ReadNative(oStream);

	Add(new ListOfFillStyles(oStyle), n);
}
//-------------------------------------------------------------------------

//=========================================================================
//== Comparison                                                          ==
//=========================================================================

//-------------------------------------------------------------------------
BOOL ListOfFillStyles::Compare(CDocResource* o)
{
#define  pFillStyle (((ListOfFillStyles *)o)->m_oFillStyle)

	return pFillStyle.Colour == m_oFillStyle.Colour && pFillStyle.Index == m_oFillStyle.Index;
}
//-------------------------------------------------------------------------
