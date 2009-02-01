/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "ListOfMetaFiles.h"
#include "ImageMetafile.h"
#include "ImagePNG.h"
#include "ImageJPEG.h"

//=========================================================================
//== ctor/dtor/initializing                                              ==
//=========================================================================

//-------------------------------------------------------------------------
ListOfMetaFiles::ListOfMetaFiles()
{
	Init( NULL );
}
//-------------------------------------------------------------------------
ListOfMetaFiles::ListOfMetaFiles( const ListOfMetaFiles& o )
{
	Init( o.m_pImage );
}
//-------------------------------------------------------------------------
ListOfMetaFiles::ListOfMetaFiles( CImage* s )
{
	Init( s );
}
//-------------------------------------------------------------------------
ListOfMetaFiles::~ListOfMetaFiles()
{
}
//-------------------------------------------------------------------------
//-- The default line style
void ListOfMetaFiles::Init( CImage* s )
{
	m_pImage = s;
}
//-------------------------------------------------------------------------

//=========================================================================
//== accessor/mutator                                                    ==
//=========================================================================

//-------------------------------------------------------------------------
//-- Get the data associated with this resource
CImage* ListOfMetaFiles::GetImage() const
{
	return m_pImage;
};
//-------------------------------------------------------------------------

//=========================================================================
//== serialization                                                       ==
//=========================================================================

//-------------------------------------------------------------------------
void ListOfMetaFiles::ReadNative( CStream& oStream )
{
	UINT32 	nSize		= -1;
  	BYTE*	naBuffer	= NULL;

	Init( NULL );

	oStream >> nSize;
	naBuffer = new BYTE[ nSize ];
  	oStream.Read( naBuffer, nSize );

  	// Create the new metafile
  	m_pImage = new CImageMetafile();
	m_pImage->SetCompressedData( naBuffer, nSize );
}
//-------------------------------------------------------------------------
void ListOfMetaFiles::Read( CXMLReader& xml, hMETAFILE& nID )
{
	CString name;
	CString type;
	UINT 	nSize 		= 0;
	BYTE*	naBuffer 	= NULL;

	Init( NULL );

	xml.getAttribute( _T("id"), nID );
	xml.getAttribute( _T("type"), type );
	xml.intoTag();

	while (xml.nextTag( name ))
	{
		if (name == "UUENCODE")
		{
			xml.getChildDataUUdecode( naBuffer, nSize );
		}
	}

	xml.outofTag();

  	// Create the new metafile
	if (type == "PNG")
	{
		m_pImage = new CImagePNG();
	}
	else if (type == "JPEG")
	{
		m_pImage = new CImageJpeg();
	}
	else
	{
		// Default to metafile...
		m_pImage = new CImageMetafile();
	}
  	
	m_pImage->SetCompressedData( naBuffer, nSize );
}
//-------------------------------------------------------------------------
void ListOfMetaFiles::Write( CXMLWriter& xml ) const
{
	m_pImage->SaveXML( xml );
}
//--------------------------------------------------------------------------
void ListOfMetaFiles::SaveItemXML( CTinyCadDoc*, CXMLWriter& xml )
{
	xml.addAttribute( _T("type"), m_pImage->GetType() );
	Write( xml );
}
//--------------------------------------------------------------------------
void ListOfMetaFiles::LoadItemXML( CTinyCadDoc*, CXMLReader& xml )
{
	ListOfMetaFiles	oMeta;
	hRESOURCE		nID;

	oMeta.Read( xml, nID );

	Add( new ListOfMetaFiles(oMeta), nID );
}
//--------------------------------------------------------------------------
void ListOfMetaFiles::LoadItem(CTinyCadDoc*, CStream& oStream, hRESOURCE n )
{
	ListOfMetaFiles oMeta;

	oMeta.ReadNative( oStream );

	Add( new ListOfMetaFiles(oMeta), n );
}
//--------------------------------------------------------------------------

//=========================================================================
//== Comparison                                                          ==
//=========================================================================

//-------------------------------------------------------------------------
BOOL ListOfMetaFiles::Compare( CDocResource* o )
{
	#define  pMetaFile (((ListOfMetaFiles *)o)->m_pImage)

	return m_pImage == pMetaFile;
}
//-------------------------------------------------------------------------
