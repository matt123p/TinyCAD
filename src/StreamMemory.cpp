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

// StreamMemory.cpp: implementation of the CStreamMemory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tinycad.h"
#include "StreamMemory.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStreamMemory::CStreamMemory()
{
	m_read_location = 0;
	m_Data.SetSize(0, 8 * 1024);
}

CStreamMemory::~CStreamMemory()
{

}

// Write this data to memory
CByteArray &CStreamMemory::GetByteArray() const
{
	return (CByteArray &) m_Data;
}

// Flush this stream
void CStreamMemory::Seek(LONG pos)
{
	m_read_location = pos;
}

LONG CStreamMemory::GetPos()
{
	return m_read_location;
}

// Flush this stream
void CStreamMemory::Flush()
{
}

// Close this stream
void CStreamMemory::Close()
{
	Flush();
	m_read_location = 0;
}

// The write operator
void CStreamMemory::Write(const void* lpBuf, UINT nMax)
{
	const char *c = static_cast<const char*> (lpBuf);
	while (nMax > 0)
	{
		m_Data.Add(*c);
		c++;
		nMax--;
	}
}

UINT CStreamMemory::Read(void* lpBuf, UINT nMax)
{
	int size = m_Data.GetSize();
	int r = 0;
	char *c = static_cast<char*> (lpBuf);
	while (m_read_location < size && nMax > 0)
	{
		*c = m_Data.GetAt(m_read_location);
		m_read_location++;
		nMax--;
		r++;
		c++;
	}

	return r;
}

////////////////////////////////////////////////////
// The CString operator
//
CStream &CStreamMemory::operator<<(const CString s)
{
	// First write out the number of bytes in the string
	int len = s.GetLength();
	(*this) << len;

	// Now write the data...
	Write(s, len);

	return *this;
}

CStream &CStreamMemory::operator>>(CString &s)
{
	// First read the number of bytes for the string
	int len = 0;
	s = "";

	if (Read(&len, sizeof (len)) != sizeof (len))
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
		TCHAR *buf = s.GetBuffer(len + 1);
		Read(buf, len);
		buf[len] = 0;
		s.ReleaseBuffer();
	}

	return *this;
}

// Compare two objects for equality
bool CStreamMemory::operator==(const CStreamMemory &stream) const
{
	CByteArray &data = stream.GetByteArray();
	if (m_Data.GetSize() == data.GetSize())
	{
		if (memcmp(m_Data.GetData(), data.GetData(), m_Data.GetSize()) == 0)
		{
			return true;
		}
	}

	return false;
}

bool CStreamMemory::operator!=(const CStreamMemory &stream) const
{
	return ! (*this == stream);
}
