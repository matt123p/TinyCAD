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

// XMLReader.h: interface for the CXMLReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLREADER_H__B212A6F7_2322_40FE_9023_639BA4F2F5C9__INCLUDED_)
#define AFX_XMLREADER_H__B212A6F7_2322_40FE_9023_639BA4F2F5C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "xml_parse_tag.h"
#include "NewTypes.h"
#include "DPoint.h"

#include "iconv.h"

typedef TCHAR xml_char_t;

class CXMLReader
{
private:

	bool internal_getAttribute(const xml_char_t *name, CString &data);
	CString internal_getChildData();

	// UUdecode
	BYTE *m_uu_data;
	UINT m_uu_size;
	int m_uu_state;
	int m_uu_line_size;
	char m_uu_bytes[4];
	void uudecode(xml_char_t in);

	// Build the child data
	CString m_child_data;
	void child_data(const xml_char_t *in);

	// The libiconv conversion handling variables
	iconv_t m_charset_conv;
	TCHAR* m_decoded_buffer;
	size_t m_decoded_buf_size;
	size_t m_decoded_chars;
	size_t m_output_pos;

	// Keep an input line counter to help with error messages
	int m_line_counter;

	// Type conversions....
	void unmakeString(CString str, CString &data);
	void unmakeString(CString str, int &data);
	void unmakeString(CString str, UINT &data);
	void unmakeString(CString str, long &data);
	void unmakeString(CString str, unsigned short &data);
	void unmakeString(CString str, short &data);
	void unmakeString(CString str, BYTE &data);
	void unmakeString(CString str, COLORREF &data);
	void unmakeString(CString str, double &data);
	void unmakeString(CString str, CDPoint &data);
	void unmakeString(CString str, bool &data);

	// Scan and find the next tag
	bool getNextTag(CString &name);

	// Find the closing of the current tag
	bool closeTag();

	// Handle the current tag as a system tag
	void handleSystemTag();

	// Here is the stack of tags we have gone into
	typedef std::deque<xml_parse_tag*> tagCollection;
	tagCollection m_tags;

	xml_parse_tag* get_current_tag();

	bool m_current_self_closing;

	// The input stream we are using
	CStream* m_pInput;

	// Set up the charaset conversions
	void SetCharset(const TCHAR* fromcode);

public:
	CXMLReader(CStream* pInput);
	virtual ~CXMLReader();

	// Find the next peer tag of this tag
	// returns false if there are no more tags at this
	// level...
	bool nextTag(CString &name);

	// Move inside the current tag.  nextTag will then return
	// tags that are inside the current tag....
	void intoTag();

	// Move up one level.  nextTag will then return tags
	// that are one above the current level...
	void outofTag();

	// Get the next character from the input stream
	bool getNextChar(xml_char_t &c);

	// Get the line counter
	int get_line_counter();

	// Get the attribute information associated with the current
	// tag...
	template<class T> bool getAttribute(const xml_char_t *name, T &t)
	{
		CString r;
		if (internal_getAttribute(name, r))
		{
			unmakeString(r, t);
			return true;
		}
		else
		{
			return false;
		}
	}

	// Get the next child data associated with the current tag
	// 
	template<class T> void getChildData(T &t)
	{
		CString r = internal_getChildData();
		unmakeString(r, t);
	}

	// Read the child data from a UUencoded source
	void getChildDataUUdecode(BYTE* &data, UINT &size);

};

#endif // !defined(AFX_XMLREADER_H__B212A6F7_2322_40FE_9023_639BA4F2F5C9__INCLUDED_)
