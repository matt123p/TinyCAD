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

// XMLReader.cpp: implementation of the CXMLReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <string>

#include "tinycad.h"
#include "XMLReader.h"
#include "XMLException.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXMLReader::CXMLReader(CStream* pInput)
{

	//TRACE("CXMLReader::CXMLReader():  Constructor.\n");


	m_pInput = pInput;
	m_uu_data = NULL;
	m_uu_size = 0;
	m_uu_state = 0;
	m_current_self_closing = false;
	m_charset_conv = CHARSET_INVALID;
	m_decoded_buffer = NULL;
	m_decoded_buf_size = 0;
	m_output_pos = 0;
	m_decoded_chars = 0;
	m_line_counter = 0;

#ifdef UNICODE
	SetCharset( _T("UTF-8") );
#endif

	intoTag();

	//TRACE("CXMLReader::CXMLReader(): Leaving CXMLReader() constructor.\n");

}

CXMLReader::~CXMLReader()
{

	//TRACE("CXMLReader::~CXMLReader():  Entering Destructor.\n");

	while (m_tags.size() > 0)
	{
		delete m_tags.front();
		m_tags.pop_front();
	}

	if (m_charset_conv != CHARSET_INVALID)
	{
		iconv_close(m_charset_conv);
	}

	delete m_decoded_buffer;

	//TRACE("CXMLReader::~CXMLReader():  Leaving Destructor\n");

}

// Set up the character set conversions
void CXMLReader::SetCharset(const TCHAR* fromcode)
{

	//TRACE("CXMLReader::SetCharset(): Entering - doing iconv stuff.\n");


	if (m_charset_conv != CHARSET_INVALID)
	{
		iconv_close(m_charset_conv);
	}

#ifdef UNICODE
	char fc[ 256 ];
	int l = WideCharToMultiByte( CP_ACP, 0, fromcode, _tcslen( fromcode), fc, static_cast<int> (sizeof( fc )), NULL, NULL );
	fc[ l ] = 0;
	m_charset_conv = iconv_open( "UCS-2-INTERNAL", fc );
#else
	m_charset_conv = iconv_open("CHAR", fromcode);

#endif

	//TRACE("CXMLReader::SetCharset():  Leaving.\n");
}

// Get the next character from the input stream
bool CXMLReader::getNextChar(xml_char_t &c)
{
	if (m_charset_conv != CHARSET_INVALID)
	{
		// Are there any bytes in the decoded buffer?
		if (m_output_pos >= m_decoded_chars)
		{
			// No bytes left, so must refetch
			m_output_pos = 0;

			// We read in a line at a time and decode it...
			std::string input;
			char in_c;
			do
			{
				if (m_pInput->Read(&in_c, 1) != 1)
				{
					// Eof
					break;
				}
				input += in_c;

				if (in_c == '\n')
				{
					//track the line number of the input XML file so that it can be used in error messages.
					m_line_counter++;
				}

			} while (in_c != '\r' && in_c != '\n');

			if (m_decoded_buf_size < input.size() * 4)
			{
				delete m_decoded_buffer;
				m_decoded_buf_size = input.size() * 4;
				m_decoded_buffer = new TCHAR[m_decoded_buf_size];
			}

			// Now perform the conversion
			size_t outbuf_size = m_decoded_buf_size * sizeof(TCHAR);
			size_t inbuf_size = input.size();
			TCHAR *out = m_decoded_buffer;
			const char *in = input.c_str();
			iconv(m_charset_conv, &in, &inbuf_size, (char **) &out, &outbuf_size);
			m_decoded_chars = (TCHAR*) out - m_decoded_buffer;
		}

		// Are there any bytes in the decoded buffer?
		if (m_output_pos >= m_decoded_chars)
		{
			// No bytes left, must be eof
			return true;
		}
		else
		{
			c = m_decoded_buffer[m_output_pos];
			++m_output_pos;
			return false;
		}
	}
	else
	{
		// No charset conversion available...
		return m_pInput->Read(&c, 1) != 1;
	}
}

xml_parse_tag* CXMLReader::get_current_tag()
{
	return m_tags.front();
}

// Get the attribute information associated with the current
// tag...
bool CXMLReader::internal_getAttribute(const xml_char_t *name, CString &data)
{
	xml_parse_tag::attrCollection::iterator it = get_current_tag()->m_attributes.find(name);
	if (it != get_current_tag()->m_attributes.end())
	{
		data = (*it).second;
		return true;
	}

	return false;
}

void CXMLReader::child_data(const xml_char_t *in)
{

	//TRACE("CXMLReader::child_data():  uudecode stuff\n");

	if (m_uu_data != NULL && m_uu_size > 0)
	{
		int l = (int) _tcslen(in);
		while (l > 0)
		{
			uudecode(in[0]);
			in++;
			l--;
		}
	}
	else
	{
		m_child_data += in;
	}
}

