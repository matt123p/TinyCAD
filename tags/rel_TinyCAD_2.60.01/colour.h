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



#ifndef _COLOUR_H_
#define _COLOUR_H_

// Sizes of some objects
#define HIGHLIGHT_SIZE	10
#define JUNCTION_SIZE 3


// The colours for objects
#define cBLACK		RGB(0,0,0)
#define cWHITE		RGB(255,255,255)
#define cRED			RGB(255,0,0)
#define cBLUE			RGB(0,0,255)

#define cSELECT		RGB(255,127,127)
#define cLINE			cBLACK
#define cBOLD			cRED

#define cTEXT			cBLACK
#define cBLOCK			cBLACK
#define cOFFPAGE	RGB(192,192,192)
#define cRULER		RGB(200,200,200)
#define cERROR		RGB(127,0,0)

#define cBLACK_CLK		RGB(32,32,32)
#define cBUS_CLK		RGB(255,32,32)
#define cJUNCTION_CLK	RGB(128,128,128)
#define cLABEL_CLK		RGB(32,128,32)
#define cPIN_CLK		RGB(192,128,128)
#define cNOCONNECT_CLK	RGB(32,32,32)
#define cRULER_CLK		RGB(128,128,128)

#define	fTEXT		0
#define	fPIN		1
#define fRULE		2

#define fLINE		0
#define fDASH		1

#define fsNONE		0

#endif

