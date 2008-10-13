/*
	TinyCAD program for schematic capture
	Copyright 1994/1995/2002-2005 Matt Pyne.

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

// xml_parse_tag.h: interface for the xml_parse_tag class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XML_PARSE_TAG_H__5E944B5F_ACFF_46E1_947C_76A5D38FDB0F__INCLUDED_)
#define AFX_XML_PARSE_TAG_H__5E944B5F_ACFF_46E1_947C_76A5D38FDB0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Stream.h"

#define ERR_XML_EXPECTING_NAME		_T("XML parse error: Expecting tag name")
#define ERR_XML_EXPECTING_ATTR		_T("XML parse error: Expecting attribute name or end of tag")
#define ERR_XML_EXPECTING_EQUALS	_T("XML parse error: Expecting =")
#define ERR_XML_EXPECTING_VALUE		_T("XML parse error: Expecting attribute value")
#define ERR_XML_EXPECTING_END		_T("XML parse error: Expecting end of XML tag")
#define ERR_XML_WRONG_CLOSE			_T("XML parse error: The closing XML tag does not match the opening tag")
#define ERR_XML_HAVE_CHILD_DATA		_T("XML output error: Cannot add attributes after child data")
#define ERR_XML_NO_TAG				_T("XML output error: There is no open tag")

class CXMLReader;

class xml_parse_tag  
{
private:

	// Here are the states of the XML parser
	typedef enum 
	{
		waitfor_close_or_name,			// Get either a / or the tag name
		waitfor_name,
		reading_name,
		waitfor_attr_name,
		reading_attribute_name,
		waitfor_attr_equals,
		waitfor_attr_value,
		reading_attribute_value,
		reading_attribute_value_q1,		// Enclosed in "s
		reading_attribute_value_q2,		// Enclosed in 's
		waitfor_end,					// after /
		waitfor_comment1,				// after !
		waitfor_comment2,				// after !-
		reading_comment,				// after !--
		waitfor_end_comment1,			// after -
		waitfor_end_comment2,			// after --
	} xml_state;

	CString	m_working_string;
	CString m_working_attr;
	xml_state	m_state;

	void out_of_sequence();
	void finished_name();


public:
	xml_parse_tag();
	virtual ~xml_parse_tag();


	bool			m_closing_tag;
	bool			m_self_closing_tag;
	bool			m_comment;
	TCHAR			m_enclosing_char;

	// Here are the attributes associated with the current tag...
	typedef std::map<CString,CString> attrCollection;
	attrCollection	m_attributes;

	// Here is the name of the current tag
	CString	m_tag_name;

	// Fetch an entity from the stream
	static CString read_entity(CXMLReader *s);

	// Is this character an XML whitespace?
	static bool is_whitespace( TCHAR c )
	{
		return c == ' ' || c == '\t' || c == '\n' || c == '\r';
	}

	bool isSystemTag() { return m_enclosing_char != '/'; }

	// Parse the incoming XML and get the next tag...
	bool parse(CXMLReader *s);
};

#endif // !defined(AFX_XML_PARSE_TAG_H__5E944B5F_ACFF_46E1_947C_76A5D38FDB0F__INCLUDED_)
