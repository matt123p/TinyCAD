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

// Image.h: interface for the CTCImage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGE_H__2833F346_CB97_455E_899B_E8D6A511D724__INCLUDED_)
#define AFX_IMAGE_H__2833F346_CB97_455E_899B_E8D6A511D724__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class CXMLWriter;

class CTCImage
{
public:
	CTCImage();
	virtual ~CTCImage();

	// Write as an UUENCODE XML tag
	virtual void SaveXML(CXMLWriter &xml) = 0;

	// Set the memory image data
	virtual bool SetCompressedData(unsigned char *data, size_t size) = 0;

	// Draw this image on the screen
	virtual void Paint(CDC &dc, CRect rect, int rotmir) = 0;

	// Get the type of this object
	virtual CString GetType() = 0;

	// Determine the size of this bitmap
	virtual CSize GetSize(CDC &dc) = 0;

	static void PaintInvalid(CDC &dc, CRect rect);
};

#endif // !defined(AFX_IMAGE_H__2833F346_CB97_455E_899B_E8D6A511D724__INCLUDED_)
