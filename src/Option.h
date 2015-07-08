/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#pragma once

#include "Symbol.h"
#include "DocResource.h"
#include "ListOfFonts.h"
#include "ListOfStyles.h"
#include "ListOfFillStyles.h"
#include "ListOfMetaFiles.h"
#include "ListOfSymbols.h"
#include "UserColor.h"

//*************************************************************************
//*                                                                       *
//*  The option categories.                                               *
//*                                                                       *
//*************************************************************************

//=========================================================================
class COption
{
private:
	CTinyCadDoc* m_pDesign;

	CUserColor m_oColors;
	ListOfFonts theFontList;
	ListOfStyles m_colLineStyles;
	ListOfFillStyles theFillStyleList;
	ListOfMetaFiles theMetaFileList;
	ListOfSymbols* theSymbolList;

	// The current settings
	std::map<ObjType, hFONT> CurrentFont; // The default font number
	std::map<ObjType, hSTYLE> CurrentStyle; // The default line style
	std::map<ObjType, hFILL> CurrentFillStyle; // The default line style

	// The current grid settings
	Bool8 GridShow; // Should we show the grid?
	LONG Units; // The units to use

	CDPoint origin; // The grid origin

	// Some default settings for dialogs
	int PinLength; // The default pin length
	int PinNumberPos; // The default pin number position

	// The current auto-snap settings
	BOOL AutoSnap;
	BOOL AutoDrag;
	BOOL AutoJunc;
	int AutoSnapRange;

	//=====================================================================
	//== ctor/dtor/initializing                                          ==
	//=====================================================================
public:
	COption();
	~COption();
	void Init(CTinyCadDoc *pDesign);

	//=====================================================================
	//== Accessor                                                        ==
	//=====================================================================
	/** Get the user defined colours. */
	CUserColor& GetUserColor();

	/** Get the styles for line drawing. */
	ListOfStyles& GetLineStyles();

	//=====================================================================
	//== Mutator                                                         ==
	//=====================================================================

	//=====================================================================
	//== Serialization                                                   ==
	//=====================================================================
	void ReadXML(CXMLReader& xml);
	void WriteXML(CXMLWriter& xml);
	void ReadNative(CStream& oArchive);

	// Untag all of our resources
	void UnTag();

	////// The font operators ////

	//-- Add a font to the list and return it's new index number
	hFONT AddFont(LOGFONT *lf, hFONT MergeNumber = -1);

	//-- Load the font table from a file
	void LoadFonts(CStream &theFile)
	{
		theFontList.Load(m_pDesign, theFile);
	}

	// Convert an old font number into a new font number
	hFONT GetNewFontNumber(hFONT n)
	{
		return theFontList.ConvertOld(n);
	}

	//-- Get a font from the number
	LOGFONT *GetFont(hFONT n)
	{
		return ((ListOfFonts *) theFontList.Get(n))->GetFontPtr();
	}
	hFONT ChangeFontSize(hFONT n, double delta_height, double width);

	//-- Specify this font is in use
	void TagFont(hFONT n)
	{
		theFontList.Tag(n);
	}

	//-- Bring up the user change font box
	void ChooseFont(ObjType t, CDC &, HWND = NULL);
	hFONT GetCurrentFont(ObjType t);
	void SetCurrentFont(ObjType t, hFONT n)
	{
		CurrentFont[t] = n;
	}
	int GetFontWidth(hFONT n);

	////// The Line Style operators //////

	// Add a line style to the list and return it's new index number
	hSTYLE AddStyle(LineStyle *ls, hFONT MergeNumber = -1)
	{
		hSTYLE n = m_colLineStyles.Add(new ListOfStyles(*ls), MergeNumber);

		// TRACE2( "COption::Add: %d %d\n", n, MergeNumber );

		return n;
	}

	// Load the line style table from a file
	void LoadStyles(CStream &theFile)
	{
		m_colLineStyles.Load(m_pDesign, theFile);
	}
	// Convert an old line style number into a new font number
	hSTYLE GetNewStyleNumber(hSTYLE n)
	{
		hSTYLE m = m_colLineStyles.ConvertOld(n);
		// TRACE2( "COption::ConvertOld: %d to %d\n", n, m );

		return m;
	}
	// Get a line style from the number
	LineStyle *GetStyle(hSTYLE n)
	{
		return ((ListOfStyles *) m_colLineStyles.Get(n))->GetLineStylePtr();
	}
	
	// Specify this line style is in use
	void TagStyle(hSTYLE n)
	{
		m_colLineStyles.Tag(n);
	}
	
	hSTYLE GetCurrentStyle(ObjType t)
	{
		return CurrentStyle[t];
	}
	
	void SetCurrentStyle(ObjType t, hSTYLE n)
	{
		CurrentStyle[t] = n;
	}

	////// The Fill Style operators //////

	// Add a fill style to the list and return it's new index number
	hFILL AddFillStyle(FillStyle *ls, hFONT MergeNumber = -1)
	{
		return theFillStyleList.Add(new ListOfFillStyles(*ls), MergeNumber);
	}
	
	// Load the fill style table from a file
	void LoadFillStyles(CStream &theFile)
	{
		theFillStyleList.Load(m_pDesign, theFile);
	}
	
	// Convert an old line style number into a new font number
	hFILL GetNewFillStyleNumber(hFILL n)
	{
		return theFillStyleList.ConvertOld(n);
	}
	
	// Get a fill style from the number
	FillStyle *GetFillStyle(hSTYLE n)
	{
		return ((ListOfFillStyles *) theFillStyleList.Get(n))->GetFillStylePtr();
	}
	
