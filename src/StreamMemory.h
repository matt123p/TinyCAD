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

// StreamMemory.h: interface for the CStreamMemory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STREAMMEMORY_H__5C823082_CFAD_4E46_84DF_AF879687A02F__INCLUDED_)
#define AFX_STREAMMEMORY_H__5C823082_CFAD_4E46_84DF_AF879687A02F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Stream.h"

class CStreamMemory : public CStream  
{
protected:
	CByteArray	m_Data;
	int			m_read_location;

public:
	CStreamMemory();
	virtual ~CStreamMemory();

	virtual CByteArray &CStreamMemory::GetByteArray() const;

	// Flush this stream
	virtual void Flush();

	// Close this stream
	virtual void Close();

	// The write operator
	virtual void Write(const void* lpBuf, UINT nMax );
	virtual UINT Read(void* lpBuf, UINT nMax );

	// Move back to the start of the stream
	virtual void Seek(LONG pos);

	// Get the current position in the stream
	virtual LONG GetPos();

	virtual bool operator==( const CStreamMemory &stream ) const; // Compare two objects for equality
	virtual bool operator!=( const CStreamMemory &stream ) const; // Compare two objects for unequality


	////////////////////////////////////////////////////
	// The default operators
	//
	template<class T> CStream &operator<<( const T& a )
	{
		Write( &a, sizeof(T) );
		return *this;
	}

	template<class T> CStream &operator>>( T& a )
	{
		Read( &a, sizeof(T) );
		return *this;
	}


	////////////////////////////////////////////////////
	// The CString operator
	//
	virtual CStream &operator<<( const CString s );
	virtual CStream &operator>>( CString &s );

};

#endif // !defined(AFX_STREAMMEMORY_H__5C823082_CFAD_4E46_84DF_AF879687A02F__INCLUDED_)
