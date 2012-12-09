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

// ImagePNG.h: interface for the CImagePNG class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BITMAPQUANTIZE_H__8D6E46FD_5D9F_4CBC_B227_47168453B0F3__INCLUDED_)
#define AFX_BITMAPQUANTIZE_H__8D6E46FD_5D9F_4CBC_B227_47168453B0F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "png/png.h"

#include "ImageBitmap.h"

class CImagePNG: public CImageBitmap
{
private:
	typedef struct QuantizedColorType
	{
		unsigned char RGB[3];
		unsigned char NewColorIndex;
		long Count;
		struct QuantizedColorType *Pnext;
	} QuantizedColorType;

	typedef struct NewColorMapType
	{
		unsigned char RGBMin[3], RGBWidth[3];
		int NumEntries;/* # of QuantizedColorType in linked list below. */
		long Count; /* Total number of pixels in all the entries. */
		QuantizedColorType *QuantizedColors;
	} NewColorMapType;

	bool SubdivColorMap(NewColorMapType *NewColorSubdiv, int ColorMapSize, int *NewColorMapSize);

	static int __cdecl SortCmpRtn(const void * Entry1, const void * Entry2);

	static int SortRGBAxis;

	CBitmap m_output;
	unsigned char* m_output_bits;
	png_color m_ColorMap[256];
	int m_ColorMapSize;

	// The output of the create bitmap can be converted to a 
	// bitmap with this function...
	void CreateOutputBitmap(CBitmap &output, CDC &dc);

	// Convert a bitmap into 256 colour palette
	void QuantizeBuffer(CBitmap &output, CDC &dc);

	struct png_buffer
	{
		const unsigned char* m_pData;
		size_t m_size_left;
	};

	static void __cdecl user_read_function(png_structp png_ptr, png_bytep data, png_size_t length);

	// Read from a memory block
	virtual bool Read(CDC &ref_dc, const unsigned char *buf, size_t size);

	// Write to a stream (either memory or file...)
	void Save(CDC &ref_dc, void *io_ptr, void *write_data_fn, void *output_flush_fn);

public:
	bool ImportBMP(const TCHAR *filename);
	bool ImportBitmap(CBitmap &bitmap);

	CImagePNG();
	virtual ~CImagePNG();

	// Write out as a PNG file
	void Save(CDC &ref_dc, const TCHAR *filename);

	// Get the type of this object
	virtual CString GetType();

};

#endif // !defined(AFX_BITMAPQUANTIZE_H__8D6E46FD_5D9F_4CBC_B227_47168453B0F3__INCLUDED_)