	// Specify this fill style is in use
	void TagFillStyle(hSTYLE n)
	{
		theFillStyleList.Tag(n);
	}
	
	hFILL GetCurrentFillStyle(ObjType t)
	{
		return CurrentFillStyle[t];
	}
	
	void SetCurrentFillStyle(ObjType t, hFILL n)
	{
		CurrentFillStyle[t] = n;
	}

	////// The Meta File operators //////

	// Add a line style to the list and return it's new index number
	hMETAFILE AddMetaFile(CTCImage* s, hMETAFILE MergeNumber = -1)
	{
		return theMetaFileList.Add(new ListOfMetaFiles(s), MergeNumber);
	}
	
	// Load the line style table from a file
	void LoadMetaFiles(CStream &theFile)
	{
		theMetaFileList.Load(m_pDesign, theFile);
	}
	
	// Convert an old line style number into a new font number
	hMETAFILE GetNewMetaFileNumber(hMETAFILE n)
	{
		return theMetaFileList.ConvertOld(n);
	}
	
	// Get a line style from the number
	CTCImage* GetImage(hMETAFILE n)
	{
		return ((ListOfMetaFiles *) theMetaFileList.Get(n))->GetImage();
	}
	
	// Specify this line style is in use
	void TagMetaFile(hSTYLE n)
	{
		theMetaFileList.Tag(n);
	}

	////// The Symbols operators //////

	// Add a line style to the list and return it's new index number
	hSYMBOL AddSymbol(CDesignFileSymbol *s, hSYMBOL MergeNumber = -1)
	{
		return theSymbolList->Add(new ListOfSymbols(s), MergeNumber);
	}
	
	// Load the line style table from a file
	void LoadSymbols(CStream &theFile)
	{
		theSymbolList->Load(m_pDesign, theFile);
	}
	
	// Convert an old line style number into a new font number
	hSYMBOL GetNewSymbolNumber(hSYMBOL n)
	{
		return theSymbolList->ConvertOld(n);
	}
	
	// Get a line style from the number
	CDesignFileSymbol *GetSymbol(hSYMBOL n)
	{
		return ((ListOfSymbols *) theSymbolList->Get(n))->GetSymbol();
	}
	
	// Specify this line style is in use
	void TagSymbol(hSYMBOL n)
	{
		theSymbolList->Tag(n);
	}

	////// The XML operations //////

	void SaveFontsXML(CXMLWriter &xml)
	{
		theFontList.SaveXML(m_pDesign, _T("FONT"), xml);
	}
	
	void SaveStylesXML(CXMLWriter &xml)
	{
		m_colLineStyles.SaveXML(m_pDesign, _T("STYLE"), xml);
	}
	
	void SaveFillStylesXML(CXMLWriter &xml)
	{
		theFillStyleList.SaveXML(m_pDesign, _T("FILL"), xml);
	}
	
	void SaveMetaFilesXML(CXMLWriter &xml)
	{
		theMetaFileList.SaveXML(m_pDesign, _T("IMAGE"), xml);
	}
	
	void SaveSymbolsXML(CXMLWriter &xml)
	{
		theSymbolList->SaveXML(m_pDesign, _T("SYMBOLDEF"), xml);
	}

	void LoadFontXML(CXMLReader &xml)
	{
		theFontList.LoadItemXML(m_pDesign, xml);
	}
	
	void LoadStyleXML(CXMLReader &xml)
	{
		m_colLineStyles.LoadItemXML(m_pDesign, xml);
	}
	
	void LoadFillStyleXML(CXMLReader &xml)
	{
		theFillStyleList.LoadItemXML(m_pDesign, xml);
	}
	
	void LoadMetaFileXML(CXMLReader &xml)
	{
		theMetaFileList.LoadItemXML(m_pDesign, xml);
	}
	
	void LoadSymbolXML(CXMLReader &xml)
	{
		theSymbolList->LoadItemXML(m_pDesign, xml);
	}

	void ResetMerge();

	////// The grid operators //////

	bool ShowGrid()
	{
		return GridShow == TRUE;
	} // The size of the current grid
	
	void SetGridShow(bool s)
	{
		GridShow = s;
	}
	
	int GetUnits()
	{
		return Units;
	}
	
	void SetUnits(int u)
	{
		Units = u;
	}
	
	CString PointToUnit(CDPoint);
	CString PointToDisplay(CDPoint, BOOL horiz);

	////// The auto-wire settins //////
	bool GetAutoSnap()
	{
		return AutoSnap == TRUE;
	}
	
	bool GetAutoDrag()
	{
		return AutoDrag == TRUE;
	}
	
	bool GetAutoJunc()
	{
		return AutoJunc == TRUE;
	}
	
	int GetAutoSnapRange()
	{
		return AutoSnapRange;
	}

	void SetAutoSnap(bool r)
	{
		AutoSnap = r;
	}
	
	void SetAutoDrag(bool r)
	{
		AutoDrag = r;
	}
	
	void SetAutoJunc(bool r)
	{
		AutoJunc = r;
	}
	
	void SetAutoSnapRange(int r)
	{
		AutoSnapRange = r;
	}

	////// Some default settings for dialogs ////// 
	int GetPinLength()
	{
		return PinLength;
	}
	
	void SetPinLength(int pl);

	int GetPinNumberPos()
	{
		return PinNumberPos;
	}
	
	void SetPinNumberPos(int pl);

	bool HasOrigin();
	void SetOrigin(CDPoint org);
	CDPoint GetOrigin();

};
//=========================================================================

