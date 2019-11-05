/*
 * Project:		TinyCAD program for schematic capture
 *				https://www.tinycad.net
 * Copyright:	© 1994-2019 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "DocResource.h"

//=========================================================================
//== ctor/dtor/initializing                                              ==
//=========================================================================

//-------------------------------------------------------------------------
CDocResource::CDocResource()
{
	ResourceNumber = 0;
	MergeNumber = 0;
	InUse = false;
	next = NULL;
}
//-------------------------------------------------------------------------
CDocResource::~CDocResource()
{
	// Delete down the chain...
	delete next;
}
//-------------------------------------------------------------------------

//=========================================================================
//== accessor/mutator                                                    ==
//=========================================================================

//-------------------------------------------------------------------------
hRESOURCE CDocResource::GetResourceNumber() const
{
	return ResourceNumber;
}
//-------------------------------------------------------------------------


//=========================================================================
//==                                                                     ==
//=========================================================================

//-------------------------------------------------------------------------
hRESOURCE CDocResource::ConvertOld(hRESOURCE nOldID)
{
	// If the number was out of range then return the default font
	hRESOURCE nReturn = 0;

	for (CDocResource* pointer = this; pointer != NULL; pointer = pointer->next)
	{
		if (nOldID == pointer->MergeNumber)
		{
			nReturn = pointer->ResourceNumber;
		}
	}

	return nReturn;
}
//-------------------------------------------------------------------------
//-- Add a new resource to this list
hRESOURCE CDocResource::Add(CDocResource* NewResource, hRESOURCE NewMerge)
{
	CDocResource* pointer = this;
	CDocResource* last = NULL;
	hRESOURCE nextNumber = 0;

	while (pointer != NULL && ! (pointer->MergeNumber == -1 && pointer->Compare(NewResource)))
	{
		// Find the next index number available
		if (pointer->ResourceNumber >= nextNumber) nextNumber = hRESOURCE(pointer->ResourceNumber + 1);

		last = pointer;
		pointer = pointer->next;
	}

	// This resouce is already defined
	if (pointer != NULL)
	{
		pointer->MergeNumber = NewMerge;
		// The new resource is not required and can be deleted!
		delete NewResource;
		// Now return the resource number
		return pointer->ResourceNumber;
	}

	ASSERT(last != NULL);

	// Not defined so add the resource to the end of the list
	NewResource->ResourceNumber = nextNumber;
	NewResource->MergeNumber = NewMerge;
	last->next = NewResource;

	return nextNumber;
}
//-------------------------------------------------------------------------
// Get a resource from the list
CDocResource* CDocResource::Get(hRESOURCE nID)
{
	// If the number was out of range then return the default font
	// (the head of the list)
	CDocResource* pReturn = this;

	for (CDocResource* pointer = this; pointer != NULL; pointer = pointer->next)
	{
		if (pointer->ResourceNumber == nID)
		{
			pReturn = pointer;
		}
	}

	return pReturn;
}
//-------------------------------------------------------------------------
// Tag a resource given a resource number
void CDocResource::Tag(hRESOURCE nID)
{
	for (CDocResource* pointer = this; pointer != NULL; pointer = pointer->next)
	{
		if (pointer->ResourceNumber == nID)
		{
			pointer->InUse = true;
		}
	}
}
//-------------------------------------------------------------------------
void CDocResource::UnTag()
{
	for (CDocResource* pointer = this; pointer != NULL; pointer = pointer->next)
	{
		pointer->InUse = false;
	}
}
//-------------------------------------------------------------------------
// Clear the merge numbers
void CDocResource::ResetMerge()
{
	// All the merge numbers to invalid
	for (CDocResource* pointer = this; pointer != NULL; pointer = pointer->next)
	{
		pointer->MergeNumber = -1;
	}
}
//-------------------------------------------------------------------------
void CDocResource::SaveXML(CTinyCadDoc* pDesign, const TCHAR* name, CXMLWriter& xml)
{
	// Now actually save the resources
	for (CDocResource* pointer = this; pointer != NULL; pointer = pointer->next)
	{
		if (pointer->InUse)
		{
			xml.addTag(name);
			xml.addAttribute(_T("id"), pointer->ResourceNumber);
			pointer->SaveItemXML(pDesign, xml);
			xml.closeTag();
		}
	}
}
//-------------------------------------------------------------------------
void CDocResource::Load(CTinyCadDoc* pDesign, CStream& oStream)
{
	// Number of Fonts in this list
	UInt16 nCntRes = 0;
	hRESOURCE nOldResNo = 0;

	// All the merge numbers to invalid
	ResetMerge();

	// Read in the number of Fonts
	oStream >> nCntRes;

	for (int n = 0; n < nCntRes; n++)
	{
		oStream >> nOldResNo;
		LoadItem(pDesign, oStream, nOldResNo);
	}
}
//-------------------------------------------------------------------------
//-- Compare this resource with another resource
bool CDocResource::Comparex(const CDocResource*) const
{
	return false;
}
//-------------------------------------------------------------------------