// Handle the current tag as a system tag
void CXMLReader::handleSystemTag()
{

	//TRACE("CXMLReader::handleSystemTag()\n");

	xml_parse_tag *tag = get_current_tag();

	// Is this the xml header tag?
	if (tag->m_tag_name == "?xml")
	{
		// Ok, get the encoding attribute if it is present...
		CString data;
		if (internal_getAttribute(_T("encoding"), data))
		{
			// Use this encoding
			SetCharset(data);
		}
	}
}

// Scan and find the next tag
bool CXMLReader::getNextTag(CString &name)
{

	//TRACE("CXMLReader::getNextTag()\n");

	if (m_current_self_closing)
	{
		return false;
	}

	// Is the sub-buffer for the child data
	const int build_len = 255;
	xml_char_t child_data_build[build_len + 1];
	int child_data_index = 0;

	// Now scan to find the next tag...
	xml_char_t c;
	for (;;)
	{
		do
		{
			if (getNextChar(c))
			{
				name = "";
				return false;
			}

			if (c != '<')
			{
				if (child_data_index == build_len || c == '&')
				{
					child_data_build[child_data_index] = 0;
					child_data(child_data_build);
					child_data_index = 0;
				}

				if (c == '&')
				{
					child_data(xml_parse_tag::read_entity(this));
				}
				else
				{
					child_data_build[child_data_index] = c;
					child_data_index++;
				}
			}
		} while (c != '<');

		// Now we have found the opening for the next
		// tag, we must scan it...
		get_current_tag()->parse(this);

		// Was this a comment?
		if (get_current_tag()->m_comment)
		{
			continue;
		}

		// Was this a system tag?
		if (get_current_tag()->isSystemTag())
		{
			// Handle it...
			handleSystemTag();
			continue;
		}

		// Was this a closing tag?
		if (get_current_tag()->m_closing_tag)
		{
			// Yep, so this is the end of this run....
			name = get_current_tag()->m_tag_name;

			child_data_build[child_data_index] = 0;
			child_data(child_data_build);

			return false;
		}
		else
		{
			// Ok, we have a new opening tag...
			name = get_current_tag()->m_tag_name;

			child_data_build[child_data_index] = 0;
			child_data(child_data_build);

			return true;
		}
	}
}

// Scan until the end of the current tag...
//
bool CXMLReader::closeTag()
{

	//TRACE("CXMLReader::closeTag()\n");

	if (m_current_self_closing)
	{
		return true;
	}

	// Is this tag self-closing?
	if (get_current_tag()->m_self_closing_tag || get_current_tag()->m_closing_tag)
	{
		return true;
	}

	CString close_name = get_current_tag()->m_tag_name;

	// Nope, we must scan until we find the closing for
	// this tag ( recursively going into tags and out
	// again until we find our match...)
	for (;;)
	{
		CString name;

		// Is this the closing tag for the current tag?
		if (!getNextTag(name))
		{
			if (close_name == name)
			{
				return true;
			}
			else
			{
				CString diagnosticMessage;
				diagnosticMessage.Format(_T("Error:  XMLReader line #378:  ERR_XML_WRONG_CLOSE:  Expecting tag [%s], but found tag [%s].  Current line number = %d.\n"), name, close_name, m_line_counter);
				TRACE(diagnosticMessage);
				throw new CXMLException(ERR_XML_WRONG_CLOSE, m_line_counter, TRUE);
			}
		}
		else
		{
			// Must be an opening tag, so we need to close this
			// tag now...
			intoTag();
			outofTag();
		}
	}

	//return true;
}

// Find the next peer tag of this tag
// returns false if there are no more tags at this
// level...
bool CXMLReader::nextTag(CString &name)
{

	//TRACE("CXMLReader::nextTag()\n");

	// First, we must find the closing of the 
	// previous tag
	if (!closeTag())
	{
		return false;
	}

	m_child_data = "";
	return getNextTag(name);
}

// Move inside the current tag.  nextTag will then return
// tags that are inside the current tag....
void CXMLReader::intoTag()
{

	//TRACE("CXMLReader::intoTag():  Entering.\n");

	// Is this tag self-closing?
	if (m_tags.size() > 0 && get_current_tag()->m_self_closing_tag)
	{
		m_current_self_closing = true;
	}
	else
	{
		// Create a new parser to go into the tag system...
		xml_parse_tag *tag = new xml_parse_tag();
		m_tags.push_front(tag);
	}

	//TRACE("CXMLReader::intoTag():  Leaving.\n");


}

