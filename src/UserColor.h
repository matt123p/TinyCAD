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

typedef std::map<int, CString> 		TNamesMap;
typedef TNamesMap::iterator 		TNamesMapIterator;

//=========================================================================
class CUserColor
{
	private:	typedef std::map<int, COLORREF> 	TColorRefMap;
	private:	typedef TColorRefMap::iterator 		TColorRefMapIterator;

	private:	static 	TNamesMap 			m_colNames;
	private:			TColorRefMap 		m_colColorRef;

	public:		static const	int	WIRE;
	public:		static const	int	BUS;
	public:		static const	int	JUNCTION;
	public:		static const	int	NOCONNECT;
	public:		static const	int	LABEL;
	public:		static const	int	POWER;
	public:		static const	int	PIN;
	public:		static const	int	HIDDEN_PIN;
	public:		static const	int	BACKGROUND;

	//=====================================================================
	//== ctor/dtor/initializing                                          ==
	//=====================================================================
	public:							CUserColor();
	public:							CUserColor( CUserColor& );

	/** Set the default colours (ignoring the registry). */
	public:			void			Init();

	//=====================================================================
	//== Accessor                                                        ==
	//=====================================================================
	public:			COLORREF		Get( int n );

	/** Saves user colors in registry if default values have really changed.*/
	public:			void			WriteRegistry();

	//=====================================================================
	//== Mutator                                                         ==
	//=====================================================================
	/** Sets a certain user color. */
	public:			void 			Set( int n, COLORREF cr );

	/** Set the default colours from the registry (if available). */
	private:		void			ReadRegistry();

	//=====================================================================
	//== Serialization                                                   ==
	//=====================================================================
	public:			void			ReadXML( CXMLReader& xml, CString sTagName );
	public:			void			WriteXML( CXMLWriter& xml );
};
//=========================================================================