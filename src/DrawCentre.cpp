/*
	TinyCAD program for schematic capture
	Copyright 1994/1995/2002,2003 Matt Pyne.

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




// This handles the actual drawing of objects

#include "stdafx.h"
#include "TinyCadView.h"
#include "diag.h"
#include "colour.h"
#include "option.h"



////// The Zoom object //////

ObjType CDrawCentre::GetType()
{
  return xNoDraw;
}


// Centre also zooms in!
void CDrawCentre::LButtonDown(CDPoint p, CDPoint)
{
	AfxGetMainWnd()->PostMessage( WM_COMMAND, IDM_VIEWZOOMIN );
}

// Zoom out
BOOL CDrawCentre::RButtonDown(CDPoint p, CDPoint s)
{
	AfxGetMainWnd()->PostMessage( WM_COMMAND, IDM_VIEWZOOMOUT );
  return TRUE;
}


void CDrawCentre::Display( BOOL erase )
{
}






