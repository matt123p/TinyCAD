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

// ImageMetafile.h: interface for the CImageMetafile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGEMETAFILE_H__2DCC1D1F_2278_47E0_8FAA_BADED5BF1DDF__INCLUDED_)
#define AFX_IMAGEMETAFILE_H__2DCC1D1F_2278_47E0_8FAA_BADED5BF1DDF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Image.h"
#include "XMLWriter.h"

class CImageMetafile: public CTCImage
{
protected:
private:
	HENHMETAFILE m_oMetaFile;

public:
	CImageMetafile(HENHMETAFILE oMetaFile = NULL);
	virtual ~CImageMetafile();

	// Write as an UUENCODE XML tag
	virtual void SaveXML(CXMLWriter &xml);

	// Draw this image on the screen
	virtual void Paint(CDC &dc, CRect r, int rotmir);

	// Set the memory image data
	virtual bool SetCompressedData(unsigned char *data, size_t size);

	// Get the type of this object
	virtual CString GetType();

	// Get the size of this bitmap
	virtual CSize GetSize(CDC &)
	{
		return CSize(100, 100);
	}

};

#endif // !defined(AFX_IMAGEMETAFILE_H__2DCC1D1F_2278_47E0_8FAA_BADED5BF1DDF__INCLUDED_)
