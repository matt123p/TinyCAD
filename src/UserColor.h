/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#pragma once

#include "XmlWriter.h"
#include "XmlReader.h"

//*************************************************************************
//*                                                                       *
//*                Encapsuled the user defined colors                     *
//*                                                                       *
//*************************************************************************

typedef std::map<int, CString> TNamesMap;
typedef TNamesMap::iterator TNamesMapIterator;

//=========================================================================
class CUserColor
{
private:
	typedef std::map<int, COLORREF> TColorRefMap;
	typedef TColorRefMap::iterator TColorRefMapIterator;

	static TNamesMap m_colNames;
	TColorRefMap m_colColorRef;

public:
	static const int WIRE;
	static const int BUS;
	static const int JUNCTION;
	static const int NOCONNECT;
	static const int LABEL;
	static const int POWER;
	static const int PIN;
	static const int HIDDEN_PIN;
	static const int BACKGROUND;

	//=====================================================================
	//== ctor/dtor/initializing                                          ==
	//=====================================================================
	CUserColor();
	CUserColor(CUserColor&);

	/** Set the default colours (ignoring the registry). */
	void Init();

	//=====================================================================
	//== Accessor                                                        ==
	//=====================================================================
	COLORREF Get(int n);

	/** Saves user colors in registry if default values have really changed.*/
	void WriteRegistry();

	//=====================================================================
	//== Mutator                                                         ==
	//=====================================================================
	/** Sets a certain user color. */
	void Set(int n, COLORREF cr);

	/** Set the default colours from the registry (if available). */
private:
	void ReadRegistry();

	//=====================================================================
	//== Serialization                                                   ==
	//=====================================================================
public:
	void ReadXML(CXMLReader& xml, CString sTagName);
	void WriteXML(CXMLWriter& xml);
};
//=========================================================================
