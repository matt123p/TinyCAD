/*
 TinyCAD program for schematic capture
 Copyright 1994-2004 Matt Pyne.

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

// XMLWriter.h: interface for the CXMLWriter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLWRITER_H__27829795_1DC3_409D_BC9E_D24D676A812D__INCLUDED_)
#define AFX_XMLWRITER_H__27829795_1DC3_409D_BC9E_D24D676A812D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "DPoint.h"
#include "Stream.h"
#include <deque>

#include "iconv.h"

typedef TCHAR xml_char_t;

class CXMLWriter
{
	CString makeString(const TCHAR *data);
	CString makeString(int data);
	CString makeString(COLORREF data);
	CString makeString(double data);
	CString makeString(CDPoint data);
	CString makeString(UINT data);

	void internal_addChildData(const xml_char_t *data);
	void internal_addAttribute(const xml_char_t *name, const xml_char_t *value);
	void internal_addTag(const xml_char_t *tag_name, const xml_char_t *data);

	// Set up the character set conversions
	void SetCharset(const char* tocode, const char* fromcode);

	// Close an open tag but not completely..
	void closeOpenTag();

	// Send a completed string to the output
	void SendString(const xml_char_t *str);

	// Did we add any child data?
	bool m_child_data;

	// A stack of the current open tags
	typedef std::deque<CString> tagCollection;
	tagCollection m_tags;

	// The output stream we are using
	CStream* m_pOutput;

	// The libiconv conversion handling variables
	char* 		m_conversion_buffer;
	size_t		m_conversion_buf_size;
	iconv_t		m_charset_conv;

	// Make the XML output pretty
	int m_nesting_level;
	bool m_needs_newline;
	bool m_needs_tabs;

	// Keep an output line counter to help with error messages
	int m_line_counter;

public:
	void closeTag();
	void addTag(const xml_char_t *tag_name);
	void addComment(const xml_char_t *comment);

	void addChildDataUUencode(BYTE *data, size_t size);

	int get_line_counter();

	template<class T> void addTag(const xml_char_t *tag_name, T t)
	{
		internal_addTag(tag_name, makeString(t));
	}

	template<class T> void addAttribute(const xml_char_t *name, T t)
	{
		internal_addAttribute(name, makeString(t));
	}

	template<class T> void addChildData(T t)
	{
		internal_addChildData(makeString(t));
	}

	CXMLWriter(CStream* pOutput);
	virtual ~CXMLWriter();

};

#endif // !defined(AFX_XMLWRITER_H__27829795_1DC3_409D_BC9E_D24D676A812D__INCLUDED_)