// Move up one level.  nextTag will then return tags
// that are one above the current level...
//
void CXMLReader::outofTag()
{

	//TRACE("CXMLReader::outofTag()\n");

	if (m_current_self_closing)
	{
		m_current_self_closing = false;
		return;
	}

	// Have we already scanned to the closing tag?
	if (get_current_tag()->m_closing_tag && m_tags.size() > 1)
	{
		CString check_name = get_current_tag()->m_tag_name;

		// Copy this tag up one level...
		xml_parse_tag *ptag = m_tags.front();
		m_tags.pop_front();

		delete m_tags.front();
		m_tags.pop_front();

		m_tags.push_front(ptag);

		if (get_current_tag()->m_tag_name != check_name)
		{
			CString diagnosticMessage;
			diagnosticMessage.Format(_T("Error:  XMLReader line #475:  ERR_XML_WRONG_CLOSE:  Expecting tag [%s], but found tag [%s].  Current line number = %d.\n"), check_name, get_current_tag()->m_tag_name, m_line_counter);
			TRACE(diagnosticMessage);

			throw new CXMLException(ERR_XML_WRONG_CLOSE, m_line_counter, TRUE);
		}
	}
	else
	{
		delete m_tags.front();
		m_tags.pop_front();

		closeTag();
	}

}

// Get the next child data associated with the current tag
// 
CString CXMLReader::internal_getChildData()
{

	//TRACE("CXMLReader::internal_getChildData()\n");

	m_child_data = "";

	closeTag();

	return m_child_data;
}

void CXMLReader::getChildDataUUdecode(BYTE* &data, UINT &size)
{

	//TRACE("CXMLReader::getChildDataUUdecode()\n");

	// Get the size of this data
	CString name;

	getAttribute(_T("size"), size);
	data = new BYTE[size];

	m_uu_size = size;
	m_uu_data = data;

	closeTag();

	m_uu_data = NULL;
	m_uu_size = 0;
	m_uu_state = 0;
}

// single character decode 
#define DEC(c)	(((c) - ' ') & 077)

// UUdecode
void CXMLReader::uudecode(xml_char_t in)
{

	//TRACE("CXMLReader::uudecode()\n");

	switch (m_uu_state)
	{
		case 0: // We are awaiting the line count
			if (xml_parse_tag::is_whitespace(in))
			{
				break;
			}
			// Not whitespace, so must be the line count...
			m_uu_line_size = DEC(in);
			m_uu_state++;
			break;
		case 1: // We are awating the 1st char in the four char set
			m_uu_bytes[0] = static_cast<char> (in);
			m_uu_state++;
			break;
		case 2: // We are awating the 2nd char in the four char set
			m_uu_bytes[1] = static_cast<char> (in);
			m_uu_state++;
			break;
		case 3: // We are awating the 3rd char in the four char set
			m_uu_bytes[2] = static_cast<char> (in);
			m_uu_state++;
			break;
		case 4: // We are awating the 4th char in the four char set
			m_uu_bytes[3] = static_cast<char> (in);
			int c[3];
			c[0] = DEC(m_uu_bytes[0]) << 2 | DEC(m_uu_bytes[1]) >> 4;
			c[1] = DEC(m_uu_bytes[1]) << 4 | DEC(m_uu_bytes[2]) >> 2;
			c[2] = DEC(m_uu_bytes[2]) << 6 | DEC(m_uu_bytes[3]);

			for (int i = 0; i < 3; i++)
			{
				if (m_uu_line_size > 0 && m_uu_size > 0)
				{
					*m_uu_data = (BYTE) c[i];
					m_uu_line_size--;
					m_uu_size--;
					m_uu_data++;
				}
			}
			if (m_uu_line_size == 0)
			{
				m_uu_state = 0;
			}
			else
			{
				m_uu_state = 1;
			}
			break;
	}

}

// Type conversions....
void CXMLReader::unmakeString(CString str, CString &data)
{
	data = str;
}

void CXMLReader::unmakeString(CString str, int &data)
{
	data = _tstoi(str);
}

void CXMLReader::unmakeString(CString str, UINT &data)
{
	_stscanf_s(str, _T("%u"), &data);
}

void CXMLReader::unmakeString(CString str, COLORREF &data)
{
	xml_char_t *dummy;
	data = _tcstol(str, &dummy, 16);
}

void CXMLReader::unmakeString(CString str, double &data)
{
	data = _tstof(str);
}

void CXMLReader::unmakeString(CString str, CDPoint &data)
{
	_stscanf_s(str, _T("%lg,%lg"), &data.x, &data.y);
	data.x = data.unmakeXMLUnits(data.x);
	data.y = data.unmakeXMLUnits(data.y);
}

void CXMLReader::unmakeString(CString str, BYTE &data)
{
	int d;
	unmakeString(str, d);
	data = (BYTE) d;
}

void CXMLReader::unmakeString(CString str, unsigned short &data)
{
	int d;
	unmakeString(str, d);
	data = (unsigned short) d;
}

void CXMLReader::unmakeString(CString str, short &data)
{
	int d;
	unmakeString(str, d);
	data = (short) d;
}

void CXMLReader::unmakeString(CString str, long &data)
{
	int d;
	unmakeString(str, d);
	data = d;
}

void CXMLReader::unmakeString(CString str, bool &data)
{
	int d;
	unmakeString(str, d);
	data = d != 0;
}

int CXMLReader::get_line_counter()
{
	return m_line_counter;
}
