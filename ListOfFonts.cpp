/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "ListOfFonts.h"

const CString 	ListOfFonts::DEFAULT_FONT 	= "Arial";
const Int32 	ListOfFonts::TEXT_HEIGHT 	= -13;
const Int32 	ListOfFonts::RULE_HEIGHT 	= -10;
const Int32 	ListOfFonts::PIN_HEIGHT 	= -10;

//=========================================================================
//== ctor/dtor/initializing                                              ==
//=========================================================================

//-------------------------------------------------------------------------
ListOfFonts::ListOfFonts()
: super()
{
	Init();
}
//-------------------------------------------------------------------------
ListOfFonts::ListOfFonts( const ListOfFonts& oFont )
: super()
{
	Init( oFont.GetFont() );
}
//-------------------------------------------------------------------------
ListOfFonts::ListOfFonts( LOGFONT oFont )
: super()
{
	Init( oFont );
}
//-------------------------------------------------------------------------
ListOfFonts::ListOfFonts( Int32 nHeight, bool bIsItalic )
: super()
{
	Init();

	m_oFont.lfHeight = nHeight;
	m_oFont.lfItalic = bIsItalic;
}
//-------------------------------------------------------------------------
ListOfFonts::~ListOfFonts()
{
}
//-------------------------------------------------------------------------
void ListOfFonts::Init()
{
	Init( ListOfFonts::GetDefaultFont() );
}
//-------------------------------------------------------------------------
void ListOfFonts::Init( LOGFONT oFont )
{
	// Set only relevant parts
	memcpy( &m_oFont, &oFont, sizeof( oFont ) );
}
//-------------------------------------------------------------------------

//=========================================================================
//== accessor/mutator                                                    ==
//=========================================================================

//-------------------------------------------------------------------------
LOGFONT ListOfFonts::GetFont() const
{
	return m_oFont;
}
//-------------------------------------------------------------------------
LOGFONT& ListOfFonts::GetFontRef()
{
	return m_oFont;
}
//-------------------------------------------------------------------------
LOGFONT* ListOfFonts::GetFontPtr()
{
	return &m_oFont;
}
//-------------------------------------------------------------------------
LOGFONT	ListOfFonts::GetDefaultFont()
{
	LOGFONT oReturn;

	memset( &oReturn, 0, sizeof(LOGFONT) );

  	oReturn.lfHeight 			= ListOfFonts::TEXT_HEIGHT;
  	oReturn.lfWidth 			= 0;
  	oReturn.lfWeight 			= 400;
  	oReturn.lfOrientation		= 0;
 	oReturn.lfItalic	 		= FALSE;
 	oReturn.lfUnderline 		= FALSE;
  	oReturn.lfStrikeOut 		= FALSE;
  	_tcscpy( oReturn.lfFaceName, ListOfFonts::DEFAULT_FONT );

  	// default values will never change
  	oReturn.lfEscapement 		= 0;
  	oReturn.lfOutPrecision 		= OUT_DEFAULT_PRECIS;
  	oReturn.lfClipPrecision 	= CLIP_DEFAULT_PRECIS | CLIP_LH_ANGLES;
  	oReturn.lfCharSet 			= ANSI_CHARSET;
  	oReturn.lfQuality 			= DEFAULT_QUALITY;
  	oReturn.lfPitchAndFamily 	= DEFAULT_PITCH | FF_SWISS;

  	return oReturn;
}
//-------------------------------------------------------------------------

//=========================================================================
//== serialization                                                       ==
//=========================================================================

