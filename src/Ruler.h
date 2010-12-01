/*
	TinyCAD program for schematic capture
	Copyright 1994/1995/2002 Matt Pyne.

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

#ifndef __RULER_H__
#define __RULER_H__

#include "context.h"

#define RULER_WIDTH 	20

class COriginButton : public CButton {

public:
	COriginButton() { };
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	void OnClick();

protected :
	DECLARE_MESSAGE_MAP()
};

class CMultiSheetDoc;

class Ruler : public CWnd {

private:
	CMultiSheetDoc	*m_pDesign;
	CRect 		Size;
	CPoint 		oldPosition;
	int			dir;
	int			theHeight;
	BOOL		hasOrigin;
	COriginButton     button;

	void ShowPositionEx(CDC &dc, CPoint r);
	Transform& GetTransform();

public:
	Ruler(CMultiSheetDoc*pDesign, int nDir, CRect nSize, CWnd *parent, BOOL hasOrigin);
	void ShowPosition(CPoint r);

	void OnPaint();
	void OnNewSize(CRect nSize);

	DECLARE_MESSAGE_MAP()
};


#endif
