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

// xml_parse_tag.cpp: implementation of the xml_parse_tag class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xml_parse_tag.h"
#include "XMLReader.h"
#include "XMLException.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

xml_parse_tag::xml_parse_tag()
{
	m_closing_tag = false;
	m_self_closing_tag = true;
}

xml_parse_tag::~xml_parse_tag()
{

}


/////////////////////////////////////////////////////////////////////////////
// Here are the xml tag functions
//

void xml_parse_tag::out_of_sequence()
{
	const xml_char_t *e;

	switch (m_state)
	{
	case waitfor_name:
	case waitfor_close_or_name:
	case reading_name:
		e = ERR_XML_EXPECTING_NAME;
		break;
	case waitfor_attr_name:
	case reading_attribute_name:
		e = ERR_XML_EXPECTING_ATTR;
		break;
	case waitfor_attr_equals:
		e = ERR_XML_EXPECTING_EQUALS;
		break;
	case waitfor_attr_value:
	case reading_attribute_value:
	case reading_attribute_value_q1:
	case reading_attribute_value_q2:
		e = ERR_XML_EXPECTING_VALUE;
		break;
	case waitfor_end:
	default:
		e = ERR_XML_EXPECTING_END;
		break;
	}

	// Raise the exception
	throw new CXMLException(e, TRUE);
}

void xml_parse_tag::finished_name( )
{
	switch (m_state)
	{
	case reading_name:
		// TRACE("got xml tag name = '%s'\n", m_working_string );
		m_state = waitfor_attr_name;
		m_tag_name = m_working_string;
		break;
	case reading_attribute_name:
		m_working_attr = m_working_string;
		m_state = waitfor_attr_equals;
		break;
	case reading_attribute_value:
	case reading_attribute_value_q1:
	case reading_attribute_value_q2:
		// TRACE("%s = %s\n", m_working_attr, m_working_string );
		m_attributes[ m_working_attr ] = m_working_string;
		m_state = waitfor_attr_name;
		break;
	}
}


