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

// ImageMetafile.cpp: implementation of the CImageMetafile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageMetafile.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImageMetafile::CImageMetafile(HENHMETAFILE oMetaFile)
{
	m_oMetaFile = oMetaFile;
}

CImageMetafile::~CImageMetafile()
{

}

// Set the memory image data
bool CImageMetafile::SetCompressedData(unsigned char *data, int size)
{
	// Now create the new metafile
	m_oMetaFile = SetEnhMetaFileBits(size, data);

	delete[] data;

	return true;
}

// Write as an UUENCODE XML tag
void CImageMetafile::SaveXML(CXMLWriter &xml)
{
	// Put the meta-file into a memory buffer
	UINT nSize = GetEnhMetaFileBits(m_oMetaFile, 0, NULL);
	BYTE* naBuffer = new BYTE[nSize];

	GetEnhMetaFileBits(m_oMetaFile, nSize, naBuffer);

	// Write the size to the archive
	xml.addChildDataUUencode(naBuffer, nSize);

	// Now discard the buffer
	delete[] naBuffer;
}

// Draw this image on the screen
void CImageMetafile::Paint(CDC &dc, CRect rect, int rotmir)
{
	// Do we need to restore the world transform?
	BOOL RestoreTransform = FALSE;

	// Save the current DC
	int dc_restore_point = dc.SaveDC();

	//??  dc.SelectDefault();

	// Does this device context support advanced transforms?
	if (SetGraphicsMode(dc.m_hDC, GM_ADVANCED) != 0)
	{
		RestoreTransform = TRUE;

		XFORM xform;

		// Normalise the rectangle
		rect.NormalizeRect();

		// Find out which way round the object goes
		static const double PI = 4.0 * atan(1.0);
		int offset_x = rect.left;
		int offset_y = rect.top;
		rect -= CPoint(offset_x, offset_y);
		int fsx = 0;
		int fsy = 0;
		int msx = 0;
		int msy = 0;
		double rotation = 0;
		switch (rotmir & 3)
		{
			case 0: // Up
				rotation = 0;
				msx = rect.Width();
				break;
			case 1: // Down
				rotation = PI;
				fsx = rect.Width();
				fsy = rect.Height();
				msy = rect.Height();
				break;
			case 2: // Left
				rotation = (3.0 * PI) / 2.0;
				fsy = rect.Height();
				break;
			case 3: // Right
				rotation = PI / 2.0;
				fsx = rect.Width();
				msy = rect.Height();
				msx = rect.Width();
				break;
		}

		xform.eM11 = static_cast<float> (cos(rotation));
		xform.eM12 = static_cast<float> (sin(rotation));
		xform.eM21 = static_cast<float> (-sin(rotation));
		xform.eM22 = static_cast<float> (cos(rotation));

		if ( (rotmir & 4) != 0)
		{
			if ( (rotmir & 3) < 2)
			{
				xform.eM11 = -xform.eM11;
			}
			else
			{
				xform.eM12 = -xform.eM12;
			}
			fsx = msx;
			fsy = msy;
		}

		xform.eDx = static_cast<float> (offset_x + fsx);
		xform.eDy = static_cast<float> (offset_y + fsy);

		SetWorldTransform(dc.m_hDC, &xform);
	}

	if ( (rotmir & 3) > 1)
	{
		int width = rect.Width();
		int height = rect.Height();

		rect.right = rect.left + height;
		rect.bottom = rect.top + width;
	}

	PlayEnhMetaFile(dc.m_hDC, m_oMetaFile, &rect);

	if (RestoreTransform)
	{
		ModifyWorldTransform(dc.m_hDC, NULL, MWT_IDENTITY);
		SetGraphicsMode(dc.m_hDC, GM_COMPATIBLE);
	}

	// Now restore the DC
	dc.RestoreDC(dc_restore_point);

}

// Get the type of this object
CString CImageMetafile::GetType()
{
	return "METAFILE";
}

