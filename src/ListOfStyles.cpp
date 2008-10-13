/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "ListOfStyles.h"
#include "Colour.h"

//=========================================================================
//== ctor/dtor/initializing                                              ==
//=========================================================================

//-------------------------------------------------------------------------
ListOfStyles::ListOfStyles()
{
	Init();
}
//-------------------------------------------------------------------------
ListOfStyles::ListOfStyles( const ListOfStyles& oStyle )
{
	Init( oStyle.m_oStyle.Style, oStyle.m_oStyle.Thickness, oStyle.m_oStyle.Colour );
}
//-------------------------------------------------------------------------
ListOfStyles::ListOfStyles( LineStyle s )
{
	Init( s.Style, s.Thickness, s.Colour );
}
//-------------------------------------------------------------------------
ListOfStyles::ListOfStyles( Int32 nStyle, Int32 nThickness, COLORREF crLine )
{
	Init( nStyle, nThickness, crLine );
}
//-------------------------------------------------------------------------
ListOfStyles::~ListOfStyles()
{
}
//-------------------------------------------------------------------------
//-- The default line style
void ListOfStyles::Init()
{
	Init( PS_SOLID, 1, cBLACK );
}
//-------------------------------------------------------------------------
void ListOfStyles::Init( Int32 nStyle, Int32 nThickness, COLORREF crLine )
{
	m_oStyle.Style		= nStyle;
	m_oStyle.Thickness	= nThickness;
	m_oStyle.Colour 	= crLine;
}
//-------------------------------------------------------------------------

//=========================================================================
//== accessor/mutator                                                    ==
//=========================================================================
//-- Get the data associated with this resource
LineStyle* ListOfStyles::GetLineStylePtr()
{
	return &m_oStyle;
}
//-------------------------------------------------------------------------

//=========================================================================
//== serialization                                                       ==
//=========================================================================

//-------------------------------------------------------------------------
void ListOfStyles::ReadNative( CStream& oStream )
{
	Init();

	oStream.Read( &m_oStyle, sizeof(LineStyle) );
}
//-------------------------------------------------------------------------
void ListOfStyles::Read( CXMLReader& xml, hSTYLE& nID )
{
	CString name;

	Init();

	xml.getAttribute( _T("id"), nID );
	xml.intoTag();

	while (xml.nextTag( name ))
	{
		if (name == _T("STYLE"))
		{
			xml.getChildData(m_oStyle.Style);
		}
		else if (name == _T("COLOR"))
		{
			xml.getChildData(m_oStyle.Colour);
		}
		else if (name == _T("THICKNESS"))
		{
			xml.getChildData(m_oStyle.Thickness);
		}
	}

	xml.outofTag();
}
//-------------------------------------------------------------------------
void ListOfStyles::Write( CXMLWriter& xml ) const
{
	xml.addTag( _T("STYLE"), 		m_oStyle.Style );
	xml.addTag( _T("COLOR"), 		m_oStyle.Colour );
	xml.addTag( _T("THICKNESS"), 	m_oStyle.Thickness );
}
//-------------------------------------------------------------------------
void ListOfStyles::LoadItem(CTinyCadDoc*, CStream& oStream, hRESOURCE nID )
{
	ListOfStyles oStyle;

	oStyle.ReadNative( oStream );

	Add( new ListOfStyles(oStyle), nID );
}
//-------------------------------------------------------------------------
void ListOfStyles::SaveItemXML( CTinyCadDoc*, CXMLWriter &xml)
{
	Write( xml );
}
//-------------------------------------------------------------------------
void ListOfStyles::LoadItemXML( CTinyCadDoc*, CXMLReader& xml )
{
	ListOfStyles	oStyle;
	hSTYLE			nID;

	oStyle.Read( xml, nID );

	Add( new ListOfStyles(oStyle), nID );
}
//-------------------------------------------------------------------------

//=========================================================================
//== Comparison                                                          ==
//=========================================================================

//-------------------------------------------------------------------------
// The resource for line styles
BOOL ListOfStyles::Compare( CDocResource* o )
{
	#define  pStyle (((ListOfStyles *)o)->m_oStyle)

	return 	(pStyle.Style     == m_oStyle.Style &&
		 	pStyle.Thickness == m_oStyle.Thickness &&
		 	pStyle.Colour    == m_oStyle.Colour );
}
//-------------------------------------------------------------------------
