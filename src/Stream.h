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

// Stream.h: interface for the CStream class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STREAM_H__B43451E8_657F_401D_A3C9_B34CC6FCB07D__INCLUDED_)
#define AFX_STREAM_H__B43451E8_657F_401D_A3C9_B34CC6FCB07D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "libiconv\iconv.h"

#define CHARSET_INVALID		(iconv_t)(-1)

class CStream
{

public:
	CStream();
	virtual ~CStream();

	// Flush this stream
	virtual void Flush() = 0;

	// Close this stream
	virtual void Close() = 0;

	// Get the default rotation for this symbol
	virtual int GetOrientation();

	// The read & write operators (including conversion)
	virtual void Write(const void* lpBuf, UINT nMax) = 0;
	virtual UINT Read(void* lpBuf, UINT nMax) = 0;

	// Move back to the start of the stream
	virtual void Seek(LONG pos) = 0;

	// Get the current position in the stream
	virtual LONG GetPos() = 0;

	////////////////////////////////////////////////////
	// The default operators
	//
	template<class T> CStream &operator<<(const T& a)
	{
		Write(&a, sizeof(T));
		return *this;
	}

	template<class T> CStream &operator>>(T& a)
	{
		Read(&a, sizeof(T));
		return *this;
	}

	// The CString operators
	virtual CStream &operator<<(const CString s) = 0;
	virtual CStream &operator>>(CString &s) = 0;
};

class CStreamFile: public CStream
{
private:
	CArchive *m_pArchive;
	BOOL m_auto_archive;

public:

	CStreamFile(CArchive *a);
	CStreamFile(CFile *f = NULL, UINT nmode = 0);
	virtual ~CStreamFile();

	// Flush this stream
	virtual void Flush()
	{
		m_pArchive->Flush();
	}

	// Close this stream
	virtual void Close()
	{
		m_pArchive->Close();
	}

	// Attach a file
	void Attach(CFile *theFile, UINT nmode)
	{
		m_pArchive = new CArchive(theFile, nmode);
		m_auto_archive = TRUE;
	}

	// Get the file associated with this stream
	CFile* GetFile()
	{
		return m_pArchive->GetFile();
	}

	// Move back to the start of the stream
	virtual void Seek(LONG pos);

	// Get the current position in the stream
	virtual LONG GetPos();

	// The write operator
	virtual void Write(const void* lpBuf, UINT nMax)
	{
		m_pArchive->Write(lpBuf, nMax);
	}

	virtual UINT Read(void* lpBuf, UINT nMax)
	{
		return m_pArchive->Read(lpBuf, nMax);
	}

	////////////////////////////////////////////////////
	// The default operators
	//
	template<class T> CStream &operator<<(const T& a)
	{
		Write(&a, sizeof(T));
		return *this;
	}

	template<class T> CStream &operator>>(T& a)
	{
		Read(&a, sizeof(T));
		return *this;
	}

	////////////////////////////////////////////////////
	// The CString operator
	//
	virtual CStream &operator<<(const CString s)
	{
		(*m_pArchive) << s;
		return *this;
	}

	virtual CStream &operator>>(CString &s)
	{
		(*m_pArchive) >> s;
		return *this;
	}

};

#endif // !defined(AFX_STREAM_H__B43451E8_657F_401D_A3C9_B34CC6FCB07D__INCLUDED_)
