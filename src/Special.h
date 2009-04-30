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


class AnotateSetup 
{
public:
	enum AddOrRemove { ADD, REMOVE };
	enum WhichReferences { ALL, UNNUMBERED, MATCHING };
	enum StartValue { DEFAULT, SPECIFIED };

	enum AddOrRemove action;
	enum WhichReferences reference;
	enum StartValue value;

	CString matchval;
	int startval;
	bool all_sheets;

	AnotateSetup();
};

// The dialog for the auto anotate function
class CDlgAnotateBox : public CDialog 
{

protected:
	void ReadData();
	void OnChangeStart();
	virtual void OnOK();
	virtual void OnRefPainter();
	virtual BOOL OnInitDialog();

public:
	AnotateSetup	v;

	CDlgAnotateBox(CWnd *pParentWnd,AnotateSetup NewV) : CDialog( IDD_ANOTATE, pParentWnd ) { v = NewV; }

	DECLARE_MESSAGE_MAP()
};

