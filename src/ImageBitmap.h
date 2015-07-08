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

// ImageBitmap.h: interface for the CImageBitmap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGEBITMAP_H__C57AAB25_1E6B_49DE_B7CA_2287398BD939__INCLUDED_)
#define AFX_IMAGEBITMAP_H__C57AAB25_1E6B_49DE_B7CA_2287398BD939__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Image.h"

class CImageBitmap: public CTCImage
{
protected:
	// The image bitmap...
	CBitmap m_input;
	unsigned char* m_input_bits;

	// The input compressed image
	unsigned char* m_compressed_data;
	size_t m_compressed_size;

	// The size of the bitmap
	int m_Bpp;
	int m_Width;
	int m_Height;
	int m_RowStep;

	bool m_invalid_image;

	// Read from a memory block
	virtual bool Read(CDC &ref_dc, const unsigned char *buf, size_t size) = 0;

public:
	CImageBitmap();
	virtual ~CImageBitmap();

	// Create bitmap for you to draw into
	CBitmap *CreateImageBitmap(int Width, int Height, CDC &dc, int bpp);

	// Roate this image through 90 degrees
	void Rotate(CDC &dc);

	// Set the memory image data
	virtual bool SetCompressedData(unsigned char *data, size_t size);

	// Write as an UUENCODE XML tag
	virtual void SaveXML(CXMLWriter &xml);

	// Get the size of this bitmap
	virtual CSize GetSize(CDC &dc);

	// Draw this image on the screen
	virtual void Paint(CDC &dc, CRect rect, int rotmir);

};

#endif // !defined(AFX_IMAGEBITMAP_H__C57AAB25_1E6B_49DE_B7CA_2287398BD939__INCLUDED_)
