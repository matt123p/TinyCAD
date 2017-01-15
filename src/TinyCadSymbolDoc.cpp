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

// TinyCadSymbolDoc.cpp: implementation of the CTinyCadSymbolDoc class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tinycad.h"
#include "TinyCadSymbolDoc.h"
#include "diag.h"
#include "library.h"
#include "Object.h"
#include "DlgUpdateBox.h"
#include "TinyCadMultiDoc.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTinyCadSymbolDoc::CTinyCadSymbolDoc(CMultiSheetDoc *pParent) :
	CTinyCadDoc(pParent)
{
	m_part = 0;
}

CTinyCadSymbolDoc::~CTinyCadSymbolDoc()
{

}

void CTinyCadSymbolDoc::setSymbol()
{
	// Reset the part number to zero
	m_part = 0;
}

int CTinyCadSymbolDoc::GetPartsPerPackage()
{
	// Find out how many parts in this package
	int max = 0;
	drawingIterator it = GetDrawingBegin();
	while (it != GetDrawingEnd())
	{
		CDrawingObject *pointer = *it;

		if (pointer->GetType() == xPinEx && ((CDrawPin *) pointer)->GetPart() > max)
		{
			max = ((CDrawPin *) pointer)->GetPart();
		}

		++it;
	}

	return max + 1;
}

// Enforce parts per package
void CTinyCadSymbolDoc::SetPartsPerPackage(int p)
{
	if (GetPart() >= p)
	{
		m_part = p - 1;
	}

	drawingCollection dels;

	// Find out how many parts in this package
	drawingIterator it = GetDrawingBegin();
	while (it != GetDrawingEnd())
	{
		CDrawingObject *pointer = *it;

		if (pointer->GetType() == xPinEx && ((CDrawPin *) pointer)->GetPart() >= p)
		{
			dels.push_back(pointer);
		}

		++it;
	}

	// Now perform the deletions
	it = dels.begin();
	while (it != dels.end())
	{
		Delete(*it);
		++it;
	}
}

// Select/add a new symbol for editing
void CTinyCadSymbolDoc::SelectSymbol(CLibraryStoreSymbol *theSymbol)
{
	SelectObject(NULL);

	CDesignFileSymbol *pSymbol = theSymbol->GetDesignSymbol(this);

	drawingCollection method;
	if (pSymbol)
	{
		pSymbol->GetMethod(0, false, method);

		drawingIterator it = method.begin();
		while (it != method.end())
		{
			CDrawingObject *q = Dup(*it);
			Add(q);
			Select(q);
			++it;
		}

		delete pSymbol;
	}

	CDrawBlockImport *pImport = new CDrawBlockImport(this);
	SelectObject(pImport);
	pImport->Import();
}

// Called after a paste or import to enable the
// document to sort out the imported block when
// necessary
void CTinyCadSymbolDoc::PostPaste()
{
	UngroupSymbols();
}

// Set which part in the package to edit
void CTinyCadSymbolDoc::EditPartInPackage(int p)
{
	int OldPart = GetPart();

	// Get rid of any drawing tool
	SelectObject(new CDrawEditItem(this));

	m_part = p;

	// Are there any pins selected for this part
	int innew = FALSE, inold = FALSE;

	drawingIterator it = GetDrawingBegin();
	while (it != GetDrawingEnd())
	{
		CDrawingObject *pointer = *it;

		if (pointer->GetType() == xPinEx && ((CDrawPin *) pointer)->GetPart() == GetPart()) innew = TRUE;
		if (pointer->GetType() == xPinEx && ((CDrawPin *) pointer)->GetPart() == OldPart) inold = TRUE;

		++it;
	}

	// Do we need to copy over the pins?
	if (!innew && inold && Message(IDS_COPYPINS, MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		drawingCollection newPins;
		CDrawPin *NewPin;
		drawingIterator it = GetDrawingBegin();
		while (it != GetDrawingEnd())
		{
			CDrawingObject *pointer = *it;
			if (pointer->GetType() == xPinEx && ((CDrawPin *) pointer)->GetPart() == OldPart)
			{
				NewPin = new CDrawPin(this);
				*NewPin = * ((CDrawPin *) pointer);
				NewPin->SetPart(GetPart());
				newPins.push_back(NewPin);
			}

			++it;
		}
		// Defer adding objects, so iterator is not invalidated iside the loop
		for (drawingIterator pin = newPins.begin(); pin != newPins.end(); ++pin)
		{
			Add(*pin);
		}
	}

	Invalidate();
}

