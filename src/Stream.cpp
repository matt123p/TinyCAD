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

// Stream.cpp: implementation of the CStream class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tinycad.h"
#include "Stream.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction for CStream
//////////////////////////////////////////////////////////////////////


CStream::CStream()
{
}

CStream::~CStream()
{
}


// Get the default rotation for this symbol
int CStream::GetOrientation()
{
	return 0;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction for CStreamFile
//////////////////////////////////////////////////////////////////////


CStreamFile::CStreamFile(CArchive *a)
{
	m_pArchive = a;
	m_auto_archive = FALSE;
}

CStreamFile::CStreamFile(CFile *f, UINT nmode)
{
	m_pArchive = NULL;

	if (f)
	{
		Attach( f, nmode );
	}
}

CStreamFile::~CStreamFile()
{
	if (m_auto_archive)
	{
		delete m_pArchive;
	}
}


// Reset this stream
void CStreamFile::Seek(LONG pos)
{
	m_pArchive->Flush();
	GetFile()->Seek( pos, CFile::begin );
}

LONG CStreamFile::GetPos()
{
	m_pArchive->Flush();
	return static_cast<LONG>(GetFile()->GetPosition());
}



