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

typedef Int16 hFONT;

//*************************************************************************
//*                                                                       *
//*                Storage of font settings                               *
//*                                                                       *
//*************************************************************************

//=========================================================================
class ListOfFonts: public CDocResource
{
	typedef CDocResource super;

public:
	static const CString DEFAULT_FONT;

	//-- The sizes of fonts
	//-- font height for normal text
public:
	static const Int32 TEXT_HEIGHT;

	//-- font height for ruler text
public:
	static const Int32 RULE_HEIGHT;

	//-- font height for pin text
public:
	static const Int32 PIN_HEIGHT;

	// The Font this describes
private:
	LOGFONT m_oFont;

	//=====================================================================
	//== ctor/dtor/initializing                                          ==
	//=====================================================================
public:
	ListOfFonts();
public:
	ListOfFonts(const ListOfFonts&);
public:
	ListOfFonts(LOGFONT f);
public:
	ListOfFonts(Int32 nHeight, bool bIsItalic);
public:
	virtual ~ListOfFonts();
public:
	void Init();
public:
	void Init(LOGFONT);

	//=====================================================================
	//== accessor/mutator                                                ==
	//=====================================================================
	//-- Get the data associated with this resource
public:
	LOGFONT GetFont() const;
public:
	LOGFONT& GetFontRef();
public:
	LOGFONT* GetFontPtr();
public:
	static LOGFONT GetDefaultFont();

	//=====================================================================
	//== serialization                                                   ==
	//=====================================================================
public:
	virtual void ReadNative(CStream& oStream);
public:
	virtual void Read(CXMLReader& xml, hFONT& nID);
public:
	virtual void Write(CXMLWriter& xml) const;

public:
	virtual void LoadItem(CTinyCadDoc* pDesign, CStream&, hRESOURCE);
public:
	virtual void SaveItemXML(CTinyCadDoc *pDesign, CXMLWriter& xml);
public:
	virtual void LoadItemXML(CTinyCadDoc *pDesign, CXMLReader& xml);

	//=====================================================================
	//== Comparison                                                      ==
	//=====================================================================
	//-- Compare this resource with another resource of the same type
public:
	virtual BOOL Compare(CDocResource*);
};
//=========================================================================
