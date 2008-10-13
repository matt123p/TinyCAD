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


#include "stdafx.h"
#include <ctype.h>
#include "TinyCadView.h"
#include "registry.h"
#include "colour.h"
#include "TinyCad.h"
#include "LibraryDb.h"



////// The symbol rename dialog //////


BOOL CDlgSymbolRename::OnInitDialog()
{
  SetDlgItemText(SYMBOLRENAME_OLD,*name);
  SetDlgItemText(SYMBOLRENAME_NEW,*name);

  return TRUE;
}

void CDlgSymbolRename::OnOK()
{
  // Change the symbol name
  GetDlgItemText(SYMBOLRENAME_NEW,name->GetBuffer(STRLEN),STRLEN);
  name->ReleaseBuffer();

  EndDialog(IDOK);
}

void CDlgSymbolRename::OnCancel()
{
  EndDialog(IDCANCEL);
}





