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

// StreamClipboard.cpp: implementation of the CStreamClipboard class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tinycad.h"
#include "StreamClipboard.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStreamClipboard::CStreamClipboard()
{
	m_read_location = 0;
	m_Data.SetSize(0, 8 * 1024);
}

CStreamClipboard::~CStreamClipboard()
{

}

// Write this data to the clipboard
void CStreamClipboard::SaveToClipboard(UINT ClipboardFormat)
{
	HANDLE hmem = GlobalAlloc(GMEM_MOVEABLE, m_Data.GetSize());
	void *data = GlobalLock(hmem);
	if (data)
	{
		memcpy(data, m_Data.GetData(), m_Data.GetSize());
		GlobalUnlock(hmem);
		SetClipboardData(ClipboardFormat, hmem);
	}
}

// Read data from the clipboard
BOOL CStreamClipboard::ReadFromClipboard(UINT ClipboardFormat)
{
	HANDLE hmem = GetClipboardData(ClipboardFormat);
	if (hmem)
	{
		size_t size = GlobalSize(hmem);
		void *data = GlobalLock(hmem);
		m_Data.SetSize(size);
		memcpy(m_Data.GetData(), data, size);
		GlobalUnlock(hmem);

		return TRUE;
	}
	else
	{
		return FALSE;
	}

}

// Flush this stream
void CStreamClipboard::Seek(LONG pos)
{
	m_read_location = pos;
}

LONG CStreamClipboard::GetPos()
{
	return (LONG) m_read_location;
}

// Flush this stream
void CStreamClipboard::Flush()
{
}

// Close this stream
void CStreamClipboard::Close()
{
	Flush();
	m_read_location = 0;
}

// The write operator
void CStreamClipboard::Write(const void* lpBuf, UINT nMax)
{
	const char *c = static_cast<const char*> (lpBuf);
	while (nMax > 0)
	{
		m_Data.Add(*c);
		c++;
		nMax--;
	}
}

UINT CStreamClipboard::Read(void* lpBuf, UINT nMax)
{
	size_t size = m_Data.GetSize();
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
CStream &CStreamClipboard::operator<<(const CString s)
{
	// First write out the number of bytes in the string
	int len = s.GetLength();
	(*this) << len;

	// Now write the data...
	Write(s, len);

	return *this;
}

CStream &CStreamClipboard::operator>>(CString &s)
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
