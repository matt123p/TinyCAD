/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#pragma once

#include "NewTypes.h"
#include "DocResource.h"
#include "Image.h"

//*************************************************************************
//*                                                                       *
//*                The MetaFile Resource                                  *
//*                                                                       *
//*************************************************************************

// The fill style type
typedef hRESOURCE hMETAFILE;

//=========================================================================
class ListOfMetaFiles: public CDocResource
{
	typedef CDocResource super;

	// The Fill style this describes
private:
	CTCImage* m_pImage;

	//=====================================================================
	//== ctor/dtor/initializing                                          ==
	//=====================================================================
public:
	ListOfMetaFiles();
public:
	ListOfMetaFiles(const ListOfMetaFiles& o);
public:
	ListOfMetaFiles(CTCImage* s);
public:
	~ListOfMetaFiles();
private:
	void Init(CTCImage* s);

	//=====================================================================
	//== accessor/mutator                                                ==
	//=====================================================================
	// Get the data associated with this resource
public:
	CTCImage* GetImage() const;

	//=====================================================================
	//== serialization                                                   ==
	//=====================================================================
public:
	virtual void ReadNative(CStream& oStream);
public:
	virtual void Read(CXMLReader& xml, hMETAFILE& nID);
public:
	virtual void Write(CXMLWriter& xml) const;

public:
	virtual void LoadItem(CTinyCadDoc*, CStream&, hRESOURCE);
public:
	virtual void SaveItemXML(CTinyCadDoc*, CXMLWriter& xml);
public:
	virtual void LoadItemXML(CTinyCadDoc*, CXMLReader& xml);

	//=====================================================================
	//== Comparision                                                     ==
	//=====================================================================
	//-- Compare this resource with another resource of the same type
public:
	virtual BOOL Compare(CDocResource*);
};
//=========================================================================
