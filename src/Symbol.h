/*
	TinyCAD program for schematic capture
	Copyright 1994/1995/2002 Matt Pyne.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include <vector>

#include "DrawingObject.h"
#include "net.h"

class CLibraryStore;
class CTinyCadSymbolDoc;
class CTinyCadMultiSymbolDoc;

//enum SymbolFieldType	//Values used for 2.60.00 and earlier
//{
//	default_show,
//	default_hidden,
//	always_hidden,
//	extra_parameter
//};

//enum SymbolFieldType	//Values used for 2.70.00 Alpha and Beta 1
//{
//    default_show,
//    default_hidden,
//    always_hidden,
//    default_show_name_and_value,
//    default_show_name_and_value_only_if_value_not_empty,
//    extra_parameter,
//};

enum SymbolFieldType	//Values used for 2.70.00 Beta 2 and later
/** Note:  The values assigned to this enum are written into the schematic and symbol design files.
 *  These values must NOT be changed, or a database revision will have to be implemented!
 */
{
    default_show = 0,
    default_hidden = 1,
    always_hidden = 2,
    extra_parameter = 3,	//Used to indicate a parameter that was added by the user to the schematic, not in the symbol editor
    default_show_name_and_value_only_if_value_not_empty = 4,
    default_show_name_and_value = 5,
	default_show_value_only_if_value_not_empty = 6,
	last_symbol_field_type	//This value is always last, and is never written to a design file so that its value may safely change from release to release
							//It is used to determine how many enums have been defined (sequential values assumed).
};

class CSymbolField
{
public:
	CString		field_name;
	CString		field_value;	// Not used!
	CString		field_default;
	CDPoint		field_pos;		// Not used!
	

	SymbolFieldType	field_type;

	// Save this field
	void Save(CStream &);							// Save this symbol field into an archive
	void SaveXML(CXMLWriter &xml, bool simple);		// Save this symbol field into xml
	void Load(CStream &);							// Load this symbol field from an archive
	void LoadXML(CXMLReader &xml);					// Load this symbol field from xml
};

class CDesignFileSymbol;

// Here is the data that is associated with
// all symbols
class CSymbolRecord
{

public:
	// The constructor
	CSymbolRecord();

	CString			name;			// The name of this symbol
	CString			reference;		// The default reference
	CString			description;	// The description of this symbol
	SymbolFieldType	name_type;		// Do we show the name by default?
	SymbolFieldType	ref_type;		// Do we show the reference by default?
	DWORD			NameID;			// The ID of this name when stored in a database

	void SaveXML(CXMLWriter &xml);	// Save this symbol into an archive
	void LoadXML(CXMLReader &xml);	// Load this symbol from an archive

	// Here are the other fields we can have
	std::vector<CSymbolField>	fields;
	BOOL						fields_loaded;
};

class CDesignFileSymbolFilter
{
public:
	bool	m_include_power_pins;
	int		m_part;
	CDPoint	m_reference_point;
	CDPoint	m_top_right;

	CDesignFileSymbolFilter( int part = 0, bool include_power_pins = false )
	{
		m_part = part;
		m_include_power_pins = include_power_pins;
	}

	bool operator<( const CDesignFileSymbolFilter &b ) const
	{
		if (m_part == b.m_part)
		{
			return m_include_power_pins < b.m_include_power_pins;
		}
		else
		{
			return m_part < b.m_part;
		}
	}
};


class CDesignFileSymbol : public CSymbolRecord
{
private:
	// Here we manipulate the main symbol to generate the different types of symbols
	// from this one
	void CreateSymbol(CTinyCadDoc *pDesign, drawingCollection &drawing, const CDesignFileSymbolFilter& filter );

	// To speed things up, here is a cache of created symbols
	typedef std::map<CDesignFileSymbolFilter,drawingCollection> FilteredSymbolCollection;
	FilteredSymbolCollection	m_filter_cache;

	CDPoint GetTr( CTinyCadDoc *pDesign, drawingCollection &drawing );

public:
	typedef std::vector<CNetListNode>	nodeCollection;

	CDesignFileSymbol();
	virtual ~CDesignFileSymbol();		// The Destructor
	
	DWORD			FilePos;			// The position in the file or database of this symbol
	BYTE 			ppp;				// Parts per package

	void SaveXML(CXMLWriter &xml);					// Save this symbol into an archive
	void LoadXML(CTinyCadDoc *, CXMLReader &xml);	// Load this symbol from an archive
	void Load(CTinyCadDoc *, CStream &);			// Load this symbol from an archive
	BOOL LoadSymbol( CTinyCadDoc *, CStream &);		// Load just the symbol, (no details) from the stream

	// Return the methods object
	BOOL GetMethod( int part, bool include_power_pins, drawingCollection &drawing );
	CDPoint GetTr( int part, bool include_power_pins );

	// What type of symbol is this?
	bool IsHeterogeneous() { return m_heterogeneous; }

	// Create default blank symbol
	void CreateNoSymbol( CTinyCadDoc *pDesign );

	CTinyCadDoc* GetDesign() { return m_pDesign; }

private:
	CTinyCadDoc*	  m_pDesign;

	bool m_heterogeneous;
	typedef std::vector<drawingCollection>	symbolCollection;
	symbolCollection m_methods;
};


class CLibraryStoreNameSet;

class CLibraryStoreSymbol : public CSymbolRecord
{
public:
	CLibraryStoreNameSet*	m_pParent;
	int						m_index;

	CLibraryStoreSymbol();
	~CLibraryStoreSymbol();

	CDesignFileSymbol *GetDesignSymbol( CTinyCadDoc *pDesign );
    bool IsMatching(const TCHAR * theString) const;
};

typedef std::vector<CLibraryStoreSymbol>	recordCollection;


class CLibraryStoreNameSet
{
private:

	recordCollection	m_records;

public:

	CLibraryStore *lib;		// The library that this symbol comes from

	CLibraryStoreNameSet(CLibraryStore *NewLib = NULL);	// The constructor
	virtual ~CLibraryStoreNameSet();		// The Destructor

	CLibraryStoreSymbol&	GetRecord( int index );
	int						GetNumRecords();
	void					SetRecords( recordCollection& records );
	void					PushBackRecord( CLibraryStoreSymbol &r );

	BYTE 			ppp;			// Parts per package
	int				orientation;	// The default orientation of the symbol
	DWORD			FilePos;		// The position in the file or database of this symbol

	// Make a blank symbol record
	void			Blank();

	// Save this NameSet as an XML export
	void SaveXML( CXMLWriter &xml );

	// Load this NameSet from an XML file
	void LoadXML(CTinyCadMultiSymbolDoc *pDesign, CXMLReader &xml);
	
	void Save(CStream &);		// Save this symbol into an archive
	void Load(CStream &);		// Load this symbol from an archive
	void OldLoad1(CStream &);	// Load the old versions of symbol
	void OldLoad2(CStream &);	// Load the old versions of symbol
	void OldLoad3(CStream &);	// Load the old versions of symbol

	// Return the methods object
	CString GetLibName();

	CDesignFileSymbol *GetDesignSymbol( CTinyCadDoc *pDesign, int index );
	CStream *GetMethodArchive();

};




#endif


