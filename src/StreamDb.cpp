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

// StreamDb.cpp: implementation of the CStreamDb class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tinycad.h"
#include "StreamDb.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStreamDb::CStreamDb( CDaoDatabase* database, 
					 BOOL read, int pos, int orientation )
: m_set( database )
{
	m_read_location = 0;
	m_read = read;
	m_new = FALSE;

	if (pos != -1)
	{
		m_set.m_strFilter.Format(_T("[SymbolID]=%d"),pos);

		if (!m_read)
		{
			// We delete the old record, if we are not reading it...
			m_set.Open();
			m_set.Delete();
			m_set.Close();
		}
	}

	m_set.Open();

	if (!m_read)
	{
		m_new = TRUE;
		m_set.AddNew();
		m_set.m_Type = 1;
		m_set.m_DefRotate = orientation;
		m_set.m_DrawRotate = 0;
	}
}

CStreamDb::~CStreamDb()
{
	Close();
}

// Get the default rotation for this symbol
int CStreamDb::GetOrientation()
{
	return m_set.m_DefRotate;
}



// Flush this stream
void CStreamDb::Flush()
{
	if (!m_read)
	{
		m_set.SetFieldNull( &m_set.m_Data, FALSE);
		m_set.SetFieldDirty( &m_set.m_Data );
		m_set.Update();

		// If this was a new record, then we need to find
		// the ID it has been assigned.
		if (m_new)
		{
			m_set.MoveLast();
		}
		m_read = TRUE;
		m_new = FALSE;
	}
}

// Reset this stream
void CStreamDb::Seek(LONG pos)
{
	m_read_location = pos;
}

LONG CStreamDb::GetPos()
{
	return m_read_location;
}



// Close this stream
void CStreamDb::Close()
{
	Flush();
	m_read_location = 0;
	m_set.Close();
}

// The write operator
void CStreamDb::Write(const void* lpBuf, UINT nMax )
{
	const char *c = static_cast<const char*>(lpBuf);
	while (nMax > 0)
	{
		m_set.m_Data.Add( *c );
		c ++;
		nMax --;
	}
}

UINT CStreamDb::Read(void* lpBuf, UINT nMax )
{
	int size = m_set.m_Data.GetSize();
	int r = 0;
	char *c = static_cast<char*>(lpBuf);
	while (m_read_location < size && nMax > 0)
	{
		*c = m_set.m_Data.GetAt( m_read_location );
		m_read_location ++;
		nMax --;
		r ++;
		c ++;
	}

	return r;
}

////////////////////////////////////////////////////
// The CString operator
//
CStream &CStreamDb::operator<<( const CString s )
{
	// First write out the number of bytes in the string
	int len = s.GetLength();
	(*this) << len;

	// Now write the data...
	Write( s, len );

	return *this;
}

CStream &CStreamDb::operator>>( CString &s )
{
	// First read the number of bytes for the string
	int len;
	s = "";

	if (Read(&len,sizeof(len)) != sizeof(len))
	{
		return *this;
	}

	// Sanity check, is this less than 64K?
	if (len > 1024 * 64)
	{
		return *this;
	}

	// Now read in the data
	if (len != 0)
	{
		char *buf = new char[ len + 1 ];
		Read( buf, len );
		buf[len] = 0;
		s = buf;
		delete buf;
	}


	return *this;
}
