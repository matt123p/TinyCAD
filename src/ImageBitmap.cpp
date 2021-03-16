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

// ImageBitmap.cpp: implementation of the CImageBitmap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageBitmap.h"
#include "XMLWriter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImageBitmap::CImageBitmap()
{
	m_Width = 0;
	m_Height = 0;
	m_Bpp = 24;
	m_input_bits = NULL;
	m_compressed_data = NULL;
	m_compressed_size = 0;
	m_invalid_image = false;
}

CImageBitmap::~CImageBitmap()
{
	if (m_compressed_data)
	{
		delete m_compressed_data;
	}

	if (m_input_bits)
	{
		m_input.DeleteObject();
		m_input_bits = NULL;
	}
}

// Roate this image through 90 degrees
void CImageBitmap::Rotate(CDC &dc)
{
	// Keep a copy of the old bitmap
	if (!m_input_bits)
	{
		return;
	}

	CBitmap old_input;
	old_input.Attach(m_input.Detach());

	unsigned char* old_input_bits = m_input_bits;
	m_input_bits = NULL;

	int old_RowStep = m_RowStep;

	// Create a new bitmap to place our data into...
	CreateImageBitmap(m_Height, m_Width, dc, m_Bpp);

	// Now perform the rotation...
	int bytes_per_pixel = (m_Bpp / 8);
	for (int y = 0; y < m_Height; y++)
	{
		unsigned char* p = m_input_bits + y * m_RowStep * bytes_per_pixel;

		for (int x = 0; x < m_Width; x++)
		{
			// Locate the old location of this pixel
			unsigned char *old_p = old_input_bits + (old_RowStep * x * bytes_per_pixel) + ( (m_Height - y) * bytes_per_pixel);

			for (int i = 0; i < bytes_per_pixel; i++)
			{
				*p = *old_p;
				++p;
				++old_p;
			}
		}
	}
}

CBitmap *CImageBitmap::CreateImageBitmap(int Width, int Height, CDC &dc, int bpp)
{
	m_Width = Width;
	m_Height = Height;
	m_Bpp = bpp;

	if (m_input_bits)
	{
		m_input.DeleteObject();
		m_input_bits = NULL;
	}

	// Create the bitmap
	CBitmap bitmap;
	struct
	{
		BITMAPINFO bi;
		RGBQUAD bipal[3];
	} q;

	if (bpp == 32)
	{
		m_RowStep = m_Width;
	}
	else if (bpp == 24)
	{
		m_RowStep = m_Width * 3;
		while ( (m_RowStep % 4) != 0)
		{
			m_RowStep++;
		}
	}

	q.bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	q.bi.bmiHeader.biWidth = Width;
	q.bi.bmiHeader.biHeight = Height;
	q.bi.bmiHeader.biPlanes = 1;
	q.bi.bmiHeader.biBitCount = (WORD) bpp;
	q.bi.bmiHeader.biCompression = BI_RGB;
	q.bi.bmiHeader.biSizeImage = 0;
	q.bi.bmiHeader.biXPelsPerMeter = 0;
	q.bi.bmiHeader.biYPelsPerMeter = 0;
	q.bi.bmiHeader.biClrUsed = 3;
	q.bi.bmiHeader.biClrImportant = 0;
	q.bi.bmiColors[0].rgbRed = 0;
	q.bi.bmiColors[0].rgbGreen = 0;
	q.bi.bmiColors[0].rgbBlue = 0;
	q.bi.bmiColors[0].rgbReserved = 0;

	m_input_bits = NULL;
	HBITMAP hb = CreateDIBSection(dc.m_hDC, &q.bi, DIB_RGB_COLORS, (void**) &m_input_bits, NULL, 0);
	m_input.Attach(hb);

	return &m_input;
}

void CImageBitmap::Paint(CDC &dc, CRect r, int rotmir)
{
	// Is there anything we can do?
	if (!m_compressed_data && !m_input_bits)
	{
		return;
	}

	// Do we need to decompress the image?
	if ((!m_input_bits || dc.IsPrinting()) && m_compressed_data)
	{
		m_invalid_image = !Read(dc, m_compressed_data, m_compressed_size);
	}

	// Now blt it to the screen
	if (m_invalid_image)
	{
		PaintInvalid(dc, r);
	}
	else
	{
		CDC bitmap_dc;
		bitmap_dc.CreateCompatibleDC(&dc);
		CBitmap *old_bitmap = bitmap_dc.SelectObject(&m_input);

		dc.SetStretchBltMode(HALFTONE);
		dc.SetBrushOrg(r.left, r.top);
		dc.StretchBlt(r.left, r.top, r.Width(), r.Height(), &bitmap_dc, 0, 0, m_Width, m_Height, SRCCOPY);

		bitmap_dc.SelectObject(old_bitmap);
	}

	if (m_input_bits && dc.IsPrinting())
	{
		m_input.DeleteObject();
		m_input_bits = NULL;
	}
}

// Get the size of this bitmap
CSize CImageBitmap::GetSize(CDC &dc)
{
	// Is there anything we can do?
	if (!m_compressed_data && !m_input_bits)
	{
		return CSize(100, 100);
	}

	// Do we need to decompress the image?
	if (!m_input_bits && m_compressed_data)
	{
		m_invalid_image = !Read(dc, m_compressed_data, m_compressed_size);
	}

	return CSize(m_Width, m_Height);
}

// Set the memory image data
bool CImageBitmap::SetCompressedData(unsigned char *data, size_t size)
{
	if (m_compressed_data)
	{
		delete m_compressed_data;
	}

	m_compressed_data = data;
	m_compressed_size = size;

	return true;
}

// Write as an UUENCODE XML tag
void CImageBitmap::SaveXML(CXMLWriter &xml)
{
	// Write the size to the archive
	xml.addChildDataUUencode(m_compressed_data, m_compressed_size);
}