//-------------------------------------------------------------------------
void ListOfFonts::ReadNative( CStream& oStream )
{
	Init();
#ifdef UNICODE
	LOGFONTA in;
	oStream.Read( &in, sizeof(LOGFONTA) );

	memcpy( &m_oFont, &in, sizeof(LOGFONTA) );
	int l = MultiByteToWideChar( CP_ACP, 0, in.lfFaceName, strlen( in.lfFaceName ), m_oFont.lfFaceName, sizeof( m_oFont.lfFaceName )/sizeof( wchar_t ) );
	m_oFont.lfFaceName[ l ] = 0;
#else
	oStream.Read( &m_oFont, sizeof(LOGFONT) );
#endif
	// Init unused parts of LOGFONT structure
//  	m_oFont.lfEscapement 		= 0;
//  	m_oFont.lfOutPrecision 		= OUT_DEFAULT_PRECIS;
//  	m_oFont.lfClipPrecision 	= CLIP_DEFAULT_PRECIS | CLIP_LH_ANGLES;
//  	m_oFont.lfCharSet 			= ANSI_CHARSET;
//  	m_oFont.lfQuality 			= DEFAULT_QUALITY;
//  	m_oFont.lfPitchAndFamily 	= DEFAULT_PITCH | FF_SWISS;
}
//-------------------------------------------------------------------------
void ListOfFonts::Write( CXMLWriter& xml ) const
{
	xml.addTag( _T("HEIGHT"), 		m_oFont.lfHeight );
	xml.addTag( _T("WIDTH"), 		m_oFont.lfWidth );
  	xml.addTag( _T("WEIGHT"), 		m_oFont.lfWeight );
  	xml.addTag( _T("ITALIC"), 		m_oFont.lfItalic );
  	xml.addTag( _T("UNDERLINE"), 	m_oFont.lfUnderline );
  	xml.addTag( _T("STRIKEOUT"), 	m_oFont.lfStrikeOut );
  	xml.addTag( _T("CHARSET"), 		m_oFont.lfCharSet );
  	xml.addTag( _T("FACENAME"), 	m_oFont.lfFaceName );
}
//-------------------------------------------------------------------------
void ListOfFonts::Read( CXMLReader& xml, hFONT& nID )
{
	CString name;

	Init();

	xml.getAttribute( _T("id"), nID );
	xml.intoTag();

	while (xml.nextTag( name ))
	{
		if (name == "HEIGHT")
		{
			xml.getChildData(m_oFont.lfHeight);
		}
		else if (name == "WIDTH")
		{
			xml.getChildData(m_oFont.lfWidth);
		}
		else if (name == "WEIGHT")
		{
			xml.getChildData(m_oFont.lfWeight);
		}
		else if (name == "ITALIC")
		{
			xml.getChildData(m_oFont.lfItalic);
		}
		else if (name == "UNDERLINE")
		{
			xml.getChildData(m_oFont.lfUnderline);
		}
		else if (name == "STRIKEOUT")
		{
			xml.getChildData(m_oFont.lfStrikeOut);
		}
		else if (name == "CHARSET")
		{
			xml.getChildData(m_oFont.lfCharSet);
		}
		else if (name == "FACENAME")
		{
			CString s;
			xml.getChildData(s);
			_tcscpy(m_oFont.lfFaceName, s );
		}
	}

	xml.outofTag();
}
//-------------------------------------------------------------------------
void ListOfFonts::LoadItem(CTinyCadDoc *pDesign, CStream& oStream, hRESOURCE n)
{
	ListOfFonts oFont;

	oFont.ReadNative( oStream );

	Add( new ListOfFonts(oFont), n );
}
//-------------------------------------------------------------------------
void ListOfFonts::SaveItemXML( CTinyCadDoc *pDesign, CXMLWriter &xml)
{
	Write( xml );
}
//-------------------------------------------------------------------------
void ListOfFonts::LoadItemXML( CTinyCadDoc* pDesign, CXMLReader& xml )
{
	hRESOURCE 		nID;
	ListOfFonts		oFont;

	oFont.Read( xml, nID );

	Add( new ListOfFonts(oFont), nID );
}
//-------------------------------------------------------------------------

//=========================================================================
//== Comparison                                                          ==
//=========================================================================

//-------------------------------------------------------------------------
BOOL ListOfFonts::Compare( CDocResource* o )
{
	#define  pFont ((ListOfFonts *)o)->m_oFont

	BOOL  bReturn =	pFont.lfHeight    == m_oFont.lfHeight &&
					pFont.lfWidth     == m_oFont.lfWidth &&
			  		pFont.lfWeight    == m_oFont.lfWeight &&
			  		pFont.lfItalic    == m_oFont.lfItalic &&
			  		pFont.lfUnderline == m_oFont.lfUnderline &&
			  		pFont.lfStrikeOut == m_oFont.lfStrikeOut &&
			  		pFont.lfCharSet   == m_oFont.lfCharSet &&
			  		_tcscmp(pFont.lfFaceName,m_oFont.lfFaceName)==0;

	return bReturn;
}
//-------------------------------------------------------------------------
