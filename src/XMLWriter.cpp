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

// XMLWriter.cpp: implementation of the CXMLWriter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tinycad.h"
#include "XMLWriter.h"
#include "XMLException.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXMLWriter::CXMLWriter(CStream*	pOutput)
{
	m_child_data = false;
	m_pOutput = pOutput;
	m_charset_conv = CHARSET_INVALID;
	m_conversion_buf_size = 0;
	m_conversion_buffer = NULL;
	m_nesting_level = -1;
	m_needs_newline = false;
	m_needs_tabs = false;
    m_line_counter = 1;

	// Setup the default encoding
#ifdef UNICODE
	SetCharset( "UTF-8", "UCS-2-INTERNAL" );
#else
	SetCharset( "UTF-8", "char" );
#endif
	SendString( _T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n") );
}

CXMLWriter::~CXMLWriter()
{
	if (m_charset_conv != CHARSET_INVALID)
	{
		iconv_close( m_charset_conv );
	}
	delete m_conversion_buffer;
}

//////////////////////////////////////////////////////////////////////
// Set up the charaset conversions
//////////////////////////////////////////////////////////////////////

void CXMLWriter::SetCharset( const char* tocode, const char* fromcode )
{
	if (m_charset_conv != CHARSET_INVALID)
	{
		iconv_close( m_charset_conv );
	}

	m_charset_conv = iconv_open( tocode, fromcode );
}

//////////////////////////////////////////////////////////////////////
// Add comments to the file
//////////////////////////////////////////////////////////////////////

void CXMLWriter::addComment( const TCHAR *comment )
{

	SendString(_T("\r\n<!--"));
	SendString(comment);
	SendString(_T("-->\r\n"));

}

//////////////////////////////////////////////////////////////////////
// Create a new tag 
//////////////////////////////////////////////////////////////////////


void CXMLWriter::addTag(const TCHAR *tag_name)
{
	// Do we need to close the previous tag?
	if (!m_child_data && m_tags.size() != 0)
	{
		closeOpenTag();
	}

	// Start a new tag, and push it's name onto the tag stack...
	m_tags.push_front( tag_name );

	++m_nesting_level;	//track the level of nesting
	if (m_needs_newline) {
		SendString( _T("\r\n"));
	}
	m_needs_newline = true;		//if not needed, it will be cleared by closeTag().  If still needed the next time that this function is entered, it will output a newline
	m_needs_tabs = false;	//flag will be set by closeTag() after outputting tabs, if needed

	assert(m_nesting_level >= -1);	//make sure that we have the nesting level matched properly

	for (int i=m_nesting_level; i > 0; --i) {
		SendString(_T("\t"));
	}

	SendString(_T("<"));
	SendString(tag_name);
	m_child_data = false;
}

void CXMLWriter::internal_addTag(const TCHAR *name, const TCHAR *data)
{
	addTag( name );
	internal_addChildData( data );
	closeTag();
}


//////////////////////////////////////////////////////////////////////
// Add attributes to the tag
//////////////////////////////////////////////////////////////////////


void CXMLWriter::internal_addAttribute(const TCHAR *name, const TCHAR *value)
{
	// Do we have child data?
	if (m_child_data)
	{
		throw new CXMLException(ERR_XML_HAVE_CHILD_DATA, m_line_counter, TRUE);
	}

	// Is there an open tag?
	if (m_tags.size() == 0)
	{
		throw new CXMLException(ERR_XML_NO_TAG, m_line_counter, TRUE);
	}

	SendString(_T(" "));
	SendString(name);
	SendString(_T("='"));
	SendString(value);
	SendString(_T("'"));
}



//////////////////////////////////////////////////////////////////////
// Add child data to the current tags
//////////////////////////////////////////////////////////////////////


void CXMLWriter::internal_addChildData(const TCHAR *data)
{
	// Is there an open tag?
	if (m_tags.size() == 0)
	{
		throw new CXMLException(ERR_XML_NO_TAG, m_line_counter, TRUE);		
	}

	if (!m_child_data)
	{
		closeOpenTag();
	}
	m_child_data = true;
	SendString( data );
}



//////////////////////////////////////////////////////////////////////
// Close the current tag
//////////////////////////////////////////////////////////////////////


void CXMLWriter::closeTag()
{
	//if the previous output contained a newline, then the tabs need to be output before the closing tag
	if (m_needs_tabs) {
		for (int i=m_nesting_level; i > 0; --i) {
			SendString(_T("\t"));
		}
	}

	// Did we have any child data?
	if (m_child_data)
	{
		// Yes, so need to close the tag with another tag
		SendString( _T("</") );
		SendString( m_tags.front() );
		SendString( _T(">\r\n") );
	}
	else
	{
		// No, so just close in a single tag
		SendString( _T("/>\r\n") );
	}

	m_tags.pop_front();
	m_child_data = m_tags.size() != 0;

	--m_nesting_level;	//track the level of nesting
	assert(m_nesting_level >= -1);

	//set the flags that help to output extra newlines and tabs when double pushing or double popping
	m_needs_newline = false;	//this flag is set by addTag()
	m_needs_tabs = true;	//this flag will be cleared by addTag() if it has already added tabs
}


void CXMLWriter::closeOpenTag()
{
	SendString( _T(">") );
}



//
// Here are our conversion utilities to convert the data
// into XML strings
//
CString	CXMLWriter::makeString( int data )
{
	CString r;
	r.Format( _T("%d"), data );
	return r;
}

CString	CXMLWriter::makeString( COLORREF data )
{
	CString r;
	r.Format( _T("%06X"), data );
	return r;
}


CString	CXMLWriter::makeString( double data )
{
	CString r;
	r.Format( _T("%.5f"), data );
	return r;
}

CString	CXMLWriter::makeString( CDPoint data )
{
	CString r;
	r.Format( _T("%.5f,%.5f"), data.makeXMLUnits( data.x ), data.makeXMLUnits( data.y ) );
	return r;
}

CString	CXMLWriter::makeString( UINT data )
{
	CString r;
	r.Format( _T("%u"), data );
	return r;
}

CString	CXMLWriter::makeString( const TCHAR *data )
{
	CString r;

	r = data;
	int i = 0;
	int l = _tcslen( data );
	for (int p = 0; p < l; p++)
	{
		CString ins;

		switch (data[p])
		{
		case'<': 
			ins = _T("&lt;");
			break;
		case'>': 
			ins = _T("&gt;");
			break;
		case'&': 
			ins = _T("&amp;");
			break;
		case '\'': 
			ins = _T("&#039;");
			break;
		case '"': 
			ins = _T("&quot;");
			break;
		}

		if (!ins.IsEmpty())
		{
			r = r.Left(i) + ins + r.Mid(i+1);
			i += ins.GetLength();
		}
		else
		{
			i ++;
		}
	}

	return r;
}




// Send a completed string to the output
//
void CXMLWriter::SendString( const TCHAR *str)
{
	size_t size = _tcslen( str );

	// Make sure the output buffer is big enough
	if (m_conversion_buf_size < size*4)
	{
		delete m_conversion_buffer;
		m_conversion_buf_size = size*4;
		m_conversion_buffer = new char[ m_conversion_buf_size ];
	}
	
	// Now perform the conversion
	size_t outbuf_size = m_conversion_buf_size;
	size_t inbuf_size = size * sizeof( TCHAR );
	char *out = m_conversion_buffer;
	iconv( m_charset_conv, (const char**)&str, &inbuf_size, &out, &outbuf_size );
	int	converted_bytes_to_write = out - m_conversion_buffer;

	m_pOutput->Write( m_conversion_buffer, converted_bytes_to_write );
    m_line_counter++;
}



// ENC is the basic 1 character encoding function to make 
// a char printable
#define ENC(c) (((c) & 077) + ' ')

//
// Add child data but uuencode it first...
//
void CXMLWriter::addChildDataUUencode( BYTE *data, UINT size )
{
	// Is there an open tag?
	if (m_tags.size() == 0)
	{
		throw new CXMLException(ERR_XML_NO_TAG, m_line_counter, TRUE);
	}

	if (!m_child_data)
	{
		closeOpenTag();
	}

	m_child_data = true;

	addTag( _T("UUENCODE") );
	addAttribute( _T("size"), size );
	closeOpenTag();
	m_child_data = true;
	SendString(_T("\r\n"));
	
	// Now send the data...
	UINT i = 0;
	while ( i < size )
	{
		TCHAR c[256];
		
		// 1 (up to) 45 character line
		int line_size = min(size - i, 45);
		int p = 0;
		c[p ++] = ENC(line_size);

		while (line_size > 0)
		{
			int i0 = data[i];
			i ++;

			int i1 = 0;
			if (i < size)
			{
				i1 = data[i];
				i ++;
			}

			int i2 = 0;
			if (i < size)
			{
				i2 = data[i];
				i ++;
			}

			// output one group of 3 bytes as 4 chars
			c[p ++] = ENC( i0 >> 2 );
			c[p ++] = ENC( (i0 << 4) & 060 | (i1 >> 4) & 017 );
			c[p ++] = ENC( (i1 << 2) & 074 | (i2 >> 6) & 03 );
			c[p ++] = ENC( i2 & 077 );
			
			line_size -= 3;
		}

		c[p ++] = '\n';
		c[p ++] = 0;

		SendString( makeString( c ) );
	}

	closeTag();
}

