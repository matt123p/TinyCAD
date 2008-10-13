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

// StreamDb.h: interface for the CStreamDb class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STREAMDB_H__8A578F55_9832_4091_9F28_00D2ED68D3DF__INCLUDED_)
#define AFX_STREAMDB_H__8A578F55_9832_4091_9F28_00D2ED68D3DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Stream.h"
#include "DbSymbolSet.h"

class CStreamDb : public CStream  
{
protected:
	int				m_read_location;
	BOOL			m_read;
	BOOL			m_new;

public:
	CDbSymbolSet	m_set;


	CStreamDb( CDaoDatabase* database, BOOL read, 
		int pos = -1, int orientation = 0 );

	virtual ~CStreamDb();

	// Flush this stream
	virtual void Flush();

	// Close this stream
	virtual void Close();

	// Get the default rotation for this symbol
	virtual int GetOrientation();

	// Move back to the start of the stream
	virtual void Seek(LONG pos);

	// Get the current position in the stream
	virtual LONG GetPos();


	// The write operator
	virtual void Write(const void* lpBuf, UINT nMax );
	virtual UINT Read(void* lpBuf, UINT nMax );

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

#endif // !defined(AFX_STREAMDB_H__8A578F55_9832_4091_9F28_00D2ED68D3DF__INCLUDED_)
