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

// Image.cpp: implementation of the CImage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tinycad.h"
#include "Image.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImage::CImage()
{
}

CImage::~CImage()
{
}

void CImage::PaintInvalid(CDC &dc, CRect rect)
{
	CPen *old_pen = (CPen *) dc.SelectStockObject(BLACK_PEN);
	CBrush *old_brush = (CBrush *) dc.SelectStockObject(WHITE_BRUSH);

	dc.Rectangle(rect);
	dc.ExtTextOut(rect.left + 5, rect.top + 5, ETO_CLIPPED, rect, _T("Invalid image"), 14, NULL);

	dc.SelectObject(old_pen);
	dc.SelectObject(old_brush);
}