// Read from the input stream, which has been determined to
// have the start of an XML tag in it.  We construct the XML
// objects from the stream...
//
// On entry the < character has been read from the stream...
bool xml_parse_tag::parse(CXMLReader *s) 
{
	// We are in the tag so read in the tag name,
	// it's attributes and find the closing >

	m_state = waitfor_close_or_name;
	m_closing_tag = false;
	m_working_string = "";
	m_working_attr = "";
	m_comment = false;

	m_attributes.erase( m_attributes.begin(), m_attributes.end() );

	m_self_closing_tag = false;
	m_enclosing_char = '/';


	// We allow whitespace before the tag name...
	bool end = false;
	while (!end)
	{
		xml_char_t c;
		if (s->getNextChar(c) )
		{
			break;
		}

		// Do something special is we are reading in-between quotes...
		if (m_state == reading_attribute_value_q1 
		 || m_state == reading_attribute_value_q2)
		{
			if (m_state == reading_attribute_value_q1 && c == '"'
			 || m_state == reading_attribute_value_q2 && c == '\'')
			{
				// end of the value...
				finished_name();
				m_working_string = "";
			}
			else
			{
				if (c == '&')
				{
					// The start of an entity
					m_working_string += read_entity(s);
				}
				else
				{
					m_working_string += c;
				}
			}
		}
		else if (m_state == reading_comment
			&& c != '-')
		{
			// Ignore comments...
		}
		else
		{
			if (is_whitespace(c))
			{
				switch (m_state)
				{
				case reading_name:
				case reading_attribute_name:
				case reading_attribute_value:
					finished_name();
					m_working_string = "";
					break;
				}

			}
			else if (c == '>')
			{
				if (m_state == waitfor_end_comment2)
				{
					m_comment = true;
					end = true;
					m_closing_tag = true;
				}
				else
				{
					// This is the closing tag
					if (m_state != waitfor_end 
						&& m_state != waitfor_attr_name 
						&& m_state != reading_attribute_value
						&& m_state != reading_name)
					{
						out_of_sequence();
					}
					finished_name();
				}
				end = true;
			}
			else if (c == m_enclosing_char)
			{
				if ( m_state == waitfor_close_or_name)
				{
					m_state = waitfor_name;
					m_closing_tag = true;
				}
				else
				{
					// This is the end-of-tag signal
					if (   m_state != waitfor_attr_name
						&& m_state != reading_attribute_value
						&& m_state != reading_name)
					{
						out_of_sequence();
					}
					finished_name();
					m_self_closing_tag = true;
					m_state = waitfor_end;
				}
			}
			else if (c == '=')
			{
				// This is an attribute definition
				if (m_state != waitfor_attr_equals 
			     && m_state != reading_attribute_name)
				{
					out_of_sequence();
				}
				finished_name();
				m_working_string = "";
				m_state = waitfor_attr_value;
			}
			else if (c == '\'')
			{
				if (m_state != waitfor_attr_value)
				{
					out_of_sequence();
				}
				m_working_string = "";
				m_state = reading_attribute_value_q2;
			}
			else if (c == '"')
			{
				if (m_state != waitfor_attr_value)
				{
					out_of_sequence();
				}
				m_working_string = "";
				m_state = reading_attribute_value_q1;
			}
			else
			{
				bool set_working_string = false;
				bool add_to_working_string = false;

				switch (m_state)
				{
				case waitfor_close_or_name:
				case waitfor_name:
					set_working_string = true;
					if (c == '!')
					{
						m_state = waitfor_comment1;
					}
					else
					{
						if (c =='?')
						{
							m_enclosing_char = c;
						}

						m_state = reading_name;
					}
					break;

				case waitfor_attr_name:
					set_working_string = true;
					m_state = reading_attribute_name;
					break;

				case waitfor_attr_value:
					set_working_string = true;
					m_state = reading_attribute_value;
					break;

				case reading_attribute_value_q1:
				case reading_attribute_value_q2:
				case reading_name:
				case reading_attribute_name:
				case reading_attribute_value:
					add_to_working_string = true;
					break;

				case waitfor_comment1:
				case waitfor_comment2:
					add_to_working_string = true;
					if (c != '-')
					{
						m_state = waitfor_name;

					}
					else
					{
						switch (m_state)
						{
						case waitfor_comment1:
							m_state = waitfor_comment2;
							break;
						case waitfor_comment2:
							m_state = reading_comment;
							break;
						}
					}
					break;

				case reading_comment:
					// Ignore all values from now on...
					if (c == '-')
					{
						m_state = waitfor_end_comment1;
					}
					break;

				case waitfor_end_comment1:
					if (c == '-')
					{
						m_state = waitfor_end_comment2;
					}
					else
					{
						m_state = reading_comment;
					}
					break;

				case waitfor_end_comment2:
					if (c != '-')
					{
						m_state = reading_comment;
					}
					break;

				case waitfor_end:
				case waitfor_attr_equals:
				default:
					out_of_sequence();
					break;
				}

				if (c == '&')
				{
					// The start of an entity
					if (m_state != reading_attribute_value)
					{
						out_of_sequence();
					}

					if (set_working_string)
					{
						m_working_string = read_entity(s);
					}
					else if (add_to_working_string)
					{
						m_working_string += read_entity(s);
					}
				}
				else
				{
					if (set_working_string)
					{
						m_working_string = c;
					}
					else if (add_to_working_string)
					{
						m_working_string += c;
					}
				}

			}
		}
	}


	// That's it! we have completed the tag..
	return m_self_closing_tag;
}



/////////////////////////////////////////////////////////////////////////////
//
// Here we look up an XML entity from the parser's entity
// dictionary
//

CString xml_parse_tag::read_entity(CXMLReader *s)
{
	const xml_char_t *r = _T("?");

	// read until the end of the string and determine which entity it is...
	CString q;
	for (;;)
	{
		xml_char_t c;
		if (s->getNextChar(c) )
		{
			break;
		}

		if (c == ';')
		{
			break;
		}
		else if ( !((c >= 'a' && c <= 'z') 
			     || ( c >= 'A' && c <= 'Z' ) 
				 || ( c >= '0' && c <= '9' )
				 || ( c == '#')) )
		{
			// Invalid character..
			return r;
		}
		else
		{
			q += c;
		}

		if (q.GetLength() > 32)
		{
			// Too long....
			return r;
		}
	}

	// Do we look up this entity or is it a character reference?
	if (q.GetAt(0) == '#')
	{
		xml_char_t c[2];
		c[1] = 0;

		// Numeric reference
		if (q.GetAt(1) == 'x')
		{
			xml_char_t *endptr;
			c[0] = static_cast<xml_char_t>(_tcstol( static_cast<const xml_char_t *>(q) + 2, &endptr, 16 ));
		}
		else
		{
			c[0] = _tstoi( static_cast<const xml_char_t *>(q) + 1 );
		}
		return c;
	}
	else
	{
		// What is this entity?
		if (q == _T("lt"))
		{
			return _T("<");
		}
		else if (q == _T("gt"))
		{
			return _T(">");
		}
		else if (q == _T("amp"))
		{
			return _T("&");
		}
		else if (q == _T("quot"))
		{
			return _T("\"");
		}
	}

	return r;
}





